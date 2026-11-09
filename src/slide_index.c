#include "slide_index.h"
#include "filereader.h"

static unsigned char indexData[SLIDE_INDEX_MAX_BYTES];
static unsigned int indexSize;
static unsigned int indexPosition;
static char titleText[SLIDE_TEXT_SIZE];

const char *slideIndexTitle(void) { return titleText; }

void slideIndexRewind(void) { indexPosition = 0; }

static unsigned char upper(unsigned char c) {
  return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

/* Copies indexData[from..to) into dst; returns 0 if too long or not printable. */
static unsigned char copyText(char *dst, unsigned int from, unsigned int to) {
  unsigned int n = 0;
  if (to - from >= SLIDE_TEXT_SIZE) return 0;
  while (from < to) {
    unsigned char c = indexData[from++];
    if (c < ' ' || c > '~') return 0;
    dst[n++] = c;
  }
  dst[n] = '\0';
  return 1;
}

static unsigned char isTitleLine(unsigned int start, unsigned int end) {
  return end - start >= 6 && upper(indexData[start]) == 'T' &&
         upper(indexData[start + 1]) == 'I' &&
         upper(indexData[start + 2]) == 'T' &&
         upper(indexData[start + 3]) == 'L' &&
         upper(indexData[start + 4]) == 'E' && indexData[start + 5] == '=';
}

int slideIndexNext(SlideEntry *entry) {
  unsigned int start;
  unsigned int end;
  unsigned int i;
  unsigned int baseLength;
  unsigned int seconds;
  unsigned int textStart;
  unsigned int authorStart;

  while (indexPosition < indexSize) {
    start = indexPosition;
    while (indexPosition < indexSize && indexData[indexPosition] != '\n')
      indexPosition++;
    end = indexPosition;
    if (indexPosition < indexSize) indexPosition++;
    if (end > start && indexData[end - 1] == '\r') end--;
    if (start == end || indexData[start] == '#') continue;
    if (isTitleLine(start, end)) {
      if (!copyText(titleText, start + 6, end)) return -1;
      continue;
    }

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
    textStart = i;
    while (textStart < end && indexData[textStart] != '|') textStart++;
    if (textStart - i == 0 || textStart - i > 3) return -1;
    seconds = 0;
    while (i < textStart) {
      unsigned char c = indexData[i++];
      if (c < '0' || c > '9') return -1;
      seconds = seconds * 10 + c - '0';
      if (seconds > 255) return -1;
    }
    entry->showInfo = 1;
    entry->caption[0] = '\0';
    entry->author[0] = '\0';
    if (textStart < end) {
      authorStart = textStart + 1;
      while (authorStart < end && indexData[authorStart] != '|') authorStart++;
      if (!copyText(entry->caption, textStart + 1, authorStart)) return -1;
      if (authorStart < end &&
          !copyText(entry->author, authorStart + 1, end)) return -1;
      // A lone "-" caption disables the info screen for this slide.
      if (entry->caption[0] == '-' && !entry->caption[1]) {
        entry->showInfo = 0;
        entry->caption[0] = '\0';
        entry->author[0] = '\0';
      }
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
  titleText[0] = '\0';
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
