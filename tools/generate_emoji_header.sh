#!/bin/bash
# Generates a C header file mapping emoji shortcodes to Unicode codepoints

set -e

EMOJI_JSON_URL="https://raw.githubusercontent.com/iamcal/emoji-data/master/emoji.json"
OUTPUT_FILE="emoji_shortcodes.h"

echo "Downloading emoji data from iamcal/emoji-data..."
curl -s "$EMOJI_JSON_URL" -o emoji.json

echo "Generating $OUTPUT_FILE..."

cat > "$OUTPUT_FILE" << 'HEADER'
#ifndef EMOJI_SHORTCODES_H
#define EMOJI_SHORTCODES_H

// Auto-generated emoji shortcode to Unicode mapping
// Source: https://github.com/iamcal/emoji-data

#include <string.h>
#include <stddef.h>

typedef struct {
    const char *shortcode;
    const char *unicode;
} EmojiEntry;

HEADER

# Convert unified codepoints (e.g., "1F602" or "1F1E8-1F1F3") to UTF-8 C escape string
to_utf8() {
    local unified="$1"
    local utf8=""
    IFS='-' read -ra codepoints <<< "$unified"
    for cp in "${codepoints[@]}"; do
        local decimal=$((16#$cp))
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

# Extract, convert, and sort by shortcode
echo "static const EmojiEntry emoji_table[] = {" >> "$OUTPUT_FILE"

jq -r '.[] | .short_names[] as $name | "\($name)\t\(.unified)"' emoji.json | \
sort -t$'\t' -k1,1 | \
while IFS=$'\t' read -r shortcode unified; do
    utf8=$(to_utf8 "$unified")
    echo "    {\"$shortcode\", \"$utf8\"},"
done >> "$OUTPUT_FILE"

echo "};" >> "$OUTPUT_FILE"

cat >> "$OUTPUT_FILE" << 'FOOTER'

#define EMOJI_TABLE_SIZE (sizeof(emoji_table) / sizeof(emoji_table[0]))

static inline const char* emoji_lookup(const char *shortcode) {
    size_t lo = 0, hi = EMOJI_TABLE_SIZE;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        int cmp = strcmp(emoji_table[mid].shortcode, shortcode);
        if (cmp == 0) return emoji_table[mid].unicode;
        if (cmp < 0) lo = mid + 1;
        else hi = mid;
    }
    return NULL;
}

#endif // EMOJI_SHORTCODES_H
FOOTER

rm -f emoji.json

count=$(grep -c '^\s*{"' "$OUTPUT_FILE" || true)
echo "Done! Generated $OUTPUT_FILE with $count emoji entries."
echo "Example: emoji_lookup(\"joy\") returns 😂 as UTF-8"
