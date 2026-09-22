//! @file lang/perl.c
//! @brief Perl language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "#.*", .token = HL_TOKEN_CMNT },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "(?:\\.e?|\\b)\\d(?:e-|[\\d.oxa-fA-F_])*(?:\\.|\\b)", .token = HL_TOKEN_NUM },
    { .pattern = "\\b(?:any|break|continue|default|delete|die|do|else|elsif|eval|for|foreach|given|goto|if|last|local|my|next|our|package|print|redo|require|return|say|state|sub|switch|undef|unless|until|use|when|while|not|and|or|xor)\\b", .token = HL_TOKEN_KWD },
    { .pattern = "[-+*/%~!&<>|=?,]+", .token = HL_TOKEN_OPER },
    { .pattern = "[a-z_]+(?=\\s*\\()", .token = HL_TOKEN_FUNC },
    { .pattern = "[$@%][a-zA-Z_][\\w_]*", .token = HL_TOKEN_VAR },
};

static const hl_detect_rule_t detect[] = {
    { "#!(/usr)?/bin/perl", 500 },
    { "\\b(use|print)\\b|\\$", 10 },
};

static const hl_lang_def_t lang = {
    .name = "pl",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_perl(void) { return &lang; }
