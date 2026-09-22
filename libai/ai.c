#include "ai.h"

#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ai_bridge.h"

#define MAX_SESSIONS_PER_CONTEXT 32

typedef struct {
  _Atomic(bool) initialized;
  _Atomic(uint64_t) next_context_id;
} global_state_t;

static global_state_t g_state = {.initialized = false, .next_context_id = 1};

struct ai_context {
  uint64_t context_id;
  char last_error[512];
  pthread_mutex_t mutex;
  void (*error_handler)(ai_result_t, const char *);

  ai_session_id_t active_sessions[MAX_SESSIONS_PER_CONTEXT];
  int session_count;

  uint64_t total_requests;
  uint64_t successful_requests;
  uint64_t failed_requests;
};

static void set_error(ai_context_t *context, ai_result_t code, const char *fmt,
                      ...) {
  va_list args;
  va_start(args, fmt);

  if (context) {
    pthread_mutex_lock(&context->mutex);
    vsnprintf(context->last_error, sizeof(context->last_error), fmt, args);
    pthread_mutex_unlock(&context->mutex);

    if (context->error_handler) {
      context->error_handler(code, context->last_error);
    }
  }

  va_end(args);
}

static void update_stats(ai_context_t *context, bool success) {
  if (!context) return;

  pthread_mutex_lock(&context->mutex);
  context->total_requests++;
  if (success)
    context->successful_requests++;
  else
    context->failed_requests++;
  pthread_mutex_unlock(&context->mutex);
}

static ai_availability_t convert_availability(ai_availability_status_t status) {
  switch (status) {
    case AI_BRIDGE_AVAILABLE:
      return AI_AVAILABLE;
    case AI_BRIDGE_DEVICE_NOT_ELIGIBLE:
      return AI_DEVICE_NOT_ELIGIBLE;
    case AI_BRIDGE_INTELLIGENCE_NOT_ENABLED:
      return AI_NOT_ENABLED;
    case AI_BRIDGE_MODEL_NOT_READY:
      return AI_MODEL_NOT_READY;
    default:
      return AI_AVAILABILITY_UNKNOWN;
  }
}

static ai_result_t convert_bridge_error(const char *error_msg) {
  if (!error_msg) return AI_ERROR_UNKNOWN;

  if (strstr(error_msg, "Session not found")) return AI_ERROR_SESSION_NOT_FOUND;
  if (strstr(error_msg, "Tool not found")) return AI_ERROR_TOOL_NOT_FOUND;
  if (strstr(error_msg, "Guardrail violation"))
    return AI_ERROR_GUARDRAIL_VIOLATION;
  if (strstr(error_msg, "Tool execution")) return AI_ERROR_TOOL_EXECUTION;
  if (strstr(error_msg, "JSON")) return AI_ERROR_JSON_PARSE;
  if (strstr(error_msg, "timeout")) return AI_ERROR_TIMEOUT;

  return AI_ERROR_GENERATION;
}

static bool validate_init(void) { return atomic_load(&g_state.initialized); }

static bool validate_context(ai_context_t *context) {
  return validate_init() && context != NULL;
}

static ai_bridge_session_id_t find_bridge_session(ai_context_t *context,
                                                  ai_session_id_t session_id) {
  if (!context || session_id == AI_INVALID_ID ||
      session_id > MAX_SESSIONS_PER_CONTEXT)
    return AI_BRIDGE_INVALID_ID;

  int index = session_id - 1;

  if (index >= 0 && index < context->session_count &&
      context->active_sessions[index] != AI_BRIDGE_INVALID_ID) {
    return context->active_sessions[index];
  }

  return AI_BRIDGE_INVALID_ID;
}

ai_result_t ai_init(void) {
  if (atomic_load(&g_state.initialized)) {
    return AI_SUCCESS;
  }

  if (!ai_bridge_init()) {
    return AI_ERROR_INIT_FAILED;
  }

  atomic_store(&g_state.next_context_id, 1);
  atomic_store(&g_state.initialized, true);

  return AI_SUCCESS;
}

void ai_cleanup(void) {
  if (!atomic_load(&g_state.initialized)) return;

  atomic_store(&g_state.initialized, false);
}

const char *ai_get_version(void) { return AI_VERSION_STRING; }

const char *ai_get_last_error(ai_context_t *context) {
  if (!context) return "No context provided";

  return context->last_error;
}

ai_availability_t ai_check_availability(void) {
  ai_availability_status_t status = ai_bridge_check_availability();
  return convert_availability(status);
}

char *ai_get_availability_reason(void) {
  if (!validate_init()) return NULL;
  return ai_bridge_get_availability_reason();
}

bool ai_is_ready(void) { return ai_check_availability() == AI_AVAILABLE; }

int32_t ai_get_supported_languages_count(void) {
  if (!validate_init()) return 0;
  return ai_bridge_get_supported_languages_count();
}

char *ai_get_supported_language(int32_t index) {
  if (!validate_init()) return NULL;
  return ai_bridge_get_supported_language(index);
}

ai_context_t *ai_context_create(void) {
  if (!validate_init()) return NULL;

  ai_context_t *context = malloc(sizeof(ai_context_t));
  if (!context) {
    return NULL;
  }

  memset(context, 0, sizeof(ai_context_t));

  context->context_id = atomic_fetch_add(&g_state.next_context_id, 1);
  pthread_mutex_init(&context->mutex, NULL);

  for (int i = 0; i < MAX_SESSIONS_PER_CONTEXT; i++) {
    context->active_sessions[i] = AI_BRIDGE_INVALID_ID;
  }

  return context;
}

void ai_context_free(ai_context_t *context) {
  if (!context) return;

  for (int i = 0; i < context->session_count; i++) {
    if (context->active_sessions[i] != AI_BRIDGE_INVALID_ID) {
      ai_bridge_destroy_session(context->active_sessions[i]);
    }
  }

  pthread_mutex_destroy(&context->mutex);
  free(context);
}

ai_session_id_t ai_create_session(ai_context_t *context,
                                  const ai_session_config_t *config) {
  if (!validate_context(context)) return AI_INVALID_ID;

  if (ai_check_availability() != AI_AVAILABLE) {
    set_error(context, AI_ERROR_NOT_AVAILABLE,
              "Apple Intelligence not available");
    return AI_INVALID_ID;
  }

  ai_session_config_t default_config = AI_DEFAULT_SESSION_CONFIG;
  if (!config) config = &default_config;

  int session_index = -1;
  pthread_mutex_lock(&context->mutex);
  for (int i = 0; i < MAX_SESSIONS_PER_CONTEXT; i++) {
    if (context->active_sessions[i] == AI_BRIDGE_INVALID_ID) {
      session_index = i;
      break;
    }
  }
  pthread_mutex_unlock(&context->mutex);

  if (session_index == -1) {
    set_error(context, AI_ERROR_INVALID_PARAMS,
              "Maximum sessions per context reached");
    return AI_INVALID_ID;
  }

  ai_bridge_session_id_t bridge_session =
      ai_bridge_create_session(config->instructions, config->tools_json,
                               config->enable_guardrails, config->prewarm);

  if (bridge_session == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_GENERATION, "Failed to create bridge session");
    return AI_INVALID_ID;
  }

  pthread_mutex_lock(&context->mutex);
  context->active_sessions[session_index] = bridge_session;

  if (session_index >= context->session_count) {
    context->session_count = session_index + 1;
  }
  pthread_mutex_unlock(&context->mutex);

  return session_index + 1;
}

ai_result_t ai_register_tool(ai_context_t *context, ai_session_id_t session_id,
                             const char *tool_name, ai_tool_callback_t callback,
                             void *user_data) {
  if (!validate_context(context) || !tool_name || !callback) {
    set_error(context, AI_ERROR_INVALID_PARAMS,
              "Invalid parameters for tool registration");
    return AI_ERROR_INVALID_PARAMS;
  }

  ai_bridge_session_id_t bridge_session =
      find_bridge_session(context, session_id);
  if (bridge_session == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_SESSION_NOT_FOUND, "Session not found");
    return AI_ERROR_SESSION_NOT_FOUND;
  }

  ai_bridge_tool_callback_t bridge_callback =
      (ai_bridge_tool_callback_t)callback;

  if (!ai_bridge_register_tool(bridge_session, tool_name, bridge_callback,
                               user_data)) {
    set_error(context, AI_ERROR_TOOL_EXECUTION,
              "Failed to register tool with bridge");
    return AI_ERROR_TOOL_EXECUTION;
  }

  return AI_SUCCESS;
}

void ai_destroy_session(ai_context_t *context, ai_session_id_t session_id) {
  if (!validate_context(context)) return;

  ai_bridge_session_id_t bridge_session =
      find_bridge_session(context, session_id);
  if (bridge_session != AI_BRIDGE_INVALID_ID) {
    ai_bridge_destroy_session(bridge_session);

    int index = session_id - 1;
    if (index >= 0 && index < MAX_SESSIONS_PER_CONTEXT) {
      pthread_mutex_lock(&context->mutex);
      context->active_sessions[index] = AI_BRIDGE_INVALID_ID;
      pthread_mutex_unlock(&context->mutex);
    }
  }
}

char *ai_get_session_history(ai_context_t *context,
                             ai_session_id_t session_id) {
  if (!validate_context(context)) return NULL;

  ai_bridge_session_id_t bridge_session =
      find_bridge_session(context, session_id);
  if (bridge_session == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_SESSION_NOT_FOUND, "Session not found");
    return NULL;
  }

  char *history = ai_bridge_get_session_history(bridge_session);
  if (!history) {
    set_error(context, AI_ERROR_INVALID_PARAMS,
              "History not available for this session");
    return NULL;
  }

  return history;
}

ai_result_t ai_clear_session_history(ai_context_t *context,
                                     ai_session_id_t session_id) {
  if (!validate_context(context)) return AI_ERROR_INVALID_PARAMS;

  ai_bridge_session_id_t bridge_session =
      find_bridge_session(context, session_id);
  if (bridge_session == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_SESSION_NOT_FOUND, "Session not found");
    return AI_ERROR_SESSION_NOT_FOUND;
  }

  if (!ai_bridge_clear_session_history(bridge_session)) {
    set_error(context, AI_ERROR_INVALID_PARAMS, "Failed to clear history");
    return AI_ERROR_INVALID_PARAMS;
  }

  return AI_SUCCESS;
}

ai_result_t ai_add_message_to_history(ai_context_t *context,
                                      ai_session_id_t session_id,
                                      const char *role, const char *content) {
  if (!validate_context(context) || !role || !content) {
    set_error(context, AI_ERROR_INVALID_PARAMS,
              "Invalid parameters for adding message to history");
    return AI_ERROR_INVALID_PARAMS;
  }

  ai_bridge_session_id_t bridge_session =
      find_bridge_session(context, session_id);
  if (bridge_session == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_SESSION_NOT_FOUND, "Session not found");
    return AI_ERROR_SESSION_NOT_FOUND;
  }

  if (!ai_bridge_add_message_to_history(bridge_session, role, content)) {
    set_error(context, AI_ERROR_INVALID_PARAMS,
              "Failed to add message to history");
    return AI_ERROR_INVALID_PARAMS;
  }

  return AI_SUCCESS;
}

char *ai_generate_response(ai_context_t *context, ai_session_id_t session_id,
                           const char *prompt,
                           const ai_generation_params_t *params) {
  if (!prompt) {
    set_error(context, AI_ERROR_INVALID_PARAMS, "Prompt cannot be NULL");
    return NULL;
  }

  if (!validate_context(context)) return NULL;

  ai_bridge_session_id_t bridge_session =
      find_bridge_session(context, session_id);
  if (bridge_session == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_SESSION_NOT_FOUND, "Session not found");
    return NULL;
  }

  ai_generation_params_t default_params = AI_DEFAULT_PARAMS;
  if (!params) params = &default_params;

  char *response = ai_bridge_generate_response(
      bridge_session, prompt, params->temperature, params->max_tokens);

  if (!response) {
    set_error(context, AI_ERROR_GENERATION, "Response generation failed");
    update_stats(context, false);
    return NULL;
  }

  if (strncmp(response, "Error:", 6) == 0) {
    ai_result_t error_code = convert_bridge_error(response);
    set_error(context, error_code, "%s", response);
    ai_bridge_free_string(response);
    update_stats(context, false);
    return NULL;
  }

  update_stats(context, true);
  return response;
}

char *ai_generate_structured_response(ai_context_t *context,
                                      ai_session_id_t session_id,
                                      const char *prompt,
                                      const char *schema_json,
                                      const ai_generation_params_t *params) {
  if (!prompt) {
    set_error(context, AI_ERROR_INVALID_PARAMS, "Prompt cannot be NULL");
    return NULL;
  }

  if (!validate_context(context)) return NULL;

  ai_bridge_session_id_t bridge_session =
      find_bridge_session(context, session_id);
  if (bridge_session == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_SESSION_NOT_FOUND, "Session not found");
    return NULL;
  }

  ai_generation_params_t default_params = AI_DEFAULT_PARAMS;
  if (!params) params = &default_params;

  char *response = ai_bridge_generate_structured_response(
      bridge_session, prompt, schema_json, params->temperature,
      params->max_tokens);

  if (!response) {
    set_error(context, AI_ERROR_GENERATION,
              "Structured response generation failed");
    update_stats(context, false);
    return NULL;
  }

  if (strncmp(response, "Error:", 6) == 0) {
    ai_result_t error_code = convert_bridge_error(response);
    set_error(context, error_code, "%s", response);
    ai_bridge_free_string(response);
    update_stats(context, false);
    return NULL;
  }

  update_stats(context, true);
  return response;
}

ai_stream_id_t ai_generate_response_stream(ai_context_t *context,
                                           ai_session_id_t session_id,
                                           const char *prompt,
                                           const ai_generation_params_t *params,
                                           ai_stream_callback_t callback,
                                           void *user_data) {
  if (!prompt || !callback) {
    set_error(context, AI_ERROR_INVALID_PARAMS,
              "Prompt and callback cannot be NULL");
    return AI_INVALID_ID;
  }

  if (!validate_context(context)) return AI_INVALID_ID;

  ai_bridge_session_id_t bridge_session =
      find_bridge_session(context, session_id);
  if (bridge_session == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_SESSION_NOT_FOUND, "Session not found");
    return AI_INVALID_ID;
  }

  ai_generation_params_t default_params = AI_DEFAULT_PARAMS;
  if (!params) params = &default_params;

  ai_bridge_stream_callback_t bridge_callback =
      (ai_bridge_stream_callback_t)callback;

  ai_bridge_stream_id_t bridge_stream = ai_bridge_generate_response_stream(
      bridge_session, prompt, params->temperature, params->max_tokens, context,
      bridge_callback, user_data);

  if (bridge_stream == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_GENERATION, "Failed to start streaming");
    return AI_INVALID_ID;
  }

  return bridge_stream;
}

ai_stream_id_t ai_generate_structured_response_stream(
    ai_context_t *context, ai_session_id_t session_id, const char *prompt,
    const char *schema_json, const ai_generation_params_t *params,
    ai_stream_callback_t callback, void *user_data) {
  if (!prompt || !callback) {
    set_error(context, AI_ERROR_INVALID_PARAMS,
              "Prompt and callback cannot be NULL");
    return AI_INVALID_ID;
  }

  if (!validate_context(context)) return AI_INVALID_ID;

  ai_bridge_session_id_t bridge_session =
      find_bridge_session(context, session_id);
  if (bridge_session == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_SESSION_NOT_FOUND, "Session not found");
    return AI_INVALID_ID;
  }

  ai_generation_params_t default_params = AI_DEFAULT_PARAMS;
  if (!params) params = &default_params;

  ai_bridge_stream_callback_t bridge_callback =
      (ai_bridge_stream_callback_t)callback;

  ai_bridge_stream_id_t bridge_stream =
      ai_bridge_generate_structured_response_stream(
          bridge_session, prompt, schema_json, params->temperature,
          params->max_tokens, context, bridge_callback, user_data);

  if (bridge_stream == AI_BRIDGE_INVALID_ID) {
    set_error(context, AI_ERROR_GENERATION,
              "Failed to start structured streaming");
    return AI_INVALID_ID;
  }

  return bridge_stream;
}

ai_result_t ai_cancel_stream(ai_context_t *context, ai_stream_id_t stream_id) {
  if (!validate_context(context)) return AI_ERROR_INVALID_PARAMS;

  if (stream_id == AI_INVALID_ID) {
    set_error(context, AI_ERROR_STREAM_NOT_FOUND, "Invalid stream ID");
    return AI_ERROR_STREAM_NOT_FOUND;
  }

  if (ai_bridge_cancel_stream(stream_id)) {
    return AI_SUCCESS;
  }

  set_error(context, AI_ERROR_STREAM_NOT_FOUND,
            "Stream not found or already completed");
  return AI_ERROR_STREAM_NOT_FOUND;
}

bool ai_validate_messages_json(const char *messages_json) {
  if (!messages_json) return false;

  // Basic validation - could be extended with proper JSON parsing
  return strlen(messages_json) > 0 && messages_json[0] == '[';
}

void ai_free_string(char *str) {
  if (str) {
    ai_bridge_free_string(str);
  }
}

const char *ai_get_error_description(ai_result_t result) {
  switch (result) {
    case AI_SUCCESS:
      return "Success";
    case AI_ERROR_INIT_FAILED:
      return "Initialization failed";
    case AI_ERROR_NOT_AVAILABLE:
      return "Apple Intelligence not available";
    case AI_ERROR_INVALID_PARAMS:
      return "Invalid parameters";
    case AI_ERROR_MEMORY:
      return "Memory allocation error";
    case AI_ERROR_JSON_PARSE:
      return "JSON parsing error";
    case AI_ERROR_GENERATION:
      return "Text generation error";
    case AI_ERROR_TIMEOUT:
      return "Operation timeout";
    case AI_ERROR_SESSION_NOT_FOUND:
      return "Session not found";
    case AI_ERROR_STREAM_NOT_FOUND:
      return "Stream not found";
    case AI_ERROR_GUARDRAIL_VIOLATION:
      return "Content blocked by safety filters";
    case AI_ERROR_TOOL_NOT_FOUND:
      return "Tool callback not registered";
    case AI_ERROR_TOOL_EXECUTION:
      return "Tool execution failed";
    case AI_ERROR_UNKNOWN:
      return "Unknown error";
    default:
      return "Invalid error code";
  }
}

void ai_set_error_handler(ai_context_t *context,
                          void (*handler)(ai_result_t, const char *)) {
  if (!context) return;

  pthread_mutex_lock(&context->mutex);
  context->error_handler = handler;
  pthread_mutex_unlock(&context->mutex);
}

ai_result_t ai_get_stats(ai_context_t *context, ai_stats_t *stats) {
  if (!stats) return AI_ERROR_INVALID_PARAMS;

  if (!context) {
    memset(stats, 0, sizeof(ai_stats_t));
    return AI_SUCCESS;
  }

  pthread_mutex_lock(&context->mutex);
  stats->total_requests = context->total_requests;
  stats->successful_requests = context->successful_requests;
  stats->failed_requests = context->failed_requests;
  stats->total_tokens_generated = 0;
  stats->average_response_time = 0.0;
  stats->total_processing_time = 0.0;
  pthread_mutex_unlock(&context->mutex);

  return AI_SUCCESS;
}

void ai_reset_stats(ai_context_t *context) {
  if (!context) return;

  pthread_mutex_lock(&context->mutex);
  context->total_requests = 0;
  context->successful_requests = 0;
  context->failed_requests = 0;
  pthread_mutex_unlock(&context->mutex);
}