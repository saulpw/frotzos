#ifndef FZ_STDLIB_H_
#define FZ_STDLIB_H_

#include <stdint.h>

typedef signed int ptrdiff_t;
typedef signed int time_t;

#define NULL ((void *) 0)

extern time_t time(time_t *t);

extern void *malloc(size_t size);
extern void free(void *ptr);
extern void *realloc(void *ptr, size_t size);

extern void abort();

extern int errno;
extern void *sbrk(int incr);

#endif
