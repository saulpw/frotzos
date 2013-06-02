#include "kernel.h"
#include "io.h"
#include "vgatext.h"

volatile double seconds = 0.0; // seconds since boot

void setup_timer()
{
    int divisor = 1193180 / TIMER_HZ;
    out8(0x43, 0x36);             // Counter0, LSB then MSB, square wave
    out8(0x40, divisor & 0xFF);   // send LSB
    out8(0x40, divisor >> 8);     // send MSB
}

void isr_timer()
{
    seconds += (1.0 / TIMER_HZ);

    static const char spinny[] = "\\|/-";
    vga_charptr(80, 1)[0] = spinny[(int) (seconds) % (sizeof(spinny)-1)];
}

