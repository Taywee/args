#!/usr/bin/env bash
# Build PoC against buggy and fixed args.hxx, run both under ASan+UBSan.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE"

# Extract the two args.hxx versions from git history into this directory.
( cd /mnt/d/args && git show a1407f8^:args.hxx > "$HERE/args_buggy.hxx" )
( cd /mnt/d/args && git show a1407f8:args.hxx  > "$HERE/args_fixed.hxx" )
ls -la args_*.hxx

SAN='-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer'
COMMON="-std=c++11 -O0 -g -I. $SAN -Wall"

echo "===== BUILD: buggy ====="
clang++ $COMMON -DARGS_HXX_PATH='"args_buggy.hxx"' poc_noexcept_iter_oob.cxx -o poc_buggy
echo "build exit: $?"

echo "===== BUILD: fixed ====="
clang++ $COMMON -DARGS_HXX_PATH='"args_fixed.hxx"' poc_noexcept_iter_oob.cxx -o poc_fixed
echo "build exit: $?"

echo
echo "===== RUN: buggy ====="
ASAN_OPTIONS=abort_on_error=0:halt_on_error=0:print_stacktrace=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=0 \
./poc_buggy
echo "buggy exit: $?"

echo
echo "===== RUN: fixed ====="
ASAN_OPTIONS=abort_on_error=0:halt_on_error=0:print_stacktrace=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=0 \
./poc_fixed
echo "fixed exit: $?"
