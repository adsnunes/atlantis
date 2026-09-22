//! @file lang/make.c
//! @brief Makefile language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^\\s*#.*", .token = HL_TOKEN_CMNT, .flags = HL_RULE_MULTILINE },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "[${}()]+", .token = HL_TOKEN_OPER },
    { .pattern = "\\.PHONY:", .token = HL_TOKEN_CLASS, .flags = HL_RULE_MULTILINE },
    { .pattern = "^[\\w.]+:", .token = HL_TOKEN_SECTION, .flags = HL_RULE_MULTILINE },
    { .pattern = "\\b(?:ifneq|ifeq|ifdef|ifndef|endif|else)\\b", .token = HL_TOKEN_KWD },
    { .pattern = "(?:\\.e?|\\b)\\d(?:e-|[\\d.oxa-fA-F_])*(?:\\.|\\b)", .token = HL_TOKEN_NUM },
    { .pattern = "[A-Z_]+(?=\\s*[?:]?=)", .token = HL_TOKEN_VAR },
};

static const hl_detect_rule_t detect[] = {
    { "\\b(ifneq|endif|if|elif|then|fi|echo|\\.PHONY)\\b|\\$", 10 },
};

static const char* const aliases[] = { "makefile", NULL };

static const hl_lang_def_t lang = {
    .name = "make",
    .aliases = aliases,
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_make(void) { return &lang; }
