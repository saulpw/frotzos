#include "frotzos.h"
#include "vgatext.h"

int cursor_x=1, cursor_y=24;
int current_color = 0x17;
static int current_style = 0;
static int current_fg = GREY_COLOUR, current_bg = BLUE_COLOUR;

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
    h_config = CONFIG_COLOUR | CONFIG_EMPHASIS | CONFIG_BOLDFACE 
             | CONFIG_FIXED | CONFIG_TIMEDINPUT;
    h_flags = UNDO_FLAG | COLOUR_FLAG;         //(aka flags 2)
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

    os_erase_area(1,1,26,80); // gets rid of discoloration on first scroll
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

static
void compute_current_color()
{
    int fg, bg;

    if (current_style & REVERSE_STYLE) {
        bg = color_code(current_fg);
        fg = color_code(current_bg);
    } else {
        fg = color_code(current_fg);
        bg = color_code(current_bg);
    }

    if (current_style & BOLDFACE_STYLE) fg |= 0x08;
    if (current_style & EMPHASIS_STYLE) bg |= 0x08;

    current_color = (bg << 4) | fg;
}

void os_set_colour (int fg, int bg)
{
    current_fg = fg;
    current_bg = bg;
    compute_current_color();
}

void os_set_text_style (int s)
{
    current_style = s;
    compute_current_color();
}

void
os_set_cursor (int y, int x)
{ 
    cursor_x = x;
    cursor_y = y;
}

static
void addch(char c)
{
    if (c == '\n')
    {
        cursor_x = 1;
        if (++cursor_y > 25) {
            os_scroll_area(1, 1, 25, 80, 1);
            cursor_y -= 1;
        }
    }
    else {
        setch(cursor_x, cursor_y, c, current_color);
        cursor_x += 1;
    }
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
        memcpy((void *) vga_charptr(left, y), (void *) vga_charptr(left, y+units), (right-left)*2+2);
        os_erase_area(y+units, left, y+units, right);
    }
}

void os_erase_area (int top, int left, int bot, int right)
{
    int y, x;
    for (y=top; y <= bot; ++y) {
        for (x=left; x <= right; ++x) {
            setch(x, y, ' ', current_color);
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

