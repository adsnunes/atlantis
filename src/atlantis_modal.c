// atlantis_modal.c - Modal form rendering helpers

#include "atlantis_modal.h"
#include <string.h>

void _modal_write_str(const char* str)
{
    ATLANTIS_BACKEND(app)->write_str(str, strlen(str));
}

void _modal_write_char(char c)
{
    ATLANTIS_BACKEND(app)->write_char(c);
}

void _modal_set_cursor_visible(bool visible)
{
    ATLANTIS_BACKEND(app)->set_cursor_visible(visible);
}
