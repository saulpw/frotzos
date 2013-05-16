
#include "frotzos.h"

volatile float seconds = 0.0; // seconds since boot

void enable_interrupts();
void isr_keyboard();
void isr_timer();

static inline void outb( unsigned short port, unsigned char val ) {
    asm volatile( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline unsigned char inportb(unsigned int port) {
   unsigned char ret;
   asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
   return ret;
}

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

static
void timer_phase(int hz)
{
    int divisor = 1193180 / hz;
    outb(0x43, 0x36);             // Counter0, LSB then MSB, square wave
    outb(0x40, divisor & 0xFF);   // send LSB
    outb(0x40, divisor >> 8);     // send MSB
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

    // call timer isr at 100hz
    timer_phase(100);

    // enable interrupts on processor
    asm volatile ("sti");
}

void setch(int x, int y, char ch, char attr)
{
    volatile char *pos = screenpos(x, y);
    pos[0] = ch;
    pos[1] = attr;
}

void set_hw_cursor(int x, int y)
{
    unsigned short pos = (y-1) * 80 + x-1;
 
    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void isr_timer()
{
    seconds += .01;

    static const char spinny[] = "\\|/-";
    screenpos(80, 1)[0] = spinny[(int) (seconds*10) % (sizeof(spinny)-1)];
}

extern void key_released(int sc);
extern void key_pressed(int sc);

static unsigned char keyqueue[128];       // classic ring queue
static unsigned int kqfront=0, kqback=0; // (kqend - kqstart) % 16 == size

int pop_scancode()
{
    if (kqback == kqfront) { // queue is empty
        return -1;
    }

    unsigned char k = keyqueue[kqfront++];
    kqfront %= sizeof(keyqueue);
    return k;
}

void
isr_keyboard()
{
    int scancode = inportb(0x60);

    if ((kqback + 1) % sizeof(keyqueue) != kqfront)
    {
        keyqueue[kqback++] = scancode;
        kqback %= sizeof(keyqueue);
    } 
    // else os_fatal("kb queue is full");
}

void yield()
{
    asm volatile ("hlt");
}

void halt()
{
    asm volatile ("1: hlt; jmp 1b");
}

unsigned long long rdtsc(void)
{
    unsigned long long int x;
    asm volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}


