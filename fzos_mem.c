#include "frotzos.h"

void *malloc(size_t size)
{
    static char *nextalloc = (char *) 0x90000;

    char *ret = nextalloc;
    nextalloc += size;
    if (nextalloc > (char *) 0xA0000) {
        os_fatal("malloc'ed more than 64k");
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

