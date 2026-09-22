// atlantis.h

#ifndef ATLANTIS_H
#define ATLANTIS_H

#include "atlantis_types.h"

//! Initialize the Atlantis application
//! @param theme initial theme (THEME_LIGHT or THEME_DARK)
//! @return true on success
bool atlantis_init(Theme theme);

//! Load a file for editing
//! @param path path to file
//! @return true on success
bool atlantis_load_file(const char* path);

//! Run one iteration of the main loop
//! @return true to continue, false to quit
bool atlantis_update(void);

//! Shutdown the Atlantis application
void atlantis_shutdown(void);

//! Handle window resize signal
void atlantis_on_resize(void);

//! Handle quit signal (SIGINT, SIGTERM, etc.)
void atlantis_on_quit(void);

//! Global application state (defined in atlantis.c)
extern App app;

#endif // ATLANTIS_H
