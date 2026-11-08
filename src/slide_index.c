#include "slide_index.h"
#include "filereader.h"

static unsigned char indexData[SLIDE_INDEX_MAX_BYTES];
static unsigned int indexSize;
static unsigned int indexPosition;

void slideIndexRewind(void) { indexPosition = 0; }

static unsigned char upper(unsigned char c) {
  return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

int slideIndexNext(SlideEntry *entry) {
  unsigned int start;
  unsigned int end;
  unsigned int i;
  unsigned int baseLength;
  unsigned int seconds;

  while (indexPosition < indexSize) {
    start = indexPosition;
    while (indexPosition < indexSize && indexData[indexPosition] != '\n')
      indexPosition++;
    end = indexPosition;
    if (indexPosition < indexSize) indexPosition++;
    if (end > start && indexData[end - 1] == '\r') end--;
    if (start == end || indexData[start] == '#') continue;

    i = start;
    while (i < end && indexData[i] != '.') {
      unsigned char c = upper(indexData[i]);
      if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '_' || c == '-')) return -1;
      i++;
    }
    baseLength = i - start;
    if (baseLength == 0 || baseLength > 8 || i + 5 >= end ||
        indexData[i] != '.' || upper(indexData[i + 1]) != 'M' ||
        indexData[i + 2] != '5' || upper(indexData[i + 3]) != 'Z' ||
        indexData[i + 4] != '=') return -1;
    for (i = 0; i < baseLength + 4; i++)
      entry->filename[i] = upper(indexData[start + i]);
    entry->filename[i] = '\0';
    i = start + baseLength + 5;
    if (end - i > 3) return -1;
    seconds = 0;
    while (i < end) {
      unsigned char c = indexData[i++];
      if (c < '0' || c > '9' || seconds > 25) return -1;
      seconds = seconds * 10 + c - '0';
      if (seconds > 255) return -1;
    }
    entry->seconds = seconds;
    return 1;
  }
  return 0;
}

unsigned char slideIndexLoad(void) {
  Reader *reader = GetFileReader("SLIDE.INI");
  SlideEntry entry;
  int status;
  unsigned int count = 0;

  indexSize = 0;
  slideIndexRewind();
  if (!reader) return 0;
  indexSize = reader->readBytes(indexData, SLIDE_INDEX_MAX_BYTES);
  if (FileReaderHasError() || !reader->eof() || FileReaderHasError()) {
    reader->close();
    indexSize = 0;
    return 0;
  }
  reader->close();
  while ((status = slideIndexNext(&entry)) == 1) count++;
  slideIndexRewind();
  if (status < 0 || count == 0) {
    indexSize = 0;
    return 0;
  }
  return 1;
}
