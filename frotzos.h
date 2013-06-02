#ifndef FROTZOS_H_
#define FROTZOS_H_

#include <string.h>
#include "kernel.h"
#include "frotz.h"

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

// debug functions
#define NOTIMPL TRACE("NOT IMPLEMENTED")

#define TRACE(FMT, args...) DEBUG("%s [%s:%d] " FMT "\r\n", __FUNCTION__, __FILE__, __LINE__, ##args)

#ifndef DEBUG
#define DEBUG(args...)
#endif

extern void kprintf(const char *fmt, ...);

#endif
