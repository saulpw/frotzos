#include "frotzos.h"

void os_display_num(int x, int y, int n, int base)
{
    static const char digits[] = "0123456789ABCDEF";

    char buf[16] = { 0 };
    int i=10;
    do { 
        int d = n % base;
        buf[i--] = digits[d];
        n /= base;
    } while (n > 0);

    int j=0;
    while (buf[++i]) 
    {
        screenpos(x+j, y)[0] = buf[i];
        j++;
    }
}

void
trace(const char *funcname, const char *filename, int lineno, const char *note)
{
    os_display_string(funcname);
    os_display_string("(");
    os_display_string(filename);
    os_display_string(":");
    os_display_num(cursor_x, cursor_y, lineno, 10);
    os_display_string(")");
    os_display_string(": ");
    os_display_string(note);
    os_display_char('\n');
}
