//! @file lang/xml.c
//! @brief XML language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "<!--(?:(?!-->)[\\s\\S])*-->", .token = HL_TOKEN_CMNT },
    { .pattern = "<!\\[CDATA\\[[\\s\\S]*?\\]\\]>", .token = HL_TOKEN_CLASS, .flags = HL_RULE_CASELESS },
    { .pattern = "</?[a-zA-Z][\\w:-]*", .token = HL_TOKEN_VAR },
    { .pattern = "[a-zA-Z][\\w:-]*(?=\\s*=)", .token = HL_TOKEN_CLASS },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "/?>", .token = HL_TOKEN_OPER },
    { .pattern = "&(?:#x?)?[\\da-z]{1,8};", .token = HL_TOKEN_VAR, .flags = HL_RULE_CASELESS },
    { .pattern = "<\\?[a-zA-Z][\\w:-]*(?:[^?]|\\?[^?>])*\\?+>", .token = HL_TOKEN_STR },
};

static const hl_detect_rule_t detect[] = {
    { "^<\\?xml", 500 },
    { "</?[a-z-]+[^\\n>]*>", 10 },
};

static const hl_lang_def_t lang = {
    .name = "xml",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_xml(void) { return &lang; }
