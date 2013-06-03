#ifndef FROTZOS_H_
#define FROTZOS_H_

#include <string.h>
#include "kernel.h"
#include "frotz.h"

extern int cursor_x, cursor_y, current_color;

extern const char *errmsg;

extern void setch(int x, int y, char ch, char attr);
extern int read_key(int timeout, int show_cursor, int readline);
// debug functions
#define NOTIMPL TRACE("NOT IMPLEMENTED")

#define TRACE(FMT, args...) DEBUG("%s [%s:%d] " FMT "\r\n", __FUNCTION__, __FILE__, __LINE__, ##args)

#endif
