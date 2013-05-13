#include "frotzos.h"

void os_display_num(int n, int base)
{
    static const char digits[] = "0123456789ABCDEF";
    char buf[16] = { 0 };
    int i=10;
    do { 
        int d = n % base;
        buf[i--] = digits[d];
        n /= base;
    } while (n > 0);
    os_display_string(&buf[i+1]);
}

void
trace(const char *funcname, const char *filename, int lineno, const char *note)
{
    os_display_string(funcname);
    os_display_string("(");
    os_display_string(filename);
    os_display_string(":");
    os_display_num(lineno, 10);
    os_display_string(")");
    os_display_string(": ");
    os_display_string(note);
    os_display_char('\n');
}
