//! @file lang/plain.c
//! @brief Plain text (minimal highlighting)

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "\"(?:[^\"\\\\]|\\\\.)*\"", .token = HL_TOKEN_STR },
};

static const char* const aliases[] = { "text", "txt", NULL };

static const hl_lang_def_t lang = {
    .name = "plain",
    .aliases = aliases,
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_plain(void) { return &lang; }
