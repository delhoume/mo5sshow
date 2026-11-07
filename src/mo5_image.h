#if !defined(MO5_IMAGE_H)
#define MO5_IMAGE_H

void showImageFromMemory(const unsigned char *imagedata);
/* Reads an 8.3 filename from drive 0; returns 0 on open, read or decoding failure. */
unsigned char showImageFromDisk(const char *filename);

#endif // MO5_IMAGE_H	