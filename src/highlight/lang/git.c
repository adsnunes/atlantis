//! @file lang/git.c
//! @brief Git output language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^#.*", .token = HL_TOKEN_CMNT, .flags = HL_RULE_MULTILINE },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "^[-<].*$", .token = HL_TOKEN_DELETED, .flags = HL_RULE_MULTILINE },
    { .pattern = "^[+>].*$", .token = HL_TOKEN_INSERT, .flags = HL_RULE_MULTILINE },
    { .pattern = "^(\\$ )?git(\\s.*)?$", .token = HL_TOKEN_FUNC, .flags = HL_RULE_MULTILINE },
    { .pattern = "^commit \\w+$", .token = HL_TOKEN_KWD, .flags = HL_RULE_MULTILINE },
    { .pattern = "^@@.*@@$", .token = HL_TOKEN_SECTION, .flags = HL_RULE_MULTILINE },
};

static const hl_lang_def_t lang = {
    .name = "git",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_git(void) { return &lang; }
