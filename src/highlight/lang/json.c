//! @file lang/json.c
//! @brief JSON language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "([\"'])[^\"']*\\1(?=\\s*:)", .token = HL_TOKEN_VAR },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "-?(?:0|[1-9]\\d*)(?:\\.\\d+)?(?:[eE][+-]?\\d+)?", .token = HL_TOKEN_NUM },
    { .pattern = "\\bnull\\b", .token = HL_TOKEN_NUM },
    { .pattern = "\\b(?:true|false)\\b", .token = HL_TOKEN_BOOL },
};

static const hl_detect_rule_t detect[] = {
    { "\\b(true|false|null|\\{\\})\\b|\"[^\"]+\":", 10 },
};

static const hl_lang_def_t lang = {
    .name = "json",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_json(void) { return &lang; }
