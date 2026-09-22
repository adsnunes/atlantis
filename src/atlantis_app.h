// atlantis_app.h
//! Frontends (terminal, web, etc.) call these functions

#ifndef ATLANTIS_APP_H
#define ATLANTIS_APP_H

#include "atlantis_types.h"

// #region Engine Lifecycle

//! Initialize the Atlantis engine
//! Must be called after platform_init() has been called by the frontend
//! @param theme_override color theme to apply unconditionally,
//!        or -1 to fall back to persisted settings (default: dark)
//! @param timer_override timer minutes to apply unconditionally,
//!        or -1 to fall back to persisted settings (0 disables timer)
//! @return true on success
bool atlantis_engine_init(int8_t theme_override, int32_t timer_override);

//! Shutdown the Atlantis engine
//! Frontend should call platform_shutdown() after this
void atlantis_engine_shutdown(void);

// #endregion

// #region Main Loop

//! Process one frame of the application
//! Handles resize, timer, autosave, render, and input
//! @return true if app should continue, false if quitting
bool atlantis_frame(void);

//! Request the application to quit
void atlantis_request_quit(void);

//! Check if quit was requested
bool atlantis_should_quit(void);

// #endregion

// #region Document Operations

//! Load a document from path
//! @param path path to markdown file
//! @return true on success
bool atlantis_load_document(const char* path);

//! Load a document in preview (read-only) mode
//! @param path path to markdown file
//! @return true on success
bool atlantis_preview_document(const char* path);

//! Print a document (render linearly and exit)
//! @param path path to markdown file
//! @return true on success
bool atlantis_print_document(const char* path);

//! Print a buffer (render linearly and exit)
//! @param content buffer containing markdown content
//! @param size size of content buffer
//! @return true on success
bool atlantis_print_buffer(const char* content, size_t size);

//! Preview a buffer in read-only mode
//! @param content buffer containing markdown content
//! @param size size of content buffer
//! @return true on success
bool atlantis_preview_buffer(const char* content, size_t size);

//! Start a new empty document
void atlantis_new_document(void);

//! Save the current document
void atlantis_save_document(void);

// #endregion

// #region Display

//! Update display dimensions from platform
//! Call this when the platform reports a resize
void atlantis_update_size(void);

//! Render the current frame
//! Called automatically by atlantis_frame(), but can be called directly
void atlantis_render(void);

// #endregion

#endif // ATLANTIS_APP_H
