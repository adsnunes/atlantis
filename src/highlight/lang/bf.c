//! @file lang/bf.c
//! @brief Brainfuck language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "[^\\[\\->+.<\\]\\s].*", .token = HL_TOKEN_CMNT },
    { .pattern = "\\.+", .token = HL_TOKEN_FUNC },
    { .pattern = "[<>]+", .token = HL_TOKEN_KWD },
    { .pattern = "[+-]+", .token = HL_TOKEN_OPER },
};

static const hl_lang_def_t lang = {
    .name = "bf",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_bf(void) { return &lang; }
