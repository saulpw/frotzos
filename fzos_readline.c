#include "frotzos.h"

zchar
os_read_line(int max, zchar *buf, int timeout, int width, int continued)
{
    int j;
    int i = strlen(buf); // allow preloaded input
    int start_x = cursor_x;

    if (continued) {
        start_x -= i + 1;
    }

    const float endseconds = seconds + ((float) timeout)/10;

    do {
        int remainingTime = 0;
        if (timeout != 0) {
            remainingTime = (endseconds - seconds)*10;
            if (remainingTime <= 0) {
                remainingTime = 1;
            }
        }
        int ch = read_key(remainingTime, TRUE, TRUE);

        if (ch > 0x200) {
            switch (ch & 0xff) {
                case 'q':
                case 'x': return ZC_HKEY_QUIT;
                case 'r': return ZC_HKEY_RECORD;
                case 'd': return ZC_HKEY_DEBUG;
                case 's': return ZC_HKEY_SEED;
                case 'n': return ZC_HKEY_RESTART;
                case 'u': return ZC_HKEY_UNDO;
                case 'h': return ZC_HKEY_HELP;
                default:  break;
            };
        }

        switch (ch)
        {
        case 8:  // bksp
            if (i > 0) { strcpy(&buf[i-1], &buf[i]); --i; } break;
        case 13: return ch;                         // enter

        case 0x0161: i = 0; break;                      // ^a go to start

        case ZC_ARROW_LEFT:
        case 0x0162: if (i > 0) i -= 1; break;          // ^b move left

        case 0x0163: return -1;                         // ^c break

        case 0x0164: strcpy(&buf[i], &buf[i+1]); break; // ^d remove char
        case 0x0165: i = strlen(buf); break;            // ^e go to end

        case ZC_ARROW_RIGHT:
        case 0x0166: if (buf[i]) { i += 1; } break;     // ^f move right

        case 0x016b: memset(&buf[i], 0, max-i); break;  // ^k delete to end of line
        case 0x0174:                      // ^t swap char with previous
            j=buf[i]; buf[i]=buf[i-1]; buf[i-1]=j;
            ++i;
            break;

        case 0x0175: memset(buf, 0, max); i=0; break;   // ^u erase to start
        case 0x0177: break;                             // ^w erase word
        case ZC_ARROW_UP: break;
        case ZC_ARROW_DOWN: break;
        default:
            if (ch >= 32 && ch <= 126 && i < max-1) {
                if (buf[i] == 0)
                    buf[i+1] = 0;
                buf[i++] = ch;
            }
            break;
        };
      
        if (i < 0) i = 0;
        if (i > max-1) i = max;
        int eol = FALSE;
        for (j=0; j<max; ++j) {
            if (! buf[j]) { eol = TRUE; }
            setch(start_x + j, cursor_y, eol ? ' ' : buf[j], current_color);
        }
        cursor_x = start_x + i;
    } while (timeout == 0 || seconds < endseconds);

    return ZC_TIME_OUT;
}
