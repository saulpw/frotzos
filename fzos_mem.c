#include "frotzos.h"

extern char HEAP[]; // after the bss
static void *nextalloc = HEAP;

void *sbrk(int incr)
{
    void *ret = nextalloc;
    nextalloc += incr;
    if (nextalloc > (void *) 0xF00000) {
        os_fatal("sbrk'ed more than 14MB");
    }
    return ret;
}

#ifdef NO_MALLOC
void *malloc(int size) { return sbrk(size); }

void free(void *ptr) { }

void *realloc(void *ptr, size_t size)
{
    void *nptr = malloc(size);
    if (ptr)
        memcpy(nptr, ptr, size); // XXX: min(oldsize, newsize)
    return nptr;
}
#endif

void __stack_chk_fail(void)
{ 
    os_fatal("stack_chk_fail");
}

