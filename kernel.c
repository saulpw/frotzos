#include "kernel.h"
#include "dev/time.h"
#include "dev/ata.h"

void
init_kernel()
{
    DEBUG("starting frotzos\r\n");
    setup_timer();

    setup_interrupts(IDT_BASE);

    dma_pci_config();
}

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

void syscall_handler(int syscallnum)
{

}

void __assert_failure(const char *msg)
{
    kprintf("%s\r\n", msg);
    halt();
}

