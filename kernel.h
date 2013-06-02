#ifndef KERNEL_H_
#define KERNEL_H_

extern void init_kernel();

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

#endif
