// atlantis_svg.h - SVG parsing and rasterization

#ifndef ATLANTIS_SVG_H
#define ATLANTIS_SVG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//! Check if a file path is an SVG file (by extension)
bool svg_is_svg_file(const char* path);

//! Rasterize SVG data to RGBA pixels
//! @param svg_data null-terminated SVG data (will be modified by parser)
//! @param out_pixels output: malloc'd RGBA pixel buffer (caller must free)
//! @param out_width output: image width in pixels
//! @param out_height output: image height in pixels
//! @return true on success
bool svg_rasterize(char* svg_data, uint8_t** out_pixels, int32_t* out_width, int32_t* out_height);

#endif // ATLANTIS_SVG_H
