//! @file lang/sql.c
//! @brief SQL language definition

#include "../highlight.h"

static const hl_lang_rule_t rules[] = {
    { .pattern = "--.*(?:\\n|$)|/\\*(?:(?!\\*/).|[\\s\\S])*(?:\\*/)?", .token = HL_TOKEN_CMNT },
    { .pattern = "([\"'])(?:\\\\[\\s\\S]|(?!\\1)[^\\r\\n\\\\])*\\1?", .token = HL_TOKEN_STR },
    { .pattern = "\\b(?:AVG|COUNT|FIRST|FORMAT|LAST|LCASE|LEN|MAX|MID|MIN|MOD|NOW|ROUND|SUM|UCASE)(?=\\s*\\()", .token = HL_TOKEN_FUNC, .flags = HL_RULE_CASELESS },
    { .pattern = "\\b(?:SELECT|INSERT|UPDATE|DELETE|FROM|WHERE|JOIN|INNER|OUTER|LEFT|RIGHT|ON|AND|OR|NOT|NULL|IS|IN|LIKE|BETWEEN|AS|ORDER|BY|GROUP|HAVING|UNION|ALL|DISTINCT|INTO|VALUES|SET|CREATE|TABLE|INDEX|VIEW|DROP|ALTER|ADD|COLUMN|PRIMARY|KEY|FOREIGN|REFERENCES|CONSTRAINT|DEFAULT|CHECK|UNIQUE|DATABASE|SCHEMA|IF|EXISTS|CASE|WHEN|THEN|ELSE|END|LIMIT|OFFSET|ASC|DESC)\\b", .token = HL_TOKEN_KWD, .flags = HL_RULE_CASELESS },
    { .pattern = "-?\\d+(?:\\.\\d+)?", .token = HL_TOKEN_NUM },
    { .pattern = "\\b(?:TRUE|FALSE)\\b", .token = HL_TOKEN_BOOL, .flags = HL_RULE_CASELESS },
    { .pattern = "[+\\-*/%=<>!&|^]+", .token = HL_TOKEN_OPER },
    { .pattern = "@\\S+", .token = HL_TOKEN_VAR },
};

static const hl_detect_rule_t detect[] = {
    { "\\b(SELECT|INSERT|FROM)\\b", 50 },
};

static const hl_lang_def_t lang = {
    .name = "sql",
    .rules = rules,
    .rule_count = sizeof(rules) / sizeof(rules[0]),
    .default_token = HL_TOKEN_NONE,
    .detect = detect,
    .detect_count = sizeof(detect) / sizeof(detect[0]),
};

const hl_lang_def_t* hl_lang_sql(void) { return &lang; }
