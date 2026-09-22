#!/bin/bash
# Generates a C header file mapping HTML entity names to Unicode codepoints

set -e

ENTITIES_JSON_URL="https://html.spec.whatwg.org/entities.json"
OUTPUT_FILE="html_entities.h"

echo "Downloading HTML entities from WHATWG..."
curl -s "$ENTITIES_JSON_URL" -o entities.json

echo "Generating $OUTPUT_FILE..."

cat > "$OUTPUT_FILE" << 'HEADER'
#ifndef HTML_ENTITIES_H
#define HTML_ENTITIES_H

// Auto-generated HTML5 named entity mapping
// Source: https://html.spec.whatwg.org/entities.json

#include <string.h>
#include <stddef.h>
#include "utf8proc/utf8proc.h"

typedef struct {
    const char *name;
    const char *utf8;
} HtmlEntity;

HEADER

# Convert codepoints array to UTF-8 C escape string
to_utf8() {
    local utf8=""
    for decimal in "$@"; do
        if [ $decimal -le 127 ]; then
            utf8+=$(printf '\\x%02x' $decimal)
        elif [ $decimal -le 2047 ]; then
            utf8+=$(printf '\\x%02x\\x%02x' $((0xC0 | (decimal >> 6))) $((0x80 | (decimal & 0x3F))))
        elif [ $decimal -le 65535 ]; then
            utf8+=$(printf '\\x%02x\\x%02x\\x%02x' $((0xE0 | (decimal >> 12))) $((0x80 | ((decimal >> 6) & 0x3F))) $((0x80 | (decimal & 0x3F))))
        else
            utf8+=$(printf '\\x%02x\\x%02x\\x%02x\\x%02x' $((0xF0 | (decimal >> 18))) $((0x80 | ((decimal >> 12) & 0x3F))) $((0x80 | ((decimal >> 6) & 0x3F))) $((0x80 | (decimal & 0x3F))))
        fi
    done
    echo "$utf8"
}

echo "static const HtmlEntity entity_table[] = {" >> "$OUTPUT_FILE"

# Extract entities that end with semicolon (CommonMark requires semicolon)
# Format: "&name;" -> codepoints array
# We strip the & prefix and ; suffix to get just the name
# Sort by length first, then alphabetically by byte value (for binary search with length comparison)
# LC_ALL=C ensures ASCII byte-order sorting (uppercase before lowercase)
jq -r 'to_entries[] | select(.key | endswith(";")) | "\(.key)\t\(.value.codepoints | join(" "))"' entities.json | \
sed 's/^&//; s/;	/	/' | \
awk -F'\t' '{print length($1) "\t" $0}' | LC_ALL=C sort -t$'\t' -k1,1n -k2,2 | cut -f2- | \
while IFS=$'\t' read -r name codepoints; do
    # Convert space-separated codepoints to UTF-8
    read -ra cp_array <<< "$codepoints"
    utf8=$(to_utf8 "${cp_array[@]}")
    echo "    {\"$name\", \"$utf8\"},"
done >> "$OUTPUT_FILE"

echo "};" >> "$OUTPUT_FILE"

cat >> "$OUTPUT_FILE" << 'FOOTER'

#define ENTITY_TABLE_SIZE (sizeof(entity_table) / sizeof(entity_table[0]))

// Binary search for named entity
// Table is sorted by: length first, then alphabetically
static inline const char* entity_lookup(const char *name, size_t name_len) {
    size_t lo = 0, hi = ENTITY_TABLE_SIZE;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        const char *entry_name = entity_table[mid].name;
        size_t entry_len = strlen(entry_name);

        int cmp;
        if (name_len != entry_len) {
            cmp = (name_len < entry_len) ? -1 : 1;
        } else {
            cmp = memcmp(name, entry_name, name_len);
        }

        if (cmp == 0) return entity_table[mid].utf8;
        if (cmp < 0) hi = mid;
        else lo = mid + 1;
    }
    return NULL;
}

// Decode numeric character reference (after &#)
// Input: str points after "&#", e.g., "123;" or "x1F;"
// Returns UTF-8 length written to out, and sets *consumed to chars consumed
// Returns 0 if invalid
static inline int entity_decode_numeric(const char *str, size_t len, char *out, size_t *consumed) {
    if (len == 0) return 0;

    unsigned int codepoint = 0;
    size_t i = 0;
    int is_hex = 0;

    // Check for hex prefix
    if (str[0] == 'x' || str[0] == 'X') {
        is_hex = 1;
        i = 1;
    }

    size_t digits_start = i;

    if (is_hex) {
        // Hex: up to 6 digits
        while (i < len && i - digits_start < 6) {
            char c = str[i];
            if (c >= '0' && c <= '9') {
                codepoint = codepoint * 16 + (c - '0');
            } else if (c >= 'a' && c <= 'f') {
                codepoint = codepoint * 16 + (c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                codepoint = codepoint * 16 + (c - 'A' + 10);
            } else {
                break;
            }
            i++;
        }
    } else {
        // Decimal: up to 7 digits
        while (i < len && i - digits_start < 7) {
            char c = str[i];
            if (c >= '0' && c <= '9') {
                codepoint = codepoint * 10 + (c - '0');
            } else {
                break;
            }
            i++;
        }
    }

    // Must have at least one digit
    if (i == digits_start) return 0;

    // Must end with semicolon
    if (i >= len || str[i] != ';') return 0;
    i++;  // consume semicolon

    *consumed = i;

    // Replace U+0000 with replacement char for security
    if (codepoint == 0) codepoint = 0xFFFD;

    // Invalid codepoints become replacement char
    if (codepoint > 0x10FFFF) codepoint = 0xFFFD;

    // Use utf8proc to encode codepoint to UTF-8
    utf8proc_ssize_t bytes = utf8proc_encode_char((utf8proc_int32_t)codepoint, (utf8proc_uint8_t *)out);
    return (int)(bytes > 0 ? bytes : 0);
}

#endif // HTML_ENTITIES_H
FOOTER

rm -f entities.json

count=$(grep -c '^\s*{"' "$OUTPUT_FILE" || true)
echo "Done! Generated $OUTPUT_FILE with $count entity entries."
echo "Example: entity_lookup(\"nbsp\", 4) returns non-breaking space as UTF-8"
