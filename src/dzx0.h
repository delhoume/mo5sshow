#ifndef DZX0_H
#define DZX0_H

#include "reader.h"

void dzx0_init(const unsigned char *src, unsigned char *dst);
/* The reader must supply raw 8-bit bytes and remain open during decoding. */
void dzx0_init_reader(Reader *reader, unsigned char *dst);
unsigned char dzx0_run(unsigned char *until);
unsigned int dzx0_decoded(void);
unsigned int dzx0_consumed(void); /* Memory input only. */
unsigned char dzx0_done(void);

#endif
