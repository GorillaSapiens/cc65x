#!/bin/bash

find . -type f \( -name "*.c" -o -name "*.h" \) | while read -r file; do
    tmpfile=$(mktemp)
    awk '
    function is_empty_cpp_comment(s) {
        return s ~ /^[[:space:]]*\/\/[[:space:]]*$/
    }
    function is_cpp_comment(s) {
        return s ~ /^[[:space:]]*\/\/.*$/
    }

    {
        # On second line or more
        if (NR > 1) {
            if (is_empty_cpp_comment(prev)) {
                if (!is_cpp_comment($0)) {
                    # skip prev
                    prev = $0
                    next
                }
            }
            print prev
        }
        prev = $0
    }

    END {
        if (!is_empty_cpp_comment(prev)) {
            print prev
        }
    }
    ' "$file" > "$tmpfile" && mv "$tmpfile" "$file"
done
