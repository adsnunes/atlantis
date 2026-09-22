//! @file lang/http.c
//! @brief HTTP language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^(GET|HEAD|POST|PUT|DELETE|CONNECT|OPTIONS|TRACE|PATCH|PRI|SEARCH)\\b", .token = HL_TOKEN_KWD, .flags = HL_RULE_MULTILINE },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "\\bHTTP/[\\d.]+\\b", .token = HL_TOKEN_SECTION },
    { .pattern = "(?:\\.e?|\\b)\\d(?:e-|[\\d.oxa-fA-F_])*(?:\\.|\\b)", .token = HL_TOKEN_NUM },
    { .pattern = "[,;:=]", .token = HL_TOKEN_OPER },
    { .pattern = "[a-zA-Z][\\w-]*(?=:)", .token = HL_TOKEN_VAR },
};

static const hl_detect_rule_t detect[] = {
    { "^(GET|HEAD|POST|PUT|DELETE|PATCH|HTTP)\\b", 500 },
};

static const hl_lang_def_t lang = {
    .name = "http",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_http(void) { return &lang; }
