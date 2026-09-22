// atlantis_backend.c - Backend Context Implementation

#include "atlantis_backend.h"
#include <stdlib.h>

bool atlantis_ctx_init(AtlantisCtx* ctx, const AtlantisBackend* backend, AtlantisMode mode)
{
    if (!ctx || !backend)
        return false;
    ctx->b = backend;
    ctx->mode = mode;
    ctx->host_bg = NULL;
    if (!backend->init(mode))
        return false;
    if (backend->get_host_bg) {
        ctx->host_bg = backend->get_host_bg();
    }
    return true;
}

void atlantis_ctx_shutdown(AtlantisCtx* ctx)
{
    if (!ctx)
        return;
    if (ctx->b && ctx->b->shutdown) {
        ctx->b->shutdown();
    }
    free(ctx->host_bg);
    ctx->host_bg = NULL;
    ctx->b = NULL;
}
