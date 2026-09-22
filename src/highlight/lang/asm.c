//! @file lang/asm.c
//! @brief Assembly language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "(;|#).*", .token = HL_TOKEN_CMNT },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "(?:\\.e?|\\b)\\d(?:e-|[\\d.oxa-fA-F_])*(?:\\.|\\b)", .token = HL_TOKEN_NUM },
    { .pattern = "\\$[\\da-fA-F]*\\b", .token = HL_TOKEN_NUM },
    { .pattern = "^[a-z]+\\s+[a-z.]+\\b", .token = HL_TOKEN_KWD, .flags = HL_RULE_MULTILINE },
    { .pattern = "^\\t*[a-z][a-z\\d]*\\b", .token = HL_TOKEN_KWD, .flags = HL_RULE_MULTILINE },
    { .pattern = "%|\\$", .token = HL_TOKEN_OPER },
};

static const hl_detect_rule_t detect[] = {
    { "^(section|global main|extern|\\t(call|mov|ret))\\b", 100 },
};

static const hl_lang_def_t lang = {
    .name = "asm",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_asm(void) { return &lang; }
