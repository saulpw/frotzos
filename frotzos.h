#ifndef FROTZOS_H_
#define FROTZOS_H_

#include <string.h>
#include "frotz.h"

extern volatile float seconds; // second since start

extern int cursor_x, cursor_y, current_color;
extern volatile char _TEXTMODE_BUFFER[];

static inline volatile char * screenpos(int x, int y) {
    int pos = (y-1) * 80 + x-1;
    return (volatile void *) &_TEXTMODE_BUFFER[pos*2];
}

// get the array of filenames
extern const char * const * enumfiles();

extern void setch(int x, int y, char ch, char attr);
extern void set_hw_cursor(int x, int y);

extern int pop_scancode(); // keyboard
extern int read_key(int timeout, int show_cursor, int readline); // keyboard

extern void yield();    // hlt, block until interrupt
extern void halt();     // hlt forever

extern void enable_interrupts();

// read time-stamp counter
extern unsigned long long rdtsc(void);

// debug functions
#define NOTIMPL TRACE(NOTIMPL)
#define TRACE(X) trace(__FUNCTION__, __FILE__, __LINE__, #X)
extern void trace(const char *func, const char *fn, int line, const char *note);
extern void os_display_num(int x, int y, int n, int base);

#endif
