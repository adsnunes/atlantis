//! @file lang/toml.c
//! @brief TOML language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "#.*(?:\\n|$)", .token = HL_TOKEN_CMNT },
    { .pattern = "(\"\"\"|''')(?:\\\\[\\s\\S]|(?!\\1)[\\s\\S])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "^\\s*\\[\\[?[^\\]]+\\]\\]?\\s*$", .token = HL_TOKEN_SECTION, .flags = HL_RULE_MULTILINE },
    { .pattern = "\\b(?:inf|nan)\\b", .token = HL_TOKEN_NUM },
    { .pattern = "\\d{4}-\\d{2}-\\d{2}(?:T\\d{2}:\\d{2}:\\d{2}(?:\\.\\d+)?(?:Z|[+-]\\d{2}:\\d{2})?)?", .token = HL_TOKEN_NUM },
    { .pattern = "(?:\\.e?|\\b)\\d(?:e-|[\\d.oxa-fA-F_])*(?:\\.|\\b)", .token = HL_TOKEN_NUM },
    { .pattern = "\\b(?:true|false)\\b", .token = HL_TOKEN_BOOL },
    { .pattern = "[\\w_-]+(?=\\s*=)", .token = HL_TOKEN_VAR },
    { .pattern = "[+,.=-]", .token = HL_TOKEN_OPER },
};

static const hl_lang_def_t lang = {
    .name = "toml",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_toml(void) { return &lang; }
