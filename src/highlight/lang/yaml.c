//! @file lang/yaml.c
//! @brief YAML language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "#.*(?:\\n|$)", .token = HL_TOKEN_CMNT },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "(?:>|\\|)\\r?\\n(?:(?:\\s[^\\n]*)?(?:\\r?\\n|$))*", .token = HL_TOKEN_STR },
    { .pattern = "!![a-z]+", .token = HL_TOKEN_TYPE },
    { .pattern = "\\b(?:Yes|No|true|false)\\b", .token = HL_TOKEN_BOOL },
    { .pattern = "\\b(?:null|~)\\b", .token = HL_TOKEN_NUM },
    { .pattern = "(?:\\.e?|\\b)\\d(?:e-|[\\d.oxa-fA-F_])*(?:\\.|\\b)", .token = HL_TOKEN_NUM },
    { .pattern = "[a-zA-Z_][\\w_]*(?=\\s*:)", .token = HL_TOKEN_VAR },
    { .pattern = "[+:-]", .token = HL_TOKEN_OPER },
    { .pattern = "[&*][a-zA-Z_][\\w_]*", .token = HL_TOKEN_VAR },
};

static const hl_detect_rule_t detect[] = {
    { "^(\\s+)?[a-z][a-z0-9]*:", 10 },
};

static const char* const aliases[] = { "yml", NULL };

static const hl_lang_def_t lang = {
    .name = "yaml",
    .aliases = aliases,
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_yaml(void) { return &lang; }
