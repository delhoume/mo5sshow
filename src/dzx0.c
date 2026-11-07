/*
 * Resumable ZX0 decompressor (format of Einar Saukas' ZX0, non-inverted).
 * https://github.com/einar-saukas/ZX0
 *
 * dzx0_run() decodes up to a given output address and can be called again
 * to continue, so the decompression cost can be spread over several frames.
 */
#include <cmoc.h>

#include "dzx0.h"

#define ST_LIT 0
#define ST_LAST 1
#define ST_NEW 2

static const unsigned char *zsrc;
static const unsigned char *zsrc_start;
static Reader *zreader;
static unsigned char *zstart;
static unsigned char *zdst;
static unsigned char zmask;
static unsigned char zbits;
static unsigned char zstate;
static unsigned char zlit;
static unsigned char zdone;
static unsigned int zoffset;
static unsigned int zpend;

void dzx0_init(const unsigned char *src, unsigned char *dst) {
  zsrc = src;
  zsrc_start = src;
  zreader = 0;
  zstart = dst;
  zdst = dst;
  zmask = 0;
  zbits = 0;
  zstate = ST_LIT;
  zlit = 0;
  zdone = 0;
  zoffset = 1;
  zpend = 0;
}

void dzx0_init_reader(Reader *reader, unsigned char *dst) {
  dzx0_init(0, dst);
  zreader = reader;
}

unsigned int dzx0_decoded(void) { return (unsigned int)(zdst - zstart); }

unsigned int dzx0_consumed(void) {
  return zreader ? 0u : (unsigned int)(zsrc - zsrc_start);
}

unsigned char dzx0_done(void) { return zdone; }

#define GETBYTE(v)                                                             \
  {                                                                            \
    if (zreader) {                                                             \
      if (zreader->eof()) return 0;                                            \
      v = zreader->nextByte();                                                 \
    } else {                                                                   \
      v = *s++;                                                                \
    }                                                                          \
  }

#define GETBIT(v)                                                              \
  {                                                                            \
    mask >>= 1;                                                                \
    if (mask == 0) {                                                           \
      mask = 128;                                                              \
      GETBYTE(bits)                                                            \
    }                                                                          \
    v = bits & mask;                                                           \
  }

// interlaced Elias-gamma value, first flag bit already known in `flag`
#define GAMMA(len, flag)                                                       \
  while (!(flag)) {                                                            \
    GETBIT(bit)                                                                \
    if (len & 0x8000) return 0;                                                \
    len = (len << 1) | (bit ? 1 : 0);                                          \
    GETBIT(flag)                                                               \
  }

// Decode until `until` is reached or the end marker is seen.
// Returns non zero when the stream is fully decoded.
unsigned char dzx0_run(unsigned char *until) {
  const unsigned char *s = zsrc;
  unsigned char *d = zdst;
  unsigned char mask = zmask;
  unsigned char bits = zbits;
  unsigned char state = zstate;
  unsigned char lit = zlit;
  unsigned int offset = zoffset;
  unsigned int pend = zpend;
  unsigned int len;
  unsigned int n;
  unsigned char flag;
  unsigned char bit;
  unsigned char b;

  if (zdone) {
    return 1;
  }

  for (;;) {
    if (pend) {
      n = (unsigned int)(until - d);
      if (n > pend) {
        n = pend;
      }
      if (n == 0) {
        break;
      }
      pend -= n;
      if (lit) {
        if (zreader) {
          if (zreader->readBytes(d, n) != n) return 0;
        } else {
          memcpy(d, s, n);
          s += n;
        }
        d += n;
      } else {
        if (offset > (unsigned int)(d - zstart)) return 0;
        const unsigned char *from = d - offset;
        do {
          *d++ = *from++;
        } while (--n);
      }
      if (pend) {
        break;
      }
      GETBIT(flag)
      state = flag ? ST_NEW : (lit ? ST_LAST : ST_LIT);
    }

    if (state == ST_NEW) {
      // handled before the `until` test so the end marker is detected as
      // soon as the last byte is produced
      len = 1;
      GETBIT(flag)
      GAMMA(len, flag)
      if (len == 256) {
        zdone = 1;
        break;
      }
      if (len > 255) return 0;
      GETBYTE(b)
      offset = len * 128 - (b >> 1);
      len = 1;
      flag = b & 1;
      GAMMA(len, flag)
      pend = len + 1;
      lit = 0;
      continue;
    }

    if (d >= until) {
      break;
    }

    len = 1;
    GETBIT(flag)
    GAMMA(len, flag)
    pend = len;
    lit = (state == ST_LIT);
  }

  zsrc = s;
  zdst = d;
  zmask = mask;
  zbits = bits;
  zstate = state;
  zlit = lit;
  zoffset = offset;
  zpend = pend;
  return zdone;
}
