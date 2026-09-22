// atlantis_gap.h
//! The gap buffer keeps a "gap" in the buffer at the cursor position,
//! making insertions and deletions at that position very fast.

#ifndef ATLANTIS_GAP_H
#define ATLANTIS_GAP_H

#include "atlantis_types.h"

// #region Core Operations

//! Initialize a gap buffer with given capacity
//! @param gb gap buffer to initialize
//! @param size initial buffer capacity in bytes
void gap_init(GapBuffer* gb, size_t size);

//! Free gap buffer memory
//! @param gb gap buffer to free
void gap_free(GapBuffer* gb);

//! Get length of text in buffer (excluding gap)
//! @param gb gap buffer to query
//! @return number of bytes of text content
size_t gap_len(const GapBuffer* gb);

//! Insert a single character at position
//! @param gb gap buffer to modify
//! @param pos byte position to insert at
//! @param c character to insert
void gap_insert(GapBuffer* gb, size_t pos, char c);

//! Insert a string at position
//! @param gb gap buffer to modify
//! @param pos byte position to insert at
//! @param s string to insert
//! @param n number of bytes to insert
void gap_insert_str(GapBuffer* gb, size_t pos, const char* s, size_t n);

//! Delete characters at position
//! @param gb gap buffer to modify
//! @param pos byte position to delete from
//! @param n number of bytes to delete
void gap_delete(GapBuffer* gb, size_t pos, size_t n);

//! Get character at position
//! @param gb gap buffer to query
//! @param pos byte position
//! @return character at position, or '\0' if out of bounds
char gap_at(const GapBuffer* gb, size_t pos);

//! Convert entire buffer to null-terminated string
//! @param gb gap buffer to convert
//! @return newly allocated string (caller must free)
char* gap_to_str(const GapBuffer* gb);

//! Extract substring from buffer
//! @param gb gap buffer to extract from
//! @param start start byte position (inclusive)
//! @param end end byte position (exclusive)
//! @return newly allocated string (caller must free)
char* gap_substr(const GapBuffer* gb, size_t start, size_t end);

//! Copy bytes from gap buffer to regular buffer
//! @param gb gap buffer to copy from
//! @param start start byte position
//! @param count number of bytes to copy
//! @param dest destination buffer (must have space for count bytes)
void gap_copy_to(const GapBuffer* gb, size_t start, size_t count, char* dest);

// #endregion

// #region UTF-8 Operations

//! Get byte position of previous UTF-8 character
//! @param gb gap buffer to navigate
//! @param pos current byte position
//! @return byte position of previous character start
size_t gap_utf8_prev(const GapBuffer* gb, size_t pos);

//! Get byte position after current UTF-8 character
//! @param gb gap buffer to navigate
//! @param pos current byte position
//! @return byte position after current character
size_t gap_utf8_next(const GapBuffer* gb, size_t pos);

//! Get Unicode codepoint at position
//! @param gb gap buffer to query
//! @param pos byte position
//! @param char_len output: byte length of character (optional, may be NULL)
//! @return Unicode codepoint, or -1 if invalid/out of bounds
int32_t gap_utf8_at(const GapBuffer* gb, size_t pos, size_t* char_len);

// #endregion

#endif // ATLANTIS_GAP_H
