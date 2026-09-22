//! @file lang/log.c
//! @brief Log file language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^#.*", .token = HL_TOKEN_CMNT, .flags = HL_RULE_MULTILINE },
    { .pattern = "\"(?:[^\"\\\\]|\\\\.)*\"", .token = HL_TOKEN_STR },
    { .pattern = "(?:\\.e?|\\b)\\d(?:e-|[\\d.oxa-fA-F_])*(?:\\.|\\b)", .token = HL_TOKEN_NUM },
    { .pattern = "\\b(err(or)?|[a-z_-]*exception|warn|warning|failed|ko|invalid|not ?found|alert|fatal)\\b", .token = HL_TOKEN_ERR, .flags = HL_RULE_CASELESS },
    { .pattern = "\\b(null|undefined)\\b", .token = HL_TOKEN_NUM, .flags = HL_RULE_CASELESS },
    { .pattern = "\\b(false|true|yes|no)\\b", .token = HL_TOKEN_BOOL, .flags = HL_RULE_CASELESS },
    { .pattern = "\\.|,", .token = HL_TOKEN_OPER },
};

static const hl_lang_def_t lang = {
    .name = "log",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_log(void) { return &lang; }
