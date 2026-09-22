//! @file lang/css.c
//! @brief CSS language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "/\\*(?:(?!\\*/)[\\s\\S])*(?:\\*/)?", .token = HL_TOKEN_CMNT },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "@\\w+\\b|\\b(?:and|not|only|or)\\b", .token = HL_TOKEN_KWD },
    { .pattern = "[.#:][\\w-]+", .token = HL_TOKEN_VAR },
    { .pattern = "[\\w-]+(?=\\s*:)", .token = HL_TOKEN_VAR },
    { .pattern = "#[\\da-fA-F]{3,8}\\b", .token = HL_TOKEN_NUM },
    { .pattern = "-?\\d+(?:\\.\\d+)?(?:cm|mm|in|px|pt|pc|em|ex|ch|rem|vw|vh|vmin|vmax|%)?", .token = HL_TOKEN_NUM },
    { .pattern = "[a-zA-Z]\\w*(?=\\s*\\()", .token = HL_TOKEN_FUNC },
    { .pattern = "\\b[a-z-]+\\b", .token = HL_TOKEN_NUM },
};

static const hl_detect_rule_t detect[] = {
    { "^(@import|@page|@media|(\\.|#)[a-z]+)", 20 },
};

static const hl_lang_def_t lang = {
    .name = "css",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_css(void) { return &lang; }
