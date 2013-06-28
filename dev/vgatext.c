#include "x86.h"
#include "vgatext.h"

void vga_set_cursor(int x, int y)
{
    unsigned short pos = (y-1) * 80 + x-1;
 
    // cursor LOW port to vga INDEX register
    out8(0x3D4, 0x0F);
    out8(0x3D5, (unsigned char)(pos & 0xFF));
    // cursor HIGH port to vga INDEX register
    out8(0x3D4, 0x0E);
    out8(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

unsigned char vga_color(color_t fg, color_t bg)
{
    return (bg << 4) | fg;
}

