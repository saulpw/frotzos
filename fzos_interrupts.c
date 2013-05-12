
#include "frotzos.h"

static
void exception_handler(unsigned int excnum)
{
    if (excnum < 0x20) {        // processor exception
        halt();                 // noreturn
    }

    if (excnum >= 0x30) {       // future syscalls
        return;
    }

    switch (excnum) {
    case 0x20:  isr_timer(); break; // timer
    case 0x21:  isr_keyboard(); break; // keyboard
    default: break;
    };

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

    // boot loader stage1 isr will call this function
    *(void **) 0x7df8 = (void *) &exception_handler;

    // enable interrupts on processor
    asm volatile ("sti");
}

void
isr_timer()
{
    static const char spinny[] = "\\|/-";
    static int i = 0;
    _TEXTMODE_BUFFER[158] = spinny[i++];
    i %= sizeof(spinny);
}
