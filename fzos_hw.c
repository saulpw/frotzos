
#include "frotzos.h"
#include "io.h"

volatile double seconds = 0.0; // seconds since boot

void enable_interrupts();
void isr_keyboard();
void isr_timer();
extern void page_fault();

static
void exception_handler(unsigned int excnum)
{
    if (excnum >= 0x30) {       // future syscalls
        return;
    }

    switch (excnum) {
    case 0x0E: page_fault(); break;
    case 0x20:  isr_timer(); break;
    case 0x21:  isr_keyboard(); break;

    case 0x00: // divide-by-zero
    case 0x01: // debug
    case 0x02: // NMI
    case 0x03: // breakpoint
    case 0x04: // overflow
    case 0x05: // bound range exceeded
    case 0x06: // invalid opcode
    case 0x07: // device not available
    case 0x08: // double fault
    case 0x09: // coprocessor segment overrun
    case 0x0a: // invalid tss
    case 0x0b: // segment not present
    case 0x0c: // stack-segment fault
    case 0x0d: // gpf
    case 0x10: // x87 fpe
    case 0x11: // alignment check
    case 0x12: // machine check
    case 0x13: // simd fpe
    case 0x1e: // security exception
    default:
        halt();
        break;
    };

    if (excnum >= 0x28) {        // IRQ8-15
        out8(0xa0, 0x20);       // EOI to slave PIC
    } 

    if (excnum >= 0x20) {
        out8(0x20, 0x20);       // EOI to master PIC
    }
}

static
void timer_phase(int hz)
{
    int divisor = 1193180 / hz;
    out8(0x43, 0x36);             // Counter0, LSB then MSB, square wave
    out8(0x40, divisor & 0xFF);   // send LSB
    out8(0x40, divisor >> 8);     // send MSB
}

static inline void a20wait()
{
    while (in8(0x64) & 0x2)
        ;
}

static inline void a20wait2()
{
    while (! (in8(0x64) & 0x1))
        ;
}

void
enable_A20()
{
    a20wait();
    out8(0x64, 0xad);
    a20wait();
    out8(0x64, 0xd0);
    a20wait2();
    int r = in8(0x60);
    a20wait();
    out8(0x64, 0xd1);
    a20wait();
    out8(0x60, r | 0x2);
    a20wait();
    out8(0x64, 0xae);
    a20wait();
}

void
enable_interrupts()
{
    // set up 8259 PIC for hardware interrupts at 0x20/0x28

    out8( 0x20, 0x11);  // expect ICW4
    out8( 0x21, 0x20);  // IRQ0 is int 0x20
    out8( 0x21, 0x04);  // slave is on IRQ2
    out8( 0x21, 0x01);  // manual EOI
    out8( 0x21, 0x0);   // unmask all ints

    out8( 0xA0, 0x11);  // expect ICW4
    out8( 0xA1, 0x28);  // IRQ8 is int 0x28
    out8( 0xA1, 0x02);  // i am attached to IRQ2
    out8( 0xA1, 0x01);  // manual EOI
    out8( 0xA1, 0x0);   // unmask all ints

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
    out8(0x3D4, 0x0F);
    out8(0x3D5, (unsigned char)(pos & 0xFF));
    // cursor HIGH port to vga INDEX register
    out8(0x3D4, 0x0E);
    out8(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void isr_timer()
{
    seconds += .01;

    static const char spinny[] = "\\|/-";
    screenpos(80, 1)[0] = spinny[(int) (seconds) % (sizeof(spinny)-1)];
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
    int scancode = in8(0x60);

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

