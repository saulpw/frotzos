
#include "frotzos.h"

static inline
void outb( unsigned short port, unsigned char val )
{
    asm volatile( "outb %0, %1"
                  : : "a"(val), "Nd"(port) );
}

    
static
void exception_handler(unsigned int excnum)
{
    if (excnum < 0x20) {        // processor exception
        halt();                 // noreturn
    }

    if (excnum > 0x28) {        // IRQ8-15
        outb(0xa0, 0x20);       // EOI to slave PIC
    } 

    outb(0x20, 0x20);       // EOI to master PIC
}

void
enable_interrupts()
{
    // set up 8259 PIC for hardware interrupts at 0x20/0x28

    outb( 0x20, 0x11);  // expect ICW4
    outb( 0x21, 0x20);  // IRQ0 is int 0x20
    outb( 0x21, 0x04);  // slave is on IRQ2
    outb( 0x21, 0x01);  // manual EOI
    outb( 0x21, 0x0);   // unmask all ints

    outb( 0xA0, 0x11);  // expect ICW4
    outb( 0xA1, 0x28);  // IRQ8 is int 0x28
    outb( 0xA1, 0x02);  // i am attached to IRQ2
    outb( 0xA1, 0x01);  // manual EOI
    outb( 0xA1, 0x0);   // unmask all ints

    *(void **) 0x7df8 = (void *) &exception_handler;

    asm volatile ("sti");
}

