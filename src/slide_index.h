#ifndef SLIDE_INDEX_H
#define SLIDE_INDEX_H

#define SLIDE_INDEX_MAX_BYTES 1024
#define SLIDE_FILENAME_SIZE 13
#define SLIDE_TEXT_SIZE 41 /* 40 characters + NUL */

typedef struct {
  char filename[SLIDE_FILENAME_SIZE];
  unsigned int seconds;
  unsigned char showInfo;        /* 0 when the caption field is "-" */
  char caption[SLIDE_TEXT_SIZE]; /* empty when absent */
  char author[SLIDE_TEXT_SIZE];  /* empty when absent */
} SlideEntry;

/* Load and validate SLIDE.INI from drive 0. */
unsigned char slideIndexLoad(void);
void slideIndexRewind(void);
/* Slideshow title from a "TITLE=..." line; empty string when absent. */
const char *slideIndexTitle(void);
/* Returns 1 for an entry, 0 at the end, or -1 for invalid input. */
int slideIndexNext(SlideEntry *entry);

#endif
