#include "frotzos.h"

void movecur(int dx, int dy)
{
    os_set_cursor(cursor_y+dy, cursor_x+dx);
}

zchar
os_read_line(int max, zchar *buf, int timeout, int width, int continued)
{
    int i = strlen(buf); // allow preloaded input

    while (1) {
        zchar ch = os_read_key(timeout, TRUE);

        if (max == 0) return ch;

        switch (ch) {
        case 1:  movecur(cursor_x - i, 0); i = 0; break; // ^a go to start
        case 2:  break; // ^b move left
        case 3:  return -1; // ^c
        case 4:  break; // ^d remove char at right of cursor
        case 5:  break; // ^e go to end of line
        case 6:  break; // ^f move right
        case 8:  if (i > 0) { 
                    buf[--i] = 0;
                    movecur(-1, 0);
                    setch(cursor_x, cursor_y, ' ', current_color);
                 }
                 break;
        case 11: break; // ^k delete to end of line
        case 13: buf[i] = 0; return ch;
        case 20: break; // ^t swap char with previous
        case 22: break; // ^u erase to start of line
        case 23: break; // ^w erase word
        case 26: break;
        case ZC_ARROW_LEFT:
        case ZC_ARROW_RIGHT:
        case ZC_ARROW_UP: break;
        case ZC_ARROW_DOWN: break;
        default:
            if (i < max-1) {
                buf[i++] = ch;
                os_display_char(ch);
            }
            break;
        };
    }
}
