#include "frotzos.h"

static void *nextalloc = (char *) 0x100000; // over the 1MB line

void *sbrk(int incr)
{
    void *ret = nextalloc;
    nextalloc += incr;
    if (nextalloc > (void *) 0x400000) {
        os_fatal("sbrk'ed more than 3MB");
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

