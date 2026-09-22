// main_term.c - Terminal Frontend for Atlantis

#include "atlantis_app.h"
#include "atlantis_args.h"
#include "atlantis_backend.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
extern const AtlantisBackend atlantis_backend_win32;
#define ATLANTIS_BACKEND_PLATFORM atlantis_backend_win32
#else
extern const AtlantisBackend atlantis_backend_posix;
#define ATLANTIS_BACKEND_PLATFORM atlantis_backend_posix
#endif

int32_t main(int32_t argc, char* argv[])
{
    // Parse command-line arguments
    AtlantisArgs args = args_parse(argc, argv);

    // Handle errors
    if (args.flags & ARG_ERROR) {
        fprintf(stderr, "atlantis: %s\n", args.error_msg);
        args_print_usage(argv[0]);
        args_free(&args);
        return 1;
    }

    // Handle help/version (no backend needed)
    if (args.flags & ARG_HELP) {
        args_print_usage(argv[0]);
        args_free(&args);
        return 0;
    }

    if (args.flags & ARG_VERSION) {
        args_print_version();
        args_free(&args);
        return 0;
    }

    // Read stdin if needed (before initializing backend)
    char* stdin_content = NULL;
    size_t stdin_size = 0;
    if (args.flags & ARG_STDIN) {
        stdin_content = args_read_stdin(&stdin_size);
        if (!stdin_content || stdin_size == 0) {
            fprintf(stderr, "atlantis: no input on stdin\n");
            args_free(&args);
            return 1;
        }
    }

    // Determine mode
    AtlantisMode mode = (args.flags & ARG_PRINT)
        ? ATLANTIS_MODE_PRINT
        : ATLANTIS_MODE_INTERACTIVE;

    // Initialize context with backend
    if (!atlantis_ctx_init(&app.ctx, &ATLANTIS_BACKEND_PLATFORM, mode)) {
        fprintf(stderr, "atlantis: failed to initialize backend\n");
        free(stdin_content);
        args_free(&args);
        return 1;
    }

    // Initialize Atlantis engine. Pass -1 to honor persisted settings;
    // -t/-T flags from the user take precedence over what's on disk.
    if (!atlantis_engine_init(args.theme, args.timer_mins)) {
        fprintf(stderr, "atlantis: failed to initialize engine\n");
        atlantis_ctx_shutdown(&app.ctx);
        free(stdin_content);
        args_free(&args);
        return 1;
    }

    // Handle print mode
    if (args.flags & ARG_PRINT) {
        bool ok;
        if (stdin_content) {
            ok = atlantis_print_buffer(stdin_content, stdin_size);
        } else {
            ok = atlantis_print_document(args.file);
        }
        if (!ok) {
            fprintf(stderr, "atlantis: cannot process input\n");
            atlantis_engine_shutdown();
            atlantis_ctx_shutdown(&app.ctx);
            free(stdin_content);
            args_free(&args);
            return 1;
        }
        atlantis_engine_shutdown();
        atlantis_ctx_shutdown(&app.ctx);
        free(stdin_content);
        args_free(&args);
        return 0;
    }

    // Handle preview mode
    if (args.flags & ARG_PREVIEW) {
        bool ok;
        if (stdin_content) {
            ok = atlantis_preview_buffer(stdin_content, stdin_size);
        } else {
            ok = atlantis_preview_document(args.file);
        }
        if (!ok) {
            fprintf(stderr, "atlantis: cannot preview input\n");
            atlantis_engine_shutdown();
            atlantis_ctx_shutdown(&app.ctx);
            free(stdin_content);
            args_free(&args);
            return 1;
        }
    } else if (args.file) {
        // Edit mode: open file directly
        if (!atlantis_load_document(args.file)) {
            fprintf(stderr, "atlantis: cannot open file: %s\n", args.file);
            atlantis_engine_shutdown();
            atlantis_ctx_shutdown(&app.ctx);
            free(stdin_content);
            args_free(&args);
            return 1;
        }
    }

    free(stdin_content);
    args_free(&args);

    // Main loop (interactive mode only)
    while (atlantis_frame()) {
        ATLANTIS_BACKEND(app)->input_ready(6.944f);
        if (ATLANTIS_BACKEND(app)->poll_jobs) {
            ATLANTIS_BACKEND(app)->poll_jobs();
        }
    }

    // Cleanup
    atlantis_engine_shutdown();
    atlantis_ctx_shutdown(&app.ctx);

    return 0;
}
