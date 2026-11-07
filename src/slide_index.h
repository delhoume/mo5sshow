#ifndef SLIDE_INDEX_H
#define SLIDE_INDEX_H

#define SLIDE_INDEX_MAX_BYTES 1024
#define SLIDE_FILENAME_SIZE 13

typedef struct {
  char filename[SLIDE_FILENAME_SIZE];
  unsigned int seconds;
} SlideEntry;

/* Load and validate SLIDE.INI from drive 0. */
unsigned char slideIndexLoad(void);
void slideIndexRewind(void);
/* Returns 1 for an entry, 0 at the end, or -1 for invalid input. */
int slideIndexNext(SlideEntry *entry);

#endif
