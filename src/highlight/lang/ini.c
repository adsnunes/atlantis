//! @file lang/ini.c
//! @brief INI configuration language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^[ \\f\\t\\v]*[#;].*", .token = HL_TOKEN_CMNT, .flags = HL_RULE_MULTILINE },
    { .pattern = "^\\s*\\[.+\\]\\s*$", .token = HL_TOKEN_SECTION, .flags = HL_RULE_MULTILINE },
    { .pattern = "[^=\\n]+(?==)", .token = HL_TOKEN_VAR },
    { .pattern = "=", .token = HL_TOKEN_OPER },
};

static const hl_lang_def_t lang = {
    .name = "ini",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_STR,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_ini(void) { return &lang; }
