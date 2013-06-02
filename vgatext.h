#ifndef _VGATEXT_H_
#define _VGATEXT_H_

static inline volatile char *vga_charptr(int x, int y)
{
    volatile char *VGA_TEXT = (volatile char *) 0xb8000;
    int pos = (y-1) * 80 + x-1;
    return &VGA_TEXT[pos*2];
}

void vga_set_cursor(int x, int y);


#endif
