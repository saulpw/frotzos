#ifndef _VGATEXT_H_
#define _VGATEXT_H_

typedef enum { 
    VGA_BLACK=0,
    VGA_BLUE=1,
    VGA_GREEN=2,
    VGA_CYAN=3,
    VGA_RED=4,
    VGA_MAGENTA=5,
    VGA_BROWN=6,
    VGA_WHITE=7,
    VGA_GREY=8,
    VGA_BRIGHT_BLUE=9,
    VGA_BRIGHT_GREEN=10,
    VGA_BRIGHT_CYAN=11,
    VGA_BRIGHT_RED=12,
    VGA_BRIGHT_MAGENTA=13,
    VGA_BRIGHT_BROWN=14,
    VGA_YELLOW=14,
    VGA_BRIGHT_WHITE=15,
} color_t;

unsigned char vga_color(color_t fg, color_t bg);

// x from 0-79, y from 0-24

void vga_set_cursor(int x, int y);

static inline volatile unsigned char *vga_charptr(int x, int y)
{
    volatile char *VGA_TEXT = (volatile char *) 0xb8000;
    int pos = y * 80 + x;
    return &VGA_TEXT[pos*2];
}

static inline unsigned char bold(unsigned char c) { return c | 0x08; }
static inline unsigned char blink(unsigned char c) { return c | 0x08; }

#endif
