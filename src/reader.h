#ifndef READER_H
#define READER_H

typedef void (*ReaderInitFn)(void *);
typedef void (*ReaderResetFn)();
typedef int (*ReaderEofFn)();
typedef unsigned char (*ReaderNextByteFn)();
typedef void (*ReaderCloseFn)();
typedef unsigned int (*ReaderReadBytesFn)(unsigned char *, unsigned int);

typedef struct {
  ReaderInitFn init;
  ReaderResetFn reset;
  ReaderEofFn eof;
  ReaderNextByteFn nextByte;
  ReaderCloseFn close;
  ReaderReadBytesFn readBytes;
} Reader;

Reader* GetReader();
void SetReader(Reader* reader);
int ReaderHasError();

unsigned char GetNextValue();
// bulk read of whole bytes; returns 0 and flags an error on short read
unsigned char GetNextBytes(unsigned char *dst, unsigned int count);

#endif
