#include "drawtext.h"
#include "gfxfont.h"
#include <mo5_video.h>

#include "FreeSans12pt7b.h"

GFXfont *currentFont = &FreeSans12pt7b;

void setFont(GFXfont *font) { currentFont = font; }

#define DRAW_TARGET_PIXEL_WIDTH 320 // Example width, replace with actual value
#define DRAW_TARGET_PIXEL_HEIGHT 40 // Maximum band height (buffer size)
#define DRAW_TARGET_BYTE_WIDTH (DRAW_TARGET_PIXEL_WIDTH / 8)
#define DRAW_TARGET_BYTE_SIZE (DRAW_TARGET_BYTE_WIDTH * DRAW_TARGET_PIXEL_HEIGHT)
uint8_t DRAW_TARGET[DRAW_TARGET_BYTE_SIZE] = "";

// Band height in rows for the current font, clamped to the buffer.
int fontHeight(void) {
  return currentFont->yAdvance < DRAW_TARGET_PIXEL_HEIGHT
             ? currentFont->yAdvance
             : DRAW_TARGET_PIXEL_HEIGHT;
}

void drawPixel(int x, int y) { // always in bounds, no checks for speed.
  if (x < 0 || x >= DRAW_TARGET_PIXEL_WIDTH || y < 0 ||
      y >= fontHeight())
    return;
  int xstart = x / 8;
  int xoffset = x % 8;
  unsigned char *dst = DRAW_TARGET + y * DRAW_TARGET_BYTE_WIDTH + xstart;
  *dst |= 1 << (7 - xoffset);
}

int drawChar(char c, int x, int y) {
  unsigned char character = (unsigned char)c;
  unsigned int glyphIndex;
  unsigned int bitmapOffset;
  GFXglyph *glyph;
  uint8_t *bitmap;
  int width;
  int height;
  int xOffset;
  int yOffset;
  uint8_t xx, yy, bits, bit = 0;

  if (character < currentFont->first || character > currentFont->last)
    return 0;
  glyphIndex = character - currentFont->first;
  glyph = &currentFont->glyph[glyphIndex];
  bitmap = currentFont->bitmap;
  bitmapOffset = glyph->bitmapOffset;
  width = glyph->width;
  height = glyph->height;
  xOffset = glyph->xOffset;
  yOffset = glyph->yOffset;

  for (yy = 0; yy < height; yy++) {
    for (xx = 0; xx < width; xx++) {
      if (!(bit++ & 7)) {
        bits = bitmap[bitmapOffset++];
      }
      if (bits & 0x80) {
        drawPixel(x + xOffset + xx, y + yOffset + yy);
      }
      bits <<= 1;
    }
  }
  return width;
}

int stringWidth(const char *str) {
  int width = 0;
  unsigned char c;
  while ((c = *str++) != 0) {
    if (c >= currentFont->first && c <= currentFont->last)
      width += currentFont->glyph[c - currentFont->first].xAdvance;
  }
  return width;
}

int drawString(const char *str, int x, int y) {
  unsigned char c;
  unsigned int yy;
  unsigned int rows = fontHeight();
  int baseline = rows * 3 / 4;

  memset(DRAW_TARGET, 0, rows * DRAW_TARGET_BYTE_WIDTH);
  while ((c = *str++) != 0) {
    if (c >= currentFont->first && c <= currentFont->last) {
      drawChar(c, x, baseline);
      x += currentFont->glyph[c - currentFont->first].xAdvance;
    }
  }
  if (y + rows > SCREEN_HEIGHT) rows = SCREEN_HEIGHT - y;
  mo5_wait_vbl();
  for (yy = 0; yy < rows; yy++) {
    memcpy(VRAM + (y + yy) * DRAW_TARGET_BYTE_WIDTH,
           DRAW_TARGET + yy * DRAW_TARGET_BYTE_WIDTH, DRAW_TARGET_BYTE_WIDTH);
  }
  return x;
}

int drawStringCenter(const char *str, int y) {
  int x = (DRAW_TARGET_PIXEL_WIDTH - stringWidth(str)) / 2;
  return drawString(str, x < 0 ? 0 : x, y);
}
