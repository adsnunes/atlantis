//! @file lang/todo.c
//! @brief TODO/FIXME comment annotations

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "\\b(TODO|FIXME|DEBUG|OPTIMIZE|WARNING|XXX|BUG)\\b", .token = HL_TOKEN_ERR },
    { .pattern = "\\bIDEA\\b", .token = HL_TOKEN_CLASS },
    { .pattern = "\\b(CHANGED|FIX|CHANGE)\\b", .token = HL_TOKEN_INSERT },
    { .pattern = "\\bQUESTION\\b", .token = HL_TOKEN_OPER },
};

static const hl_lang_def_t lang = {
    .name = "todo",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_CMNT,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_todo(void) { return &lang; }
