#include "frotzos.h"

int cursor_x=0, cursor_y=0;
int current_color = 0xf;

#define STRIDE (80 * 2)

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

int os_font_data (int font, int *height, int *width)
{
    if (font == TEXT_FONT) {
      *height = 1; *width = 1; return 1;
    }
    return 0;
}

void os_set_font (int x)
{
//    NOTIMPL;
}

void os_set_text_style (int x)
{
    NOTIMPL;
}

int os_char_width (zchar c)
{
    // XXX: what about ZC_INDENT and ZC_GAP?
    return 1;
}

int os_string_width (const zchar *s)
{
    int width = 0;
    zchar c;

    while ((c = *s++) != 0) {
        if (c == ZC_NEW_STYLE || c == ZC_NEW_FONT) {
            s++;
            /* No effect */
        } else {
            width += os_char_width(c);
        }
    }

    return width;
}

void os_init_screen(void)
{
    h_config = CONFIG_COLOUR;      //(aka flags 1)
    h_flags = COLOUR_FLAG;         //(aka flags 2)
    h_screen_cols = 80; // (aka screen width in characters)
    h_screen_rows = 24; // (aka screen height in lines)
    h_screen_width = h_screen_cols;
    h_screen_height = h_screen_rows;
    h_font_height = 1;
    h_font_width = 1;
    h_default_foreground = WHITE_COLOUR;
    h_default_background = BLUE_COLOUR;
    h_interpreter_number = h_version == 6 ? INTERP_MSDOS : INTERP_DEC_20;
    h_interpreter_version = 'F';
}

void os_reset_screen (void)
{
    NOTIMPL;
}

void
os_set_cursor (int y, int x)
{ 
    cursor_x = x;
    cursor_y = y;
}

static
unsigned int color_code(int c)
{
    switch (c) {
    case BLACK_COLOUR:   return 0;
    case BLUE_COLOUR:    return 1;
    case GREEN_COLOUR:   return 2;
    case CYAN_COLOUR:    return 3;
    case RED_COLOUR:     return 4;
    case MAGENTA_COLOUR: return 5;
    case YELLOW_COLOUR:  return 6 + 8;  // bright brown
    case WHITE_COLOUR:   return 7 + 8;  // bright grey
    case GREY_COLOUR:
//    case LIGHTGREY_COLOUR:
    case MEDIUMGREY_COLOUR:
    case DARKGREY_COLOUR:
//    case DEFAULT_COLOUR:
    default:
        return 7;
    };
}

void 	os_set_colour (int fg, int bg)
{
    current_color = (color_code(bg) << 4) | color_code(fg);
}

static inline void * screenpos(int x, int y) {
    return (void *) &_TEXTMODE_BUFFER[(y-1)*STRIDE + (x-1)*2];
}

static
void addch(char c)
{
    if (c == '\n')
    {
        yield();
        cursor_x = 0;
        if (++cursor_y > h_screen_rows) {
            os_scroll_area(0, 0, h_screen_rows, h_screen_cols, 1);
            cursor_y -= 1;
        }
    }
    else {
        volatile char *curpos = screenpos(cursor_x, cursor_y);
        curpos[0] = c;
        curpos[1] = current_color;
        cursor_x += 1;
    }
    // XXX: move screen cursor
}

void os_display_char(zchar c)
{
    switch (c) {
    case ZC_INDENT:
        addch(' '); addch(' '); addch(' ');
        break;
    case ZC_GAP:
        addch(' '); addch(' ');
        break;
    default:
        addch(c);
        break;
    };
}

void os_display_string(const zchar *s)
{
    while (*s) {
        os_display_char(*s++);
    }
}

void os_scroll_area (int top, int left, int bot, int right, int units)
{
    int y;
    for (y=top; y < bot; ++y) {
        memcpy(screenpos(left, y), screenpos(left, y+units), (right-left)*2);
        memset(screenpos(left, y+units), 0, (right-left)*2);
    }
}

void os_erase_area (int top, int left, int bot, int right)
{
    int y, x;
    for (y=top; y < bot; ++y) {
        for (x=left; x < right; ++x) {
            volatile char *p = screenpos(x, y);
            p[0] = ' ';
            p[1] = current_color;
        }
    }
}

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


