// atlantis_theme.c

#include "atlantis_theme.h"

// #region AtlantisColor Palettes

//! Light theme - warm paper aesthetic
static const AtlantisColor LIGHT_BG = { 238, 247, 249 }; //!< Sea-glass background
static const AtlantisColor LIGHT_FG = { 24, 45, 52 }; //!< Deep blue ink
static const AtlantisColor LIGHT_DIM = { 103, 135, 143 }; //!< Muted tide annotations
static const AtlantisColor LIGHT_ACCENT = { 0, 132, 151 }; //!< Lagoon accent
static const AtlantisColor LIGHT_SELECT = { 208, 239, 239 }; //!< Shallow-water highlight
static const AtlantisColor LIGHT_AI_BG = { 229, 242, 245 }; //!< Subtle AI panel
static const AtlantisColor LIGHT_BORDER = { 188, 219, 224 }; //!< Soft wave borders
static const AtlantisColor LIGHT_CODE_BG = { 224, 239, 242 }; //!< Code block background
static const AtlantisColor LIGHT_MODAL_BG = { 247, 252, 253 }; //!< Modal popup background

//! Dark theme - deep ocean aesthetic
static const AtlantisColor DARK_BG = { 7, 22, 30 }; //!< Midnight water background
static const AtlantisColor DARK_FG = { 211, 237, 239 }; //!< Moonlit text
static const AtlantisColor DARK_DIM = { 91, 143, 151 }; //!< Distant tide annotations
static const AtlantisColor DARK_ACCENT = { 52, 207, 196 }; //!< Bioluminescent accent
static const AtlantisColor DARK_SELECT = { 15, 63, 73 }; //!< Deep-water highlight
static const AtlantisColor DARK_AI_BG = { 11, 33, 42 }; //!< Slightly lighter AI panel
static const AtlantisColor DARK_BORDER = { 24, 74, 82 }; //!< Submerged borders
static const AtlantisColor DARK_CODE_BG = { 10, 31, 39 }; //!< Code block background
static const AtlantisColor DARK_MODAL_BG = { 12, 39, 48 }; //!< Modal popup background

// #endregion

// #region Output Primitives

void set_fg(AtlantisColor c)
{
    ATLANTIS_BACKEND(app)->set_fg(c);
}

void set_bg(AtlantisColor c)
{
    ATLANTIS_BACKEND(app)->set_bg(c);
}

void move_to(int32_t r, int32_t c)
{
    ATLANTIS_BACKEND(app)->set_cursor(c, r); // Note: backend uses (col, row) order
}

void out_str(const char* str)
{
    ATLANTIS_BACKEND(app)->write_str(str, strlen(str));
}

void out_str_n(const char* str, size_t len)
{
    ATLANTIS_BACKEND(app)->write_str(str, len);
}

void out_char(char c)
{
    ATLANTIS_BACKEND(app)->write_char(c);
}

void out_spaces(int32_t n)
{
    ATLANTIS_BACKEND(app)->repeat_char(' ', n);
}

void out_int(int32_t value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    out_str(buf);
}

void out_flush(void)
{
    ATLANTIS_BACKEND(app)->flush();
}

void clear_screen(void)
{
    ATLANTIS_BACKEND(app)->clear_screen();
}

void clear_line(void)
{
    ATLANTIS_BACKEND(app)->clear_line();
}

void clear_range(int32_t n)
{
    ATLANTIS_BACKEND(app)->clear_range(n);
}

void cursor_visible(bool visible)
{
    ATLANTIS_BACKEND(app)->set_cursor_visible(visible);
}

void cursor_home(void)
{
    move_to(1, 1);
}

void sync_begin(void)
{
    ATLANTIS_BACKEND(app)->sync_begin();
}

void sync_end(void)
{
    ATLANTIS_BACKEND(app)->sync_end();
}

void fill_line_end(AtlantisColor bg)
{
    // Only applies in print mode
    if (app.ctx.mode != ATLANTIS_MODE_PRINT)
        return;

    int32_t cols = 0, rows = 0;
    ATLANTIS_BACKEND(app)->get_size(&cols, &rows);
    if (cols <= 0)
        cols = 80;

    // Set background and fill to end of line
    set_bg(bg);
    // Use CSI K (erase to end of line) which uses current bg
    ATLANTIS_BACKEND(app)->write_str("\x1b[K", 3);
}

// #endregion

// #region Theme Colors

AtlantisColor get_bg(void)
{
    if (app.ctx.mode == ATLANTIS_MODE_PRINT && app.ctx.host_bg) {
        return *app.ctx.host_bg;
    }
    return app.theme == THEME_DARK ? DARK_BG : LIGHT_BG;
}
AtlantisColor get_fg(void) { return app.theme == THEME_DARK ? DARK_FG : LIGHT_FG; }
AtlantisColor get_dim(void) { return app.theme == THEME_DARK ? DARK_DIM : LIGHT_DIM; }
AtlantisColor get_accent(void) { return app.theme == THEME_DARK ? DARK_ACCENT : LIGHT_ACCENT; }
AtlantisColor get_select(void) { return app.theme == THEME_DARK ? DARK_SELECT : LIGHT_SELECT; }
AtlantisColor get_ai_bg(void) { return app.theme == THEME_DARK ? DARK_AI_BG : LIGHT_AI_BG; }
AtlantisColor get_border(void) { return app.theme == THEME_DARK ? DARK_BORDER : LIGHT_BORDER; }
AtlantisColor get_code_bg(void) { return app.theme == THEME_DARK ? DARK_CODE_BG : LIGHT_CODE_BG; }
AtlantisColor get_modal_bg(void) { return app.theme == THEME_DARK ? DARK_MODAL_BG : LIGHT_MODAL_BG; }

// #endregion

// #region AtlantisColor Utilities

AtlantisColor color_lerp(AtlantisColor a, AtlantisColor b, float t)
{
    return (AtlantisColor) {
        (uint8_t)(a.r + (b.r - a.r) * t),
        (uint8_t)(a.g + (b.g - a.g) * t),
        (uint8_t)(a.b + (b.b - a.b) * t)
    };
}

// #endregion

// #region Text Attributes

void set_bold(bool on)
{
    ATLANTIS_BACKEND(app)->set_bold(on);
}

void set_italic(bool on)
{
    ATLANTIS_BACKEND(app)->set_italic(on);
}

void set_dim(bool on)
{
    ATLANTIS_BACKEND(app)->set_dim(on);
}

void set_strikethrough(bool on)
{
    ATLANTIS_BACKEND(app)->set_strike(on);
}

void reset_attrs(void)
{
    ATLANTIS_BACKEND(app)->reset_attrs();
}

// #endregion

// #region Styled Text

void set_underline(UnderlineStyle style)
{
    ATLANTIS_BACKEND(app)->set_underline(style);
}

void set_underline_color(AtlantisColor c)
{
    ATLANTIS_BACKEND(app)->set_underline_color(c);
}

void clear_underline(void)
{
    ATLANTIS_BACKEND(app)->clear_underline();
}

// #endregion

// #region Text Sizing

void print_scaled_char(char c, int32_t scale)
{
    if (scale <= 1 || !atlantis_ctx_has(&app.ctx, ATLANTIS_CAP_TEXT_SIZING)) {
        ATLANTIS_BACKEND(app)->write_char(c);
        return;
    }
    char str[2] = { c, '\0' };
    ATLANTIS_BACKEND(app)->write_scaled(str, 1, scale);
}

void print_scaled_str(const char* str, size_t len, int32_t scale)
{
    if (scale <= 1 || !atlantis_ctx_has(&app.ctx, ATLANTIS_CAP_TEXT_SIZING)) {
        ATLANTIS_BACKEND(app)->write_str(str, len);
        return;
    }
    ATLANTIS_BACKEND(app)->write_scaled(str, len, scale);
}

void print_scaled_frac_char(char c, int32_t scale, int32_t num, int32_t denom)
{
    // No scaling needed if scale is 1 with no fractional part, or no text sizing support
    if ((scale <= 1 && (num == 0 || denom == 0)) || !atlantis_ctx_has(&app.ctx, ATLANTIS_CAP_TEXT_SIZING)) {
        ATLANTIS_BACKEND(app)->write_char(c);
        return;
    }
    if (ATLANTIS_BACKEND(app)->write_scaled_frac) {
        char str[2] = { c, '\0' };
        ATLANTIS_BACKEND(app)->write_scaled_frac(str, 1, scale, num, denom);
    } else if (scale > 1) {
        // Fallback to integer scaling if fractional not supported
        char str[2] = { c, '\0' };
        ATLANTIS_BACKEND(app)->write_scaled(str, 1, scale);
    } else {
        ATLANTIS_BACKEND(app)->write_char(c);
    }
}

void print_scaled_frac_str(const char* str, size_t len, int32_t scale, int32_t num, int32_t denom)
{
    // No scaling needed if scale is 1 with no fractional part, or no text sizing support
    if ((scale <= 1 && (num == 0 || denom == 0)) || !atlantis_ctx_has(&app.ctx, ATLANTIS_CAP_TEXT_SIZING)) {
        ATLANTIS_BACKEND(app)->write_str(str, len);
        return;
    }
    if (ATLANTIS_BACKEND(app)->write_scaled_frac) {
        ATLANTIS_BACKEND(app)->write_scaled_frac(str, len, scale, num, denom);
    } else if (scale > 1) {
        // Fallback to integer scaling if fractional not supported
        ATLANTIS_BACKEND(app)->write_scaled(str, len, scale);
    } else {
        ATLANTIS_BACKEND(app)->write_str(str, len);
    }
}

// #endregion
