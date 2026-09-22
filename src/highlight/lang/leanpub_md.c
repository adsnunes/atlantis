//! @file lang/leanpub_md.c
//! @brief Leanpub Markdown language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "leanpub-(start|end)-insert", .token = HL_TOKEN_INSERT },
    { .pattern = "leanpub-(start|end)-delete", .token = HL_TOKEN_DELETED },
    { .pattern = "^>.*|^(=|-)\\1+$", .token = HL_TOKEN_CMNT, .flags = HL_RULE_MULTILINE },
    { .pattern = "\\*\\*(?:(?!\\*\\*).)*\\*\\*", .token = HL_TOKEN_CLASS },
    { .pattern = "```(?:(?!```)[\\s\\S])*```", .token = HL_TOKEN_KWD },
    { .pattern = "`[^`]*`", .token = HL_TOKEN_STR },
    { .pattern = "~~(?:(?!~~).)*~~", .token = HL_TOKEN_VAR },
    { .pattern = "\\b_\\S(?:[^\\n]*?\\S)?_\\b|\\*\\S(?:[^\\n]*?\\S)?\\*", .token = HL_TOKEN_KWD },
    { .pattern = "^\\s*(?:\\*|\\d+\\.)\\s", .token = HL_TOKEN_KWD, .flags = HL_RULE_MULTILINE },
    { .pattern = "\\[[^\\]]*\\]\\([^)]*\\)|<[^>]*>", .token = HL_TOKEN_FUNC },
    { .pattern = "^#{1,6}\\s.*$", .token = HL_TOKEN_SECTION, .flags = HL_RULE_MULTILINE },
};

static const hl_lang_def_t lang = {
    .name = "leanpub-md",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = NULL,
    .detect_count = 0,
};

const hl_lang_def_t* hl_lang_leanpub_md(void) { return &lang; }
