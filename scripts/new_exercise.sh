#!/usr/bin/env bash
# Creates exercises/<name>/ from templates/exercise. Usage:
#   ./scripts/new_exercise.sh <snake_case_name> ["Human Title"]
# Works with the bash 3.2 and BSD sed that ship with macOS.
set -euo pipefail

name="${1:-}"
title="${2:-}"
if ! printf '%s' "$name" | grep -Eq '^[a-z][a-z0-9_]*$'; then
    echo "error: name must be snake_case (a-z, 0-9, _), got '$name'" >&2
    exit 1
fi
if [ -z "$title" ]; then title="$name"; fi

root="$(cd "$(dirname "$0")/.." && pwd)"
dest="$root/exercises/$name"
if [ -e "$dest" ]; then
    echo "error: exercises/$name already exists" >&2
    exit 1
fi
mkdir -p "$dest"

for ext in h cpp; do
    sed -e "s|__NAME__|$name|g" -e "s|__TITLE__|$title|g" \
        "$root/templates/exercise/NAME.$ext.in" > "$dest/$name.$ext"
done

echo "Created exercises/$name/$name.h and $name.cpp"
echo
echo "Now register it in src/main.cpp:"
echo "  1. add the include:   #include \"$name/$name.h\""
echo "  2. add a table row:   {\"$name\", &refresher::$name::Run},"
echo
echo "Then:  make run ONLY=$name V=1"
