//! @file lang/csv.c
//! @brief CSV language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "\"(?:[^\"]|\"\")*\"", .token = HL_TOKEN_STR },
    { .pattern = ",", .token = HL_TOKEN_OPER },
};

static const hl_lang_def_t lang = {
    .name = "csv",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_csv(void) { return &lang; }
