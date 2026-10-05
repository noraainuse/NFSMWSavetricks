#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/scripts"
SRC_MAIN="${SCRIPT_DIR}/src/main.c"
TARGET="${OUTPUT_DIR}/NFSMWSavetricks.asi"

mkdir -p "${OUTPUT_DIR}"

echo "==> Building NFSMWSavetricks.asi..."

if command -v i686-w64-mingw32-gcc >/dev/null 2>&1; then
    echo "Using system i686-w64-mingw32-gcc..."
    i686-w64-mingw32-gcc -O2 -s -shared -Wall -Wextra \
        -I"${SCRIPT_DIR}/include" \
        "${SRC_MAIN}" \
        -lkernel32 \
        -o "${TARGET}"
elif command -v wine >/dev/null 2>&1 && [ -f "${SCRIPT_DIR}/tools/tcc/tcc.exe" ]; then
    echo "Using bundled Tiny C Compiler via Wine..."
    wine "${SCRIPT_DIR}/tools/tcc/tcc.exe" -shared \
        -I"${SCRIPT_DIR}/include" \
        "${SRC_MAIN}" \
        -lkernel32 \
        -o "${TARGET}"
else
    echo "Error: No suitable compiler found (need i686-w64-mingw32-gcc or wine + tools/tcc)." >&2
    exit 1
fi

if [ -f "${TARGET}" ]; then
    echo "==> Successfully built: ${TARGET}"
    ls -lh "${TARGET}"
else
    echo "==> Build failed!" >&2
    exit 1
fi
