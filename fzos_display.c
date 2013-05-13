#include "frotzos.h"

int cursor_x=0, cursor_y=0;
int current_color = 0x7;

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

void os_set_font (int f)
{
    switch (f) {
    case TEXT_FONT:
    case PICTURE_FONT:
    case GRAPHICS_FONT:
    case FIXED_WIDTH_FONT:
    default:
        break;
    };
}

void os_set_text_style (int s)
{
    switch (s) {
    case REVERSE_STYLE:
        current_color = (current_color >> 4) | (current_color << 4);
        break;
    case BOLDFACE_STYLE:
    case EMPHASIS_STYLE:
        current_color |= 0x8;
        break;
    case FIXED_WIDTH_STYLE:
    default:
        break;
    };
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
    h_screen_rows = 25; // (aka screen height in lines)
    h_screen_width = h_screen_cols;
    h_screen_height = h_screen_rows;
    h_font_height = 1;
    h_font_width = 1;
    h_default_foreground = GREY_COLOUR;
    h_default_background = BLUE_COLOUR;
    h_interpreter_number = h_version == 6 ? INTERP_MSDOS : INTERP_DEC_20;
    h_interpreter_version = 'F';
}

void os_reset_screen (void)
{
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

void os_set_colour (int fg, int bg)
{
    current_color = (color_code(bg) << 4) | color_code(fg);
}

static inline int scrpos(int x, int y) {
    return (y-1) * 80 + x-1;
}

static inline void * screenpos(int x, int y) {
    return (void *) &_TEXTMODE_BUFFER[scrpos(x, y)*2];
}

static void set_hw_cursor()
{
    unsigned short pos = scrpos(cursor_x, cursor_y);
 
    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void
os_set_cursor (int y, int x)
{ 
    cursor_x = x;
    cursor_y = y;
    set_hw_cursor();
}

void setch(char ch)
{
    volatile char *pos = screenpos(cursor_x, cursor_y);
    pos[0] = ch;
    pos[1] = current_color;
}

static
void addch(char c)
{
    if (c == '\n')
    {
        cursor_x = 0;
#if 0
        if (++cursor_y > h_screen_rows) {
            os_scroll_area(0, 0, h_screen_rows, h_screen_cols, 1);
            cursor_y -= 1;
        }
#endif
    }
    else {
        setch(c);
        cursor_x += 1;
    }
    set_hw_cursor();
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
    zchar ch;
    while ((ch = *s++)) {
        switch (ch) {
        case ZC_NEW_FONT:
            os_set_font(*s++);
            break;
        case ZC_NEW_STYLE:
            os_set_text_style(*s++);
            break;
        default:
            os_display_char(ch);
            break;
        };
    };
}

void os_scroll_area (int top, int left, int bot, int right, int units)
{
    int y;
    for (y=top; y <= bot; ++y) {
        memcpy(screenpos(left, y), screenpos(left, y+units), (right-left)*2+2);
        os_erase_area(y+units, left, y+units, right);
    }
}

void os_erase_area (int top, int left, int bot, int right)
{
    int y, x;
    for (y=top; y <= bot; ++y) {
        for (x=left; x <= right; ++x) {
            volatile char *p = screenpos(x, y);
            p[0] = ' ';
            p[1] = current_color;
        }
    }
}

void os_more_prompt(void)
{
    int oldx = cursor_x;
    int oldy = cursor_y;
    os_display_string("[more]");
    os_read_key(0, TRUE);
    os_set_cursor(oldy, oldx);
    os_display_string("      ");
    os_set_cursor(oldy, oldx);
}

