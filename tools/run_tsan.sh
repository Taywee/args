#!/usr/bin/env bash
set -euo pipefail

# Helper script to build and run the concurrency reproducer under ThreadSanitizer.
# Produces logs: tsan_mutex_disabled.log and tsan_mutex_enabled.log

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR_DISABLED="$ROOT_DIR/build.mutex_disabled"
BUILD_DIR_ENABLED="$ROOT_DIR/build.mutex_enabled"

clang++ --version >/dev/null 2>&1 || { echo "clang++ not found in PATH"; exit 1; }

# Vulnerable build (mutex disabled)
mkdir -p "$BUILD_DIR_DISABLED"
cmake -S "$ROOT_DIR" -B "$BUILD_DIR_DISABLED" -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DARGS_DISABLE_PARSE_MUTEX=1 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=thread -g -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=thread'
cmake --build "$BUILD_DIR_DISABLED" --parallel

echo "Running reproducer with ARGS_DISABLE_PARSE_MUTEX=1 (expect races)"
"$BUILD_DIR_DISABLED/argstest-concurrency_reproducer" 2>&1 | tee "$ROOT_DIR/tsan_mutex_disabled.log" || true

# Fixed build (mutex enabled)
mkdir -p "$BUILD_DIR_ENABLED"
cmake -S "$ROOT_DIR" -B "$BUILD_DIR_ENABLED" -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DARGS_DISABLE_PARSE_MUTEX=0 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=thread -g -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=thread'
cmake --build "$BUILD_DIR_ENABLED" --parallel

echo "Running reproducer with mutex enabled (expect clean)"
"$BUILD_DIR_ENABLED/argstest-concurrency_reproducer" 2>&1 | tee "$ROOT_DIR/tsan_mutex_enabled.log" || true

echo "Logs saved to: $ROOT_DIR/tsan_mutex_disabled.log and $ROOT_DIR/tsan_mutex_enabled.log"
