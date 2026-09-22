//! @file lang/uri.c
//! @brief URI/URL language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^#.*", .token = HL_TOKEN_CMNT, .flags = HL_RULE_MULTILINE },
    { .pattern = "^\\w+(?=:)", .token = HL_TOKEN_CLASS, .flags = HL_RULE_MULTILINE },
    { .pattern = ":\\d+", .token = HL_TOKEN_NUM },
    { .pattern = "[:/&?]|\\w+=", .token = HL_TOKEN_OPER },
    { .pattern = "[.\\w]+@|#[\\w]+$", .token = HL_TOKEN_FUNC, .flags = HL_RULE_MULTILINE },
    { .pattern = "\\w+\\.\\w+(\\.\\w+)*", .token = HL_TOKEN_VAR },
};

static const hl_detect_rule_t detect[] = {
    { "https?:|mailto:|tel:|ftp:", 30 },
};

static const hl_lang_def_t lang = {
    .name = "uri",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_uri(void) { return &lang; }
