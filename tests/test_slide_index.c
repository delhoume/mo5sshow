#include <stdio.h>
#include <string.h>

#include "filereader.h"
#include "slide_index.h"

static unsigned char data[SLIDE_INDEX_MAX_BYTES + 1];
static unsigned int size, position, closes;
static int fail_open, fail_read;

static int eof(void) { return position >= size || fail_read; }
static unsigned int read_bytes(unsigned char *dst, unsigned int count) {
  unsigned int remaining = size - position;
  if (fail_read) return 0;
  if (count > remaining) count = remaining;
  memcpy(dst, data + position, count);
  position += count;
  return count;
}
static void close_reader(void) { closes++; }
static Reader reader = {0, 0, eof, 0, close_reader, read_bytes};

Reader *GetFileReader(const char *name, BitMode mode) {
  if (fail_open || strcmp(name, "SLIDE.INI") || mode != MODE_8BIT) return 0;
  position = 0;
  return &reader;
}
int FileReaderHasError(void) { return fail_read; }

static void input(const char *text) {
  size = strlen(text);
  memcpy(data, text, size);
}

int main(void) {
  SlideEntry entry;

  input("# comment\r\n\r\nlena.m5z=4\nBIRDS.M5Z=255\nA.M5Z=0");
  if (!slideIndexLoad() || closes != 1) return 1;
  if (slideIndexNext(&entry) != 1 || strcmp(entry.filename, "LENA.M5Z") ||
      entry.seconds != 4) return 2;
  if (slideIndexNext(&entry) != 1 || strcmp(entry.filename, "BIRDS.M5Z") ||
      entry.seconds != 255) return 3;
  if (slideIndexNext(&entry) != 1 || strcmp(entry.filename, "A.M5Z") ||
      entry.seconds != 0 || slideIndexNext(&entry) != 0) return 4;
  slideIndexRewind();
  if (slideIndexNext(&entry) != 1 || strcmp(entry.filename, "LENA.M5Z")) return 5;

  input("BIRDS.M5Z=4\nTOOLONGXX.M5Z=2\n");
  if (slideIndexLoad()) return 6;
  input("BIRDS.M5Z=256\n");
  if (slideIndexLoad()) return 7;
  input("BIRDS.M5Z=\n");
  if (slideIndexLoad()) return 8;
  input("BIRDS.M5Z=0000\n");
  if (slideIndexLoad()) return 9;
  input("BIRDS.BIN=4\n");
  if (slideIndexLoad()) return 10;
  input("# no entries\n\n");
  if (slideIndexLoad()) return 11;
  memset(data, '#', SLIDE_INDEX_MAX_BYTES);
  memcpy(data + SLIDE_INDEX_MAX_BYTES - 12, "\nBIRDS.M5Z=4", 12);
  size = SLIDE_INDEX_MAX_BYTES;
  if (!slideIndexLoad() || slideIndexNext(&entry) != 1 ||
      strcmp(entry.filename, "BIRDS.M5Z")) return 16;
  memset(data, '#', sizeof(data));
  size = sizeof(data);
  if (slideIndexLoad()) return 12;
  input("BIRDS.M5Z=4\n");
  fail_read = 1;
  if (slideIndexLoad()) return 13;
  fail_read = 0;
  fail_open = 1;
  if (slideIndexLoad()) return 14;
  fail_open = 0;
  if (!slideIndexLoad() || slideIndexNext(&entry) != 1) return 15;
  puts("Slideshow index parsing and error handling passed");
  return 0;
}
