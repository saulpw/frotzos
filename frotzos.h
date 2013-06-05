#ifndef FROTZOS_H_
#define FROTZOS_H_

#include <string.h>
#include "x86.h"
#include "frotz.h"

extern int cursor_x, cursor_y, current_color;
void setch(int x, int y, char ch, char attr);

// debug functions
#define NOTIMPL do { \
    os_display_string(__FUNCTION__); \
    os_display_string(": NOT IMPLEMENTED\n"); \
} while (0)

#define TRACE(FMT, args...) DEBUG("%s [%s:%d] " FMT "\r\n", __FUNCTION__, __FILE__, __LINE__, ##args)

#endif
