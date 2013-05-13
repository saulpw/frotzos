#include "frotzos.h"

void *malloc(size_t size)
{
    static char *nextalloc = (char *) 0x100000; // over the 1MB line

    char *ret = nextalloc;
    nextalloc += size;
    if (nextalloc > (char *) 0x300000) {
        os_fatal("malloc'ed more than 3MB");
    }
    return ret;
}

void free(void *ptr)
{
}

void *realloc(void *ptr, size_t size)
{
    void *nptr = malloc(size);
    if (ptr)
        memcpy(nptr, ptr, size); // XXX: min(oldsize, newsize)
    return nptr;
}

void __stack_chk_fail(void)
{ 
    os_fatal("stack_chk_fail");
}

