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

static unsigned char decodeBuffer[FORM_STREAM_BYTES];

void set_color_mode() { *PRC &= ~0x01; }
void set_form_mode() { *PRC |= 0x01; }

static unsigned char decodeStream(const unsigned char **stream,
                                  Reader *reader,
                                  unsigned char *buffer,
                                  unsigned int size) {
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

static void writeColorsToVRAM(const unsigned char *foreground,
                              const unsigned char *background) {
  unsigned int col;
  unsigned int row;
  unsigned int offset;
  unsigned char fg;
  unsigned char bg;

  for (col = 0; col < SCREEN_WIDTH_BYTES; col++) {
    for (row = 0; row < SCREEN_HEIGHT; row++) {
      offset = col * (SCREEN_HEIGHT / 2) + row / 2;
      fg = (row & 1) ? (foreground[offset] & 0x0F)
                     : (foreground[offset] >> 4);
      bg = (row & 1) ? (background[offset] & 0x0F)
                     : (background[offset] >> 4);
      VRAM[row * SCREEN_WIDTH_BYTES + col] = COLOR(bg, fg);
    }
  }
}

static void writeFormToVRAM(const unsigned char *buffer) {
  unsigned int col;
  unsigned int row;

  for (col = 0; col < SCREEN_WIDTH_BYTES; col++) {
    for (row = 0; row < SCREEN_HEIGHT; row++) {
        VRAM[row * SCREEN_WIDTH_BYTES + col] =
            buffer[col * SCREEN_HEIGHT + row];
    } 
  }
}

static unsigned char showImage(const unsigned char *imagedata, Reader *reader) {
    const unsigned char *stream = imagedata;

    set_form_mode();
    memset(VRAM, 0, SCREEN_SIZE_BYTES);
    set_color_mode();
    memset(VRAM, COLOR(C_BLACK, C_BLACK)  , SCREEN_SIZE_BYTES);
    if (!decodeStream(&stream, reader, decodeBuffer, COLOR_STREAM_BYTES) ||
        !decodeStream(&stream, reader, decodeBuffer + COLOR_STREAM_BYTES,
                      COLOR_STREAM_BYTES)) {
      return 0;
    }

    mo5_wait_vbl();
  
    set_color_mode();
    writeColorsToVRAM(decodeBuffer, decodeBuffer + COLOR_STREAM_BYTES);

    if (!decodeStream(&stream, reader, decodeBuffer, FORM_STREAM_BYTES)) {
      return 0;
    }
    set_form_mode();
    writeFormToVRAM(decodeBuffer);
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