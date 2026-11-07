#ifndef TEST_MO5_VIDEO_H
#define TEST_MO5_VIDEO_H

extern unsigned char test_prc;
extern unsigned char color_vram[8000];
extern unsigned char form_vram[8000];

#define PRC (&test_prc)
#define VRAM ((test_prc & 1) ? form_vram : color_vram)
#define SCREEN_WIDTH_BYTES 40
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE_BYTES 8000
#define C_BLACK 0
#define COLOR(bg, fg) ((unsigned char)((bg) | ((fg) << 4)))

void mo5_wait_vbl(void);
#endif
