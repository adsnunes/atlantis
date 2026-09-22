// atlantis_highlight.h

#ifndef ATLANTIS_HIGHLIGHT_H
#define ATLANTIS_HIGHLIGHT_H

#include <stdbool.h>
#include <stddef.h>

typedef struct hl_ctx hl_ctx_t;

hl_ctx_t* highlight_init(bool dark_mode);
void highlight_cleanup(hl_ctx_t* ctx);

char* highlight_code(hl_ctx_t* ctx, const char* code, size_t code_len, const char* lang, size_t* out_len);
bool highlight_lang_supported(hl_ctx_t* ctx, const char* lang);

#endif // ATLANTIS_HIGHLIGHT_H
