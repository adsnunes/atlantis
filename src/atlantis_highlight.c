#include "atlantis_highlight.h"
#include "highlight/highlight.h"

hl_ctx_t* highlight_init(bool dark_mode)
{
    return hl_ctx_new_with_defaults(dark_mode);
}

void highlight_cleanup(hl_ctx_t* ctx)
{
    hl_ctx_free(ctx);
}

char* highlight_code(hl_ctx_t* ctx, const char* code, size_t code_len, const char* lang, size_t* out_len)
{
    if (!ctx)
        return NULL;

    const char* detected_lang = lang;
    if (!detected_lang) {
        detected_lang = hl_ctx_detect_language(ctx, code, code_len);
    }

    return hl_highlight_ex(ctx, code, code_len, detected_lang, out_len);
}

bool highlight_lang_supported(hl_ctx_t* ctx, const char* lang)
{
    if (!ctx)
        return false;
    return hl_ctx_lang_supported(ctx, lang);
}
