// atlantis_clipboard.h

#ifndef ATLANTIS_CLIPBOARD_H
#define ATLANTIS_CLIPBOARD_H

#include "atlantis_types.h"

// #region Clipboard Operations

//! Copy text to system clipboard
//! @param text text to copy
//! @param len length of text in bytes
void clipboard_copy(const char* text, size_t len);

//! Paste text from system clipboard
//! @param out_len output: length of pasted text
//! @return newly allocated string (caller must free), or NULL on failure
char* clipboard_paste(size_t* out_len);

// #endregion

#endif // ATLANTIS_CLIPBOARD_H
