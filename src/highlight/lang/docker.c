//! @file lang/docker.c
//! @brief Dockerfile language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^(FROM|RUN|CMD|LABEL|MAINTAINER|EXPOSE|ENV|ADD|COPY|ENTRYPOINT|VOLUME|USER|WORKDIR|ARG|ONBUILD|STOPSIGNAL|HEALTHCHECK|SHELL)\\b", .token = HL_TOKEN_KWD, .flags = HL_RULE_MULTILINE | HL_RULE_CASELESS },
    { .pattern = "#.*", .token = HL_TOKEN_CMNT },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "\\$\\w+|\\$\\{[^}]*\\}", .token = HL_TOKEN_VAR },
};

static const hl_detect_rule_t detect[] = {
    { "^(FROM|ENTRYPOINT|RUN)\\b", 500 },
};

static const char* const aliases[] = { "docker", NULL };

static const hl_lang_def_t lang = {
    .name = "dockerfile",
    .aliases = aliases,
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_docker(void) { return &lang; }
