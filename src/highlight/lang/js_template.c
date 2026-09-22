//! @file lang/js_template.c
//! @brief JavaScript template literal language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "\\$\\{", .token = HL_TOKEN_KWD },
    { .pattern = "\\}", .token = HL_TOKEN_KWD },
};

static const hl_lang_def_t lang = {
    .name = "js_template",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_STR,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_js_template(void) { return &lang; }
