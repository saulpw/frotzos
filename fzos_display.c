#include "frotz.h"

static inline
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

int  	os_font_data (int font, int *height, int *width)
{
    if (font == TEXT_FONT) {
      *height = 1; *width = 1; return 1;
    }
    return 0;
}
void 	os_set_cursor (int x, int y) { NOTIMPL; }
void 	os_set_font (int x) { NOTIMPL; }
void 	os_set_text_style (int x) { NOTIMPL; }
int  	os_string_width (const zchar *s) { NOTIMPL; return 1; }

void os_init_screen(void)
{
}
void os_reset_screen (void)
{
}

int cursor_x=0, cursor_y=1;
int height = 24, width = 80;
int current_color = 0xf;

void 	os_set_colour (int fg, int bg)
{ 
//    NOTIMPL;
}

void os_display_char(zchar c)
{
#if 0
    if (current_color != 0xf)
    {
        volatile char *curpos = &_TEXTMODE_BUFFER[(cursor_y*width + cursor_x)*2];
        T(Z)
        curpos[0] = c;
        curpos[1] = 0xf;
        current_color = 0xf;
        memory_dump(((const char *) &current_color) - 256, 16);
        halt();
    }
#endif
    if (c == '\n')
    {
        yield();
        cursor_x = 0;
        if (++cursor_y > height) {
            os_scroll_area(0, 0, height, width, 1);
            cursor_y -= 1;
        }
    }
    else {
        volatile char *curpos = &_TEXTMODE_BUFFER[(cursor_y*width + cursor_x)*2];
        curpos[0] = c;
        curpos[1] = current_color;
        cursor_x += 1;
    }
    // XXX: move screen cursor
}

void os_display_string(const zchar *s)
{
    while (*s) {
        os_display_char(*s++);
    }
}

static inline void * screenpos(int x, int y) {
    return (void *) &_TEXTMODE_BUFFER[(y*width + x)*2];
}

void os_scroll_area (int top, int left, int bot, int right, int units)
{
    int y;
    for (y=top; y < bot; ++y) {
        memcpy(screenpos(left, y), screenpos(left, y+units), (right-left)*2);
        memset(screenpos(left, y+units), 0, (right-left)*2);
    }
}


int  	os_char_width (zchar c) { NOTIMPL; return 1; }
void 	os_erase_area (int top, int left, int bot, int right) { NOTIMPL; }

void memory_dump(const char *ptr, int len)
{
    int i;
    for (i=0; i < len; i++) {
        os_display_num(ptr[i], 16);
        if (i % 16 == 0) 
            os_display_char('\n');
        else
            os_display_char(' ');
    }
}


