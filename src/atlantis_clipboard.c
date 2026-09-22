// atlantis_clipboard.c

#include "atlantis_clipboard.h"

// #region Clipboard Operations

void clipboard_copy(const char* text, size_t len)
{
    ATLANTIS_BACKEND(app)->copy(text, len);
}

char* clipboard_paste(size_t* out_len)
{
    return ATLANTIS_BACKEND(app)->paste(out_len);
}

// #endregion
