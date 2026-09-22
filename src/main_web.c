// main_web.c - Web Frontend for Atlantis
//! Emscripten/Canvas implementation that uses the Atlantis engine
//! This file handles platform initialization and the main loop via requestAnimationFrame

#ifdef __EMSCRIPTEN__

#include "atlantis_app.h"
#include "atlantis_backend.h"
#include "atlantis_types.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>

extern const AtlantisBackend atlantis_backend_web;

// Frame callback for requestAnimationFrame loop
static void main_loop(void)
{
    if (!atlantis_frame()) {
        // App wants to quit
        emscripten_cancel_main_loop();
        atlantis_engine_shutdown();
        atlantis_ctx_shutdown(&app.ctx);
    }
}

// Called from JavaScript to load a file
EMSCRIPTEN_KEEPALIVE
void atlantis_web_load_file(const char* content, size_t len, const char* filename)
{
    // Save to virtual filesystem and load
    char path[256];
    snprintf(path, sizeof(path), "/atlantis/%s", filename);

    // Write content to virtual FS
    FILE* f = fopen(path, "w");
    if (f) {
        fwrite(content, 1, len, f);
        fclose(f);
        atlantis_load_document(path);
    }
}

// Called from JavaScript to create a new document
EMSCRIPTEN_KEEPALIVE
void atlantis_web_new_document(void)
{
    atlantis_new_document();
}

// Called from JavaScript to save the current document
EMSCRIPTEN_KEEPALIVE
void atlantis_web_save(void)
{
    atlantis_save_document();
}

// Called from JavaScript to set the theme
EMSCRIPTEN_KEEPALIVE
void atlantis_web_set_theme(int32_t dark)
{
    // Theme is set at init, would need to add a runtime theme change API
    (void)dark;
}

int32_t main(int32_t argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // Initialize backend context
    if (!atlantis_ctx_init(&app.ctx, &atlantis_backend_web, ATLANTIS_MODE_INTERACTIVE)) {
        fprintf(stderr, "atlantis: failed to initialize backend\n");
        return 1;
    }

    // Initialize Atlantis engine; the web build doesn't expose flags,
    // so let the engine load any persisted settings (defaults to dark).
    if (!atlantis_engine_init(-1, -1)) {
        fprintf(stderr, "atlantis: failed to initialize engine\n");
        atlantis_ctx_shutdown(&app.ctx);
        return 1;
    }

    // Start the main loop using requestAnimationFrame
    // 0 = use requestAnimationFrame (vsync)
    // 1 = simulate infinite loop (not recommended)
    emscripten_set_main_loop(main_loop, 0, 1);

    // This is never reached because emscripten_set_main_loop doesn't return
    // when the last parameter is 1 (simulate infinite loop)

    return 0;
}

#endif // __EMSCRIPTEN__
