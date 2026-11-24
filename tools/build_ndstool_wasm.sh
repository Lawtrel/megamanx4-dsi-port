#!/usr/bin/env bash
# Compile the same ndstool release used by the native SDK for browsers.
set -euo pipefail

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
workspace="$root/build/browser-tools"
source="$workspace/ndstool"
output="$workspace/wasm"
revision=76e8b681bb225d945a48852821e03114e6c7ce1c

command -v emcc >/dev/null
command -v em++ >/dev/null
mkdir -p "$workspace" "$output"
if [[ ! -d "$source/.git" ]]; then
    git clone --quiet --depth 1 --branch v2.3.1 https://github.com/devkitPro/ndstool.git "$source"
fi
if [[ "$(git -C "$source" rev-parse HEAD)" != "$revision" ]] ||
   [[ -n "$(git -C "$source" status --porcelain)" ]]; then
    echo "Unexpected ndstool revision or local changes" >&2
    exit 1
fi
for file in "$source"/source/*.c; do
    name=$(basename "$file" .c)
    emcc -O2 -c "$file" -o "$output/$name.o"
done
em++ -O2 -g2 '-DPACKAGE_VERSION="2.3.1"' "$source"/source/*.cpp "$output"/*.o \
    -sMODULARIZE=1 -sINVOKE_RUN=0 -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=2097152 -sSTACK_OVERFLOW_CHECK=2 \
    -sEXPORTED_RUNTIME_METHODS=FS,callMain -sENVIRONMENT=web,worker,node \
    -lworkerfs.js -o "$output/ndstool.mjs"
cp "$source/COPYING" "$output/ndstool-COPYING.txt"
