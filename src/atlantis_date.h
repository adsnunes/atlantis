// atlantis_date.h - Cross-platform ISO 8601 date parsing

#ifndef ATLANTIS_DATE_H
#define ATLANTIS_DATE_H

#include "atlantis_backend.h"

//! Parse ISO 8601 date/time string
//! Handles: YYYY-MM-DD, YYYY-MM-DDTHH:MM:SS[.sss][Z|+HH:MM|-HH:MM]
//! @param str Input string to parse
//! @param out Output AtlantisDate structure
//! @return true on success, false on parse error
bool atlantis_parse_iso_date(const char* str, AtlantisDate* out);

//! Format AtlantisDate to ISO 8601 string
//! Output format depends on fields: YYYY-MM-DD or YYYY-MM-DDTHH:MM:SS[.sss][tz]
//! @param d Input AtlantisDate structure
//! @param buf Output buffer
//! @param size Size of output buffer (recommend 32+ bytes)
void atlantis_format_iso_date(const AtlantisDate* d, char* buf, size_t size);

//! Format AtlantisTime to ISO 8601 string with Z suffix
//! Output format: YYYY-MM-DDTHH:MM:SSZ
//! @param t Input AtlantisTime structure
//! @param buf Output buffer
//! @param size Size of output buffer (recommend 24+ bytes)
void atlantis_format_iso_time(const AtlantisTime* t, char* buf, size_t size);

//! Format AtlantisTime as filename-safe timestamp
//! Output format: YYYY-MM-DD_HHMMSS
//! @param t Input AtlantisTime structure
//! @param buf Output buffer
//! @param size Size of output buffer (recommend 20+ bytes)
void atlantis_format_filename_time(const AtlantisTime* t, char* buf, size_t size);

//! Format AtlantisTime as human-readable date
//! Output format: "Mon D, YYYY at H:MM"
//! @param t Input AtlantisTime structure
//! @param buf Output buffer
//! @param size Size of output buffer (recommend 32+ bytes)
void atlantis_format_human_time(const AtlantisTime* t, char* buf, size_t size);

#endif // ATLANTIS_DATE_H
