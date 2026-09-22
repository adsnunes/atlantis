// atlantis_backend.h - Backend Abstraction Layer

#ifndef ATLANTIS_BACKEND_H
#define ATLANTIS_BACKEND_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "atlantis_support.h"

// #region Types

typedef struct {
    uint8_t r, g, b;
} AtlantisColor;

ATLANTIS_ENUM(uint8_t) {
    ATLANTIS_MODE_INTERACTIVE = 0,
    ATLANTIS_MODE_PRINT = 1
} AtlantisMode;

ATLANTIS_ENUM(uint16_t) {
    ATLANTIS_CAP_NONE = 0,
    ATLANTIS_CAP_TRUE_COLOR = 1 << 0,
    ATLANTIS_CAP_SYNC_OUTPUT = 1 << 1,
    ATLANTIS_CAP_STYLED_UNDERLINE = 1 << 2,
    ATLANTIS_CAP_TEXT_SIZING = 1 << 3,
    ATLANTIS_CAP_IMAGES = 1 << 4,
    ATLANTIS_CAP_MOUSE = 1 << 5,
    ATLANTIS_CAP_BRACKETED_PASTE = 1 << 6,
    ATLANTIS_CAP_FOCUS_EVENTS = 1 << 7,
    ATLANTIS_CAP_CLIPBOARD = 1 << 8,
} AtlantisCap;

ATLANTIS_ENUM(uint8_t) {
    ATLANTIS_UNDERLINE_SINGLE,
    ATLANTIS_UNDERLINE_CURLY,
    ATLANTIS_UNDERLINE_DOTTED,
    ATLANTIS_UNDERLINE_DASHED
} AtlantisUnderline;

ATLANTIS_ENUM(int32_t) {
    ATLANTIS_KEY_NONE = -1,
    ATLANTIS_KEY_ESC = 0x1b,
    ATLANTIS_KEY_UP = 0x110000, // Above Unicode max (0x10FFFF) to not conflict with codepoints
    ATLANTIS_KEY_DOWN,
    ATLANTIS_KEY_RIGHT,
    ATLANTIS_KEY_LEFT,
    ATLANTIS_KEY_HOME,
    ATLANTIS_KEY_END,
    ATLANTIS_KEY_PGUP,
    ATLANTIS_KEY_PGDN,
    ATLANTIS_KEY_DEL,
    ATLANTIS_KEY_SHIFT_UP,
    ATLANTIS_KEY_SHIFT_DOWN,
    ATLANTIS_KEY_SHIFT_LEFT,
    ATLANTIS_KEY_SHIFT_RIGHT,
    ATLANTIS_KEY_CTRL_LEFT,
    ATLANTIS_KEY_CTRL_RIGHT,
    ATLANTIS_KEY_CTRL_SHIFT_LEFT,
    ATLANTIS_KEY_CTRL_SHIFT_RIGHT,
    ATLANTIS_KEY_ALT_LEFT,
    ATLANTIS_KEY_ALT_RIGHT,
    ATLANTIS_KEY_ALT_SHIFT_LEFT,
    ATLANTIS_KEY_ALT_SHIFT_RIGHT,
    ATLANTIS_KEY_ALT_UP,
    ATLANTIS_KEY_ALT_DOWN,
    ATLANTIS_KEY_CTRL_HOME,
    ATLANTIS_KEY_CTRL_END,
    ATLANTIS_KEY_MOUSE_SCROLL_UP,
    ATLANTIS_KEY_MOUSE_SCROLL_DOWN,
    ATLANTIS_KEY_MOUSE_CLICK,
    ATLANTIS_KEY_BTAB
} AtlantisKey;

typedef struct {
    int16_t x, y; //!< Mouse position (max 32K)
    uint8_t button; //!< Button number
    uint8_t flags; //!< bit0=pressed, bit1=scroll_up, bit2=scroll_down
} AtlantisMouse;

#define ATLANTIS_MOUSE_PRESSED 0x01
#define ATLANTIS_MOUSE_SCROLL_UP 0x02
#define ATLANTIS_MOUSE_SCROLL_DOWN 0x04

typedef struct {
    int16_t year; //!< Year (e.g., 2024)
    uint8_t mon, mday; //!< Month (0-11), day (1-31)
    uint8_t hour, min, sec; //!< Time components
    uint8_t wday; //!< Day of week (0-6)
} AtlantisTime;

//! ISO 8601 date/time for parsing input strings
typedef struct {
    int16_t year; //!< Year (e.g., 2024)
    uint8_t mon, mday; //!< Month (1-12), day (1-31)
    uint8_t hour, min, sec; //!< Time components
    uint16_t ms; //!< Milliseconds (0-999)
    bool has_time : 1; //!< True if time component present
    bool has_tz : 1; //!< True if timezone present
    char tz[8]; //!< Timezone: "Z" or "+05:30" etc
} AtlantisDate;

ATLANTIS_ENUM(uint8_t) {
    ATLANTIS_CLOCK_SEC, //!< Seconds since epoch
    ATLANTIS_CLOCK_MS //!< Milliseconds (monotonic)
} AtlantisClock;

// #endregion

// #region Backend Interface

typedef struct AtlantisBackend {
    const char* name;

    // Lifecycle
    bool (*init)(AtlantisMode mode);
    void (*shutdown)(void);
    uint32_t (*get_caps)(void);
    AtlantisColor* (*get_host_bg)(void); // Returns malloc'd color or NULL

    // Display
    void (*get_size)(int32_t* cols, int32_t* rows);
    void (*set_cursor)(int32_t col, int32_t row);
    void (*set_cursor_visible)(bool visible);
    void (*set_fg)(AtlantisColor c);
    void (*set_bg)(AtlantisColor c);
    void (*reset_attrs)(void);
    void (*set_bold)(bool on);
    void (*set_italic)(bool on);
    void (*set_dim)(bool on);
    void (*set_strike)(bool on);
    void (*set_underline)(AtlantisUnderline style);
    void (*set_underline_color)(AtlantisColor c);
    void (*clear_underline)(void);
    void (*clear_screen)(void);
    void (*clear_line)(void);
    void (*clear_range)(int32_t count); //!< Erase N chars at cursor using current bg
    void (*write_str)(const char* s, size_t len);
    void (*write_char)(char c);
    void (*repeat_char)(char c, int32_t n);
    void (*write_scaled)(const char* s, size_t len, int32_t scale);
    void (*write_scaled_frac)(const char* s, size_t len, int32_t scale, int32_t num, int32_t denom);
    void (*flush)(void);
    void (*sync_begin)(void);
    void (*sync_end)(void);
    void (*set_title)(const char* title);
    void (*link_begin)(const char* url);
    void (*link_end)(void);

    // Input
    int32_t (*read_key)(void);
    int32_t (*mouse_col)(void);
    int32_t (*mouse_row)(void);
    bool (*check_resize)(void);
    bool (*check_quit)(void);
    void (*poll_jobs)(void);
    bool (*input_ready)(float timeout_ms);
    void (*register_signals)(void (*on_resize)(int32_t), void (*on_quit)(int32_t));

    // Clipboard
    void (*copy)(const char* text, size_t len);
    char* (*paste)(size_t* out_len);

    // Filesystem
    const char* (*home_dir)(void);
    bool (*mkdir_p)(const char* path);
    bool (*file_exists)(const char* path);
    char* (*read_file)(const char* path, size_t* out_len);
    bool (*write_file)(const char* path, const char* data, size_t len);
    bool (*list_dir)(const char* path, char*** names, int32_t* count);
    int64_t (*mtime)(const char* path);
    bool (*rm)(const char* path);
    void (*reveal)(const char* path);

    // Shutdown events
    void (*on_shutdown)(void (*callback)(void));

    // Time
    int64_t (*clock)(AtlantisClock kind);
    void (*sleep_ms)(int32_t ms);
    void (*localtime)(AtlantisTime* out);
    void (*localtime_from)(AtlantisTime* out, int64_t timestamp);
    const char* (*username)(void);

    // Images
    bool (*img_supported)(const char* path);
    bool (*img_size)(const char* path, int32_t* w, int32_t* h);
    int32_t (*img_display)(const char* path, int32_t row, int32_t col, int32_t max_cols, int32_t max_rows);
    int32_t (*img_display_cropped)(const char* path, int32_t row, int32_t col, int32_t max_cols, int32_t crop_top, int32_t visible);
    void (*img_frame_start)(void);
    void (*img_frame_end)(void);
    void (*img_clear_all)(void);
    void (*img_mask)(int32_t col, int32_t row, int32_t cols, int32_t rows, AtlantisColor bg);
    bool (*img_resolve)(const char* raw, const char* base_dir, char* out, size_t out_size);
    int32_t (*img_calc_rows)(int32_t pw, int32_t ph, int32_t max_cols, int32_t max_rows);
    void (*img_invalidate)(const char* path);

} AtlantisBackend;

// #endregion

// #region Context

typedef struct AtlantisCtx {
    const AtlantisBackend* b;
    AtlantisMode mode;
    AtlantisColor* host_bg;
} AtlantisCtx;

bool atlantis_ctx_init(AtlantisCtx* ctx, const AtlantisBackend* backend, AtlantisMode mode);
void atlantis_ctx_shutdown(AtlantisCtx* ctx);

static inline bool atlantis_ctx_has(const AtlantisCtx* ctx, AtlantisCap cap)
{
    return (ctx->b->get_caps() & (uint32_t)cap) != 0;
}

//! Shorthand for accessing backend from app struct
#define ATLANTIS_BACKEND(app) ((app).ctx.b)

// #endregion

#endif // ATLANTIS_BACKEND_H
