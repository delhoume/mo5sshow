#include "reader.h"


static Reader* reader = 0;
static unsigned char readerError;

Reader* GetReader() {
  return reader;
}

void SetReader(Reader* areader) {
  if (reader) reader->close();
  reader = areader;
  readerError = 0;
}

static unsigned char get_next_value8() {
  if (reader == 0 || reader->eof()) {
    readerError = 1;
    return 0;
  }
  return reader->nextByte();
}

unsigned char GetNextValue() {
  return get_next_value8();
}
  
unsigned char GetNextBytes(unsigned char *dst, unsigned int count) {
  if (reader != 0) {
    if (reader->readBytes(dst, count) == count) {
      return 1;
    }
    readerError = 1;
    return 0;
  }
  while (count--) {
    *dst++ = GetNextValue();
  }
  return !readerError;
}

int ReaderHasError(void) {
  return readerError;
}
