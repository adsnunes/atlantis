//! @file lang/diff.c
//! @brief Diff language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^[-<].*$", .token = HL_TOKEN_DELETED, .flags = HL_RULE_MULTILINE },
    { .pattern = "^[+>].*$", .token = HL_TOKEN_INSERT, .flags = HL_RULE_MULTILINE },
    { .pattern = "^!.*$", .token = HL_TOKEN_KWD, .flags = HL_RULE_MULTILINE },
    { .pattern = "^@@.*@@$|^\\d.*|^([*-+])\\1\\1.*$", .token = HL_TOKEN_SECTION, .flags = HL_RULE_MULTILINE },
};

static const hl_detect_rule_t detect[] = {
    { "^@@ ?[-+,0-9 ]+ ?@@", 25 },
    { "^[+><-]", 10 },
};

static const hl_lang_def_t lang = {
    .name = "diff",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_diff(void) { return &lang; }
