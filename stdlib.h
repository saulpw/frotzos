#ifndef FZ_STDLIB_H_
#define FZ_STDLIB_H_

typedef signed int ptrdiff_t;
typedef unsigned int size_t;
typedef signed int time_t;

#define NULL 0

#define assert(X) do { if (!(X)) { os_fatal("assert failed: " #X); } \
    } while (0)

extern time_t time(time_t *t);

extern void *malloc(size_t size);
extern void free(void *ptr);
extern void *realloc(void *ptr, size_t size);

extern void abort();

extern int errno;
extern void *sbrk(int incr);

#endif
