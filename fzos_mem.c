#include "frotzos.h"

static void *nextalloc = (void *) HEAP_ADDR;

void *sbrk(int incr)
{
    void *ret = nextalloc;
    nextalloc += incr;
    if (nextalloc > (void *) HEAP_ADDR_MAX) {
        kprintf("sbrk(%u) would go over 0x%x", incr, HEAP_ADDR_MAX);
        halt();
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

