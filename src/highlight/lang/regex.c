//! @file lang/regex.c
//! @brief Regex literal language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^(?!/).*", .token = HL_TOKEN_CMNT, .flags = HL_RULE_MULTILINE },
    { .pattern = "\\[((?!\\])[^\\\\]|\\\\.)*\\]", .token = HL_TOKEN_NUM },
    { .pattern = "\\||\\^|\\$|\\\\.|\\w+($|\\r|\\n)", .token = HL_TOKEN_KWD },
    { .pattern = "\\*|\\+|\\{\\d+,\\d+\\}", .token = HL_TOKEN_VAR },
};

static const hl_lang_def_t lang = {
    .name = "regex",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_OPER,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_regex(void) { return &lang; }
