#include "kernel.h"

void *sbrk(int incr)
{
    static void *nextalloc = (void *) HEAP_ADDR;
    void *ret = nextalloc;

    nextalloc += incr;
    if (nextalloc > (void *) HEAP_ADDR_MAX) {
        kprintf("sbrk(%u) would go over 0x%x", incr, HEAP_ADDR_MAX);
        halt();
    }
    return ret;
}
