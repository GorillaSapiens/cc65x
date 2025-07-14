#!/bin/bash

find . -type f \( -name "*.c" -o -name "*.h" \) | while read -r file; do
    tmpfile=$(mktemp)
    awk '
    BEGIN { in_comment = 0 }

    function trim_stars(line) {
        sub(/^[[:space:]]*\**[[:space:]]*/, "", line)
        return line
    }

    {
        line = $0

        # If not inside a block and line has both /* and */, skip it (inline or one-liner)
        if (!in_comment && line ~ /\/\*.*\*\//) {
            print line
            next
        }

        # Start of a block comment? Must start with optional whitespace then /*
        if (!in_comment && line ~ /^[[:space:]]*\/\*/) {
            in_comment = 1
            indent = match(line, /[^[:space:]]/) ? substr(line, 1, RSTART - 1) : ""
            sub(/^[[:space:]]*\/\*/, "", line)  # remove starting /*
            if (line ~ /\*\//) {
                # Ends on same line — leave it alone
                print indent "/*" line
                in_comment = 0
                next
            }
            print indent "// " trim_stars(line)
            next
        }

        if (in_comment) {
            if (line ~ /\*\//) {
                sub(/\*\/[[:space:]]*$/, "", line)
                print indent "// " trim_stars(line)
                in_comment = 0
            } else {
                print indent "// " trim_stars(line)
            }
            next
        }

        print
    }
    ' "$file" > "$tmpfile" && mv "$tmpfile" "$file"
done
