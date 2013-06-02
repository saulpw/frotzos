
#include "io.h"
#include "vgatext.h"
#include "kernel.h"

void
init_kernel()
{
    DEBUG("starting frotzos\r\n");
    setup_timer();

    setup_interrupts(IDT_BASE);
}

void setch(int x, int y, char ch, char attr)
{
    vga_charptr(x, y)[0] = ch;
    vga_charptr(x, y)[1] = attr;
}

void yield()
{
    asm volatile ("hlt");
}

void halt()
{
    asm volatile ("1: hlt; jmp 1b");
}

