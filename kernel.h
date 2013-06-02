#ifndef KERNEL_H_
#define KERNEL_H_

extern void init_kernel();
extern void kprintf(const char *fmt, ...);

extern void yield();    // hlt, block until interrupt
extern void halt();     // hlt forever

#ifndef DEBUG
#define DEBUG(args...)
#endif

// interrupts
#define NUM_INTERRUPTS 64
#define IDT_BASE ((void *) 0x1000)

void setup_interrupts(void *idtaddr);

// timer
#define TIMER_HZ 100

extern volatile double seconds;        // seconds since start

extern void setup_timer();
extern void isr_timer();

static inline unsigned long long rdtsc(void) // read time-stamp counter
{
    unsigned long long int x;
    asm volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

// keyboard

extern int pop_scancode();
extern void isr_keyboard();

#endif
