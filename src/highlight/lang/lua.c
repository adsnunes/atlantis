//! @file lang/lua.c
//! @brief Lua language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "^#!.*|--(?:\\[(=*)\\[(?:(?!--\\]\\1\\])[\\s\\S])*--\\]\\1\\]|.*)", .token = HL_TOKEN_CMNT },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "\\b(?:and|break|do|else|elseif|end|for|function|if|in|local|not|or|repeat|return|then|until|while)\\b", .token = HL_TOKEN_KWD },
    { .pattern = "\\b(?:true|false|nil)\\b", .token = HL_TOKEN_BOOL },
    { .pattern = "[+*/%^#=~<>:,.-]+", .token = HL_TOKEN_OPER },
    { .pattern = "(?:\\.e?|\\b)\\d(?:e-|[\\d.oxa-fA-F_])*(?:\\.|\\b)", .token = HL_TOKEN_NUM },
    { .pattern = "[a-z_]+(?=\\s*[({])", .token = HL_TOKEN_FUNC },
};

static const hl_detect_rule_t detect[] = {
    { "#!(/usr)?/bin/lua", 500 },
};

static const hl_lang_def_t lang = {
    .name = "lua",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_lua(void) { return &lang; }
