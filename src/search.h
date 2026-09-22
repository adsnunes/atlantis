// search.h - Web search and AI tools
//! Provides web search and tool callbacks for AI integration

#ifndef SEARCH_H
#define SEARCH_H

// #region Initialization

//! Initialize search subsystem (call once at startup)
void search_tool_init(void);

//! Cleanup search subsystem (call at shutdown)
void search_tool_cleanup(void);

// #endregion

// #region Search API

//! Perform a web search using DuckDuckGo
//! @param query search query string
//! @return newly allocated string with results (caller must free)
char* search_web(const char* query);

// #endregion

// #region AI Tool Callbacks

//! Web search tool callback for AI integration
//! @param params_json JSON string with "query" parameter
//! @param user_data unused
//! @return JSON response string (caller must free)
char* search_tool_callback(const char* params_json, void* user_data);

//! Time/date tool callback for AI integration
//! @param params_json unused
//! @param user_data unused
//! @return JSON response with current time/date (caller must free)
char* time_tool_callback(const char* params_json, void* user_data);

//! Past sessions tool callback for AI integration
//! @param params_json JSON with "action" (list/read) and optional "filename"
//! @param user_data should be the history directory path
//! @return JSON response (caller must free)
char* sessions_tool_callback(const char* params_json, void* user_data);

// #endregion

#endif // SEARCH_H
