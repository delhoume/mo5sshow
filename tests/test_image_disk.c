#include <stdio.h>
#include <string.h>

#include "filereader.h"
#include "mo5_image.h"

unsigned char test_prc;
unsigned char color_vram[8000];
unsigned char form_vram[8000];
static unsigned char expected_color[8000];
static unsigned char expected_form[8000];
static unsigned char input[20000];
static unsigned int size, pos, cutoff, closed;
static int failed;

void mo5_wait_vbl(void) {}
static int eof(void) { return pos >= cutoff; }
static unsigned char next(void) { return input[pos++]; }
static unsigned int read_bytes(unsigned char *dst, unsigned int n) {
  unsigned int avail = cutoff - pos;
  if (n > avail) n = avail;
  memcpy(dst, input + pos, n);
  pos += n;
  return n;
}
static void close_reader(void) { closed++; }
static Reader reader = {0, 0, eof, next, close_reader, read_bytes};

Reader *GetFileReader(const char *name, BitMode mode) {
  if (mode != MODE_8BIT || strcmp(name, "TEST.M5Z")) return 0;
  pos = 0;
  return &reader;
}

int FileReaderHasError(void) { return failed; }

int main(int argc, char **argv) {
  FILE *file;
  unsigned int i;

  if (argc != 2 || !(file = fopen(argv[1], "rb"))) return 1;
  size = (unsigned int)fread(input, 1, sizeof(input), file);
  fclose(file);
  if (size == 0 || size == sizeof(input)) return 2;
  cutoff = size;

  showImageFromMemory(input);
  for (i = 0; i < sizeof(form_vram) && !form_vram[i]; ++i) {}
  if (i == sizeof(form_vram)) return 8;
  memcpy(expected_color, color_vram, sizeof(color_vram));
  memcpy(expected_form, form_vram, sizeof(form_vram));
  memset(color_vram, 0x55, sizeof(color_vram));
  memset(form_vram, 0xaa, sizeof(form_vram));
  if (!showImageFromDisk("TEST.M5Z") || pos != size ||
      memcmp(expected_color, color_vram, sizeof(color_vram)) ||
      memcmp(expected_form, form_vram, sizeof(form_vram))) return 3;

  cutoff = size / 2;
  if (showImageFromDisk("TEST.M5Z")) return 4;
  cutoff = size;
  failed = 1;
  if (showImageFromDisk("TEST.M5Z")) return 5;
  failed = 0;
  if (showImageFromDisk("MISSING.M5Z")) return 6;
  if (!showImageFromDisk("TEST.M5Z") || closed != 4) return 7;

  puts("Disk/memory parity and disk error handling passed");
  return 0;
}
