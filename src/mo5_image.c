#include "mo5_image.h"
#include "dzx0.h"
#include "filereader.h"
#include "mo5_video.h"

#include <cmoc.h>

// format is Fred Stark mo5z
// 3 zx streams
// - foreground colors -> 4000 bytes (two color nibbles per byte)
// - background colors -> 4000 bytes (two color nibbles per byte)
// - form -> 8000 bytes
// column major order
// https://github.com/fstark/png2mo5

#define COLOR_STREAM_BYTES 4000
#define FORM_STREAM_BYTES 8000

static unsigned char decodeBufferHalf[COLOR_STREAM_BYTES];
static unsigned char decodeBufferFull[FORM_STREAM_BYTES];

void set_color_mode() { *PRC &= ~0x01; }
void set_form_mode() { *PRC |= 0x01; }

static unsigned char decodeStream(const unsigned char **stream, Reader *reader,
                                  unsigned char *buffer, unsigned int size) {
  if (reader) {
    dzx0_init_reader(reader, buffer);
  } else {
    dzx0_init(*stream, buffer);
  }
  if (!dzx0_run(buffer + size) || dzx0_decoded() != size) {
    return 0;
  }
  if (!reader) {
    *stream += dzx0_consumed();
  }
  return 1;
}

// Unpacks column-major nibble pairs into row-major color bytes.
// shift is 4 for the foreground stream, 0 for the background stream.
static void convertColorToBuffer(unsigned char shift) {
  unsigned int col;
  unsigned int row;
  unsigned int idx;
  unsigned int dst;
  unsigned char packed;

  for (col = 0; col < SCREEN_WIDTH_BYTES; col++) {
    for (row = 0; row < SCREEN_HEIGHT / 2; row++) {
      idx = col * (SCREEN_HEIGHT / 2) + row;
      dst = row * 2 * SCREEN_WIDTH_BYTES + col;
      packed = decodeBufferHalf[idx];
      decodeBufferFull[dst] |= (packed >> 4) << shift;
      decodeBufferFull[dst + SCREEN_WIDTH_BYTES] |= (packed & 0x0F) << shift;
    }
  }
}

static void writeFormToVRAM(const unsigned char *buffer) {
  for (unsigned int col = 0; col < SCREEN_WIDTH_BYTES; col++) {
    for (unsigned int row = 0; row < SCREEN_HEIGHT; row++) {
      VRAM[row * SCREEN_WIDTH_BYTES + col] = buffer[col * SCREEN_HEIGHT + row];
    }
  }
}

static void showColors() {
  mo5_wait_vbl();
  set_color_mode();
  memcpy(VRAM, decodeBufferFull, SCREEN_SIZE_BYTES);
}

static unsigned char showImage(const unsigned char *imagedata, Reader *reader) {
  const unsigned char *stream = imagedata;

  mo5_wait_vbl();
  set_color_mode();
  memset(VRAM, COLOR(C_BLACK, C_BLACK), SCREEN_SIZE_BYTES);
  set_form_mode();
  memset(VRAM, 0, SCREEN_SIZE_BYTES);

  // The file stores the foreground stream first, then the background stream.
  // Each color phase is shown as it is decoded; the form is set to all-ones
  // for the foreground phase and left untouched afterwards.
  memset(decodeBufferFull, 0, SCREEN_SIZE_BYTES);
  if (!decodeStream(&stream, reader, decodeBufferHalf, COLOR_STREAM_BYTES)) {
    return 0;
  }
  convertColorToBuffer(4);
  showColors();
  set_form_mode();
  memset(VRAM, 0xAA, SCREEN_SIZE_BYTES);
    if (!decodeStream(&stream, reader, decodeBufferHalf, COLOR_STREAM_BYTES)) {
    return 0;
  }
  convertColorToBuffer(0);
  showColors();

  if (!decodeStream(&stream, reader, decodeBufferFull, FORM_STREAM_BYTES)) {
    return 0;
  }
  mo5_wait_vbl();
  set_form_mode();
  writeFormToVRAM(decodeBufferFull);
  return 1;
}

void showImageFromMemory(const unsigned char *imagedata) {
  showImage(imagedata, 0);
}

unsigned char showImageFromDisk(const char *filename) {
  Reader *reader = GetFileReader(filename);
  unsigned char result;

  if (!reader) {
    return 0;
  }
  result = showImage(0, reader);
  reader->close();
  return result && !FileReaderHasError();
}
