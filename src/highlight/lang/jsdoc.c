//! @file lang/jsdoc.c
//! @brief JSDoc comment language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "@\\w+", .token = HL_TOKEN_KWD },
    { .pattern = "\\{[\\w\\s|<>,.@\\[\\]]+\\}", .token = HL_TOKEN_CLASS },
    { .pattern = "\\[[\\w\\s=\"']+\\]", .token = HL_TOKEN_VAR },
    { .pattern = "\\b(TODO|FIXME|DEBUG|OPTIMIZE|WARNING|XXX|BUG)\\b", .token = HL_TOKEN_ERR },
    { .pattern = "\\bIDEA\\b", .token = HL_TOKEN_CLASS },
    { .pattern = "\\b(CHANGED|FIX|CHANGE)\\b", .token = HL_TOKEN_INSERT },
    { .pattern = "\\bQUESTION\\b", .token = HL_TOKEN_OPER },
};

static const hl_lang_def_t lang = {
    .name = "jsdoc",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_CMNT,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_jsdoc(void) { return &lang; }
