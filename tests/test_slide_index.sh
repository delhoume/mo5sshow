#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_binary="$(mktemp "${TMPDIR:-/tmp}/mo5-slide-test.XXXXXXXX")"
trap 'rm -f "$test_binary"' 0
cc -Wall -Wextra -Itests/include -Isrc -Itools/sdk_mo5 \
  src/slide_index.c tests/test_slide_index.c -o "$test_binary"
"$test_binary"
