#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_binary="$(mktemp "${TMPDIR:-/tmp}/mo5-image-test.XXXXXXXX")"
trap 'rm -f "$test_binary"' 0
cc -Wall -Wextra -Wno-unused-parameter -Itests/include -Isrc -Itools/sdk_mo5 \
  src/mo5_image.c src/dzx0.c tests/test_image_disk.c -o "$test_binary"
for image in assets/BIRDS.M5Z assets/LENA.M5Z assets/BEACH.M5Z \
             assets/BLAKE.M5Z assets/PINK.M5Z; do
  echo "Testing $image"
  "$test_binary" "$image"
done
