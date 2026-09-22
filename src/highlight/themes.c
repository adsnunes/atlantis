//! @file themes.c
//! @brief ANSI color theme definitions

#include "highlight.h"

#define ANSI_RESET "\x1b[0m"
#define ANSI_BLACK "\x1b[30m"
#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_WHITE "\x1b[37m"
#define ANSI_GRAY "\x1b[90m"

static const hl_theme_t default_theme = {
    .name = "default",
    .colors = {
        [HL_TOKEN_NONE] = "",
        [HL_TOKEN_DELETED] = ANSI_RED,
        [HL_TOKEN_ERR] = ANSI_RED,
        [HL_TOKEN_VAR] = ANSI_RED,
        [HL_TOKEN_SECTION] = ANSI_MAGENTA,
        [HL_TOKEN_KWD] = ANSI_RED,
        [HL_TOKEN_CLASS] = ANSI_YELLOW,
        [HL_TOKEN_CMNT] = ANSI_GRAY,
        [HL_TOKEN_INSERT] = ANSI_GREEN,
        [HL_TOKEN_TYPE] = ANSI_BLUE,
        [HL_TOKEN_FUNC] = ANSI_MAGENTA,
        [HL_TOKEN_BOOL] = ANSI_CYAN,
        [HL_TOKEN_NUM] = ANSI_YELLOW,
        [HL_TOKEN_OPER] = ANSI_BLUE,
        [HL_TOKEN_STR] = ANSI_GREEN,
        [HL_TOKEN_ESC] = ANSI_CYAN,
    },
    .reset = ANSI_RESET
};

const hl_theme_t* hl_theme_default(void)
{
    return &default_theme;
}

static const hl_theme_t atom_dark_theme = {
    .name = "atom-dark",
    .colors = {
        [HL_TOKEN_NONE] = "",
        [HL_TOKEN_DELETED] = ANSI_RED,
        [HL_TOKEN_ERR] = ANSI_RED,
        [HL_TOKEN_VAR] = ANSI_RED,
        [HL_TOKEN_SECTION] = ANSI_MAGENTA,
        [HL_TOKEN_KWD] = ANSI_MAGENTA,
        [HL_TOKEN_CLASS] = ANSI_YELLOW,
        [HL_TOKEN_CMNT] = ANSI_GRAY,
        [HL_TOKEN_INSERT] = ANSI_GREEN,
        [HL_TOKEN_TYPE] = ANSI_CYAN,
        [HL_TOKEN_FUNC] = ANSI_BLUE,
        [HL_TOKEN_BOOL] = ANSI_YELLOW,
        [HL_TOKEN_NUM] = ANSI_YELLOW,
        [HL_TOKEN_OPER] = ANSI_BLUE,
        [HL_TOKEN_STR] = ANSI_GREEN,
        [HL_TOKEN_ESC] = ANSI_CYAN,
    },
    .reset = ANSI_RESET
};

const hl_theme_t* hl_theme_atom_dark(void)
{
    return &atom_dark_theme;
}
