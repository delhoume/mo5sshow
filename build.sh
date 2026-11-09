#!/bin/bash
set -e
mkdir -p build/disk

cmoc --thommo --limit=9e00 -o build/MO5SHOW.BIN -Isrc -Itools/sdk_mo5 -Itools/sdk_mo5/include \
  src/main.c src/slide_index.c src/mo5_image.c src/dzx0.c src/filereader.c src/reader.c src/drawtext.c  \
  -Ltools/sdk_mo5/lib -lsdk_mo5


if [ ! -x tools/fdfs ] || [ tools/src/fdfs.c -nt tools/fdfs ]; then
  gcc tools/src/fdfs.c -o tools/fdfs
fi

  mkdir -p  output

images=()
while IFS= read -r line || [ -n "$line" ]; do
  line="${line%$'\r'}"
  case "$line" in ''|\#*) continue ;; esac
  case "$line" in [Tt][Ii][Tt][Ll][Ee]=*) continue ;; esac
  if [[ ! "$line" =~ ^([A-Za-z0-9_-]{1,8})\.[Mm]5[Zz]=([0-9]{1,3})(\|[^|]*(\|.*)?)?$ ]]; then
    echo "Invalid SLIDE.INI entry: $line" >&2
    exit 1
  fi
  seconds="${BASH_REMATCH[2]}"
  if (( 10#$seconds > 255 )); then
    echo "Invalid slide duration: $line" >&2
    exit 1
  fi
  filename="$(printf '%s.M5Z' "${BASH_REMATCH[1]}" | tr '[:lower:]' '[:upper:]')"
  if [ ! -f "assets/$filename" ]; then
    echo "Missing slideshow image: assets/$filename" >&2
    exit 1
  fi
  found=0
  for image in "${images[@]}"; do
    if [ "$image" = "$filename" ]; then found=1; break; fi
  done
  if [ "$found" -eq 0 ]; then images+=("$filename"); fi
done < assets/SLIDE.INI
if [ "${#images[@]}" -eq 0 ] || [ "$(wc -c < assets/SLIDE.INI)" -gt 1024 ]; then
  echo "SLIDE.INI must list at least one image and fit in 1024 bytes" >&2
  exit 1
fi

cp build/MO5SHOW.BIN assets/BOOTMO.BIN assets/SLIDE.INI build/disk/
for image in "${images[@]}"; do cp "assets/$image" build/disk/; done

pushd build/disk
../../tools/fdfs -addBLfirst ../../output/mo5show.fd BOOTMO.BIN MO5SHOW.BIN SLIDE.INI "${images[@]}"
popd

ls -la output/*.fd