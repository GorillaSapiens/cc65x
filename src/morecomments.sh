find . -type f \( -name "*.c" -o -name "*.h" \) -exec sed -i -E 's|^([[:space:]]*)//?[/*=\\|:~<>+\-]{3,}.*|\1////////////////////////////////////////////////////////////////////////////////|' {} +

find . -type f \( -name "*.c" -o -name "*.h" \) -exec sed -i -E 's|/\*+(.*?)\*/[[:space:]]*$|//\1|' {} +

find . -type f \( -name "*.c" -o -name "*.h" \) -exec sed -i -E 's/[[:space:]]+$//' {} +




