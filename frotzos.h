#ifndef FROTZOS_H_
#define FROTZOS_H_

#include <string.h>
#include "frotz.h"

extern volatile double seconds; // second since start

extern int cursor_x, cursor_y, current_color;

extern const char *errmsg;

// get the array of filenames
extern struct fz_filehdr * const * enumfiles();

extern void setch(int x, int y, char ch, char attr);
extern void set_hw_cursor(int x, int y);

extern int pop_scancode(); // keyboard
extern int read_key(int timeout, int show_cursor, int readline); // keyboard

extern void yield();    // hlt, block until interrupt
extern void halt();     // hlt forever

extern void init_kernel();

// read time-stamp counter
extern unsigned long long rdtsc(void);

// debug functions
#define NOTIMPL TRACE("NOT IMPLEMENTED")

#define TRACE(FMT, args...) DEBUG("%s [%s:%d] " FMT "\r\n", __FUNCTION__, __FILE__, __LINE__, ##args)

#ifndef DEBUG
#define DEBUG(args...)
#endif

extern void os_display_num(int x, int y, int n, int base);
extern void kprintf(const char *fmt, ...);

#endif
