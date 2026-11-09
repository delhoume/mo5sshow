#ifndef DRAWTEXT_H
#define DRAWTEXT_H

#include "gfxfont.h"

extern GFXfont FreeSans12pt7b;
extern GFXfont FreeSans9pt7b;

/* Sets the font used by all drawing and measuring functions. */
void setFont(GFXfont *font);
/* Height in rows of the band drawString copies to the screen. */
int fontHeight(void);
/* Width in pixels of str in the current font. */
int stringWidth(const char *str);
/* Draws str at pixel column x in the fontHeight() rows starting at screen row y. */
int drawString(const char *str, int x, int y);
/* Same as drawString but horizontally centered on the 320-pixel screen. */
int drawStringCenter(const char *str, int y);
int drawChar(char c, int x, int y);

#endif
