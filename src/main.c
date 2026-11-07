/*
 * Frederic Delhoume 2026
 */

#include <mo5_video.h>
#include <mo5_font8.h>
#include <mo5_image.h>
#include "slide_index.h"

static void waitSeconds(unsigned int seconds) {
  while (seconds--) {
    unsigned char frame;
    for (frame = 0; frame < 50; frame++) mo5_wait_vbl();
  }
}

static void showError(const char *message, const char *name) {
  mo5_clear_screen(COLOR(C_BLACK, C_BLACK));
  mo5_font8_puts(0, 0, message, C_WHITE);
  mo5_font8_puts(0, 8, name, C_WHITE);
  waitSeconds(4);
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
    while ((status = slideIndexNext(&slide)) == 1) {
      if (showImageFromDisk(slide.filename)) {
        waitSeconds(slide.seconds);
      } else {
        showError("ERROR READING", slide.filename);
      }
    }
    if (status < 0) showError("INVALID", "SLIDE.INI");
  }
  return 0;
}
