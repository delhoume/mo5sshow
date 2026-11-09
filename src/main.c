/*
 * Frederic Delhoume 2026
 */

#include "drawtext.h"
#include "slide_index.h"
#include <mo5_image.h>
#include <mo5_video.h>
#include "FreeSans9pt7b.h"

static void waitSeconds(unsigned int seconds) {
  while (seconds--) {
    unsigned char frame;
    for (frame = 0; frame < 50; frame++)
      mo5_wait_vbl();
  }
}

static void clearForText(void) {
  set_form_mode();
  memset(VRAM, 0, SCREEN_SIZE_BYTES);
  set_color_mode();
  memset(VRAM, COLOR(C_BLACK, C_WHITE), SCREEN_SIZE_BYTES);
  set_form_mode();
}

static void showError(const char *message, const char *name) {
  clearForText();
  drawStringCenter(message, 50);
  drawStringCenter(name, 100);
  waitSeconds(4);
}

static void displayTitle(const char *title) {
  clearForText();
  drawStringCenter(title, 80);
  waitSeconds(3);
}

// Caption band is 29 rows tall (FreeSans 12pt) starting at row 60.
#define AUTHOR_Y 90

void displaySlideInfo(const SlideEntry *slide) {
  char buffer[SLIDE_FILENAME_SIZE];
  unsigned char idx = 0;
  const char *name = slide->caption;

  clearForText();
  if (!*name) {
    // Fall back to the filename without its extension.
    while (slide->filename[idx] && slide->filename[idx] != '.') {
      buffer[idx] = slide->filename[idx];
      idx++;
    }
    buffer[idx] = '\0';
    name = buffer;
  }
  setFont(&FreeSans12pt7b);
  drawStringCenter(name, 60);
  if (*slide->author) {
    // Text is form bits over the color bank, so recolor the author band.
    setFont(&FreeSans9pt7b);
    set_color_mode();
    memset(VRAM + AUTHOR_Y * SCREEN_WIDTH_BYTES, COLOR(C_BLACK, C_GRAY),
           fontHeight() * SCREEN_WIDTH_BYTES);
    set_form_mode();
    drawStringCenter(slide->author, AUTHOR_Y);
    setFont(&FreeSans12pt7b);
  }
  waitSeconds(2);
}

int main() {
  static SlideEntry slide;
  static int status;

  // Keep the stack away from the monitor's disk parameter block at $2048.
  asm {
    LDS #$9F00
  }
  mo5_video_init(COLOR(C_BLACK, C_BLACK));

  while (1) {
    if (!slideIndexLoad()) {
      showError("ERROR READING", "SLIDE.INI");
      continue;
    }
    if (*slideIndexTitle())
      displayTitle(slideIndexTitle());
    while ((status = slideIndexNext(&slide)) == 1) {
      if (slide.showInfo)
        displaySlideInfo(&slide);
      if (showImageFromDisk(slide.filename)) {
        waitSeconds(slide.seconds);
      } else {
        showError("ERROR READING", slide.filename);
      }
    }
    if (status < 0)
      showError("INVALID", "SLIDE.INI");
  }
  return 0;
}
