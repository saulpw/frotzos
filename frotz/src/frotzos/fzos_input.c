#include "kernel.h"
#include "frotzos.h"
#include "dev/kb.h"
#include "dev/time.h"
#include "dev/vgatext.h"

inline zchar KeyToZchar(unsigned int k)
{
    if (k >= F1 && k <= F12)
        return k-F1+ZC_FKEY_MIN;
    if (k >= PAD0 && k <= PAD9)
        return k-PAD1+ZC_NUMPAD_MIN;

    switch (k)
    {
    case TAB: return ZC_INDENT;
    case ENTER: return ZC_RETURN;
    case BKSP: return ZC_BACKSPACE;
    case ESC: return ZC_ESCAPE;
    case DEL: return ZC_BACKSPACE;
    case UP: return ZC_ARROW_UP;
    case DOWN: return ZC_ARROW_DOWN;
    case RIGHT: return ZC_ARROW_RIGHT;
    case LEFT: return ZC_ARROW_LEFT;
    default:
        return k;
    };
}

unsigned int
read_key (int timeout, int show_cursor, int readline)
{
    if (show_cursor)
        vga_set_cursor(cursor_x, cursor_y);

    const double endseconds = seconds() + ((double) timeout)/10;

    int extended = FALSE;

    do {
        unsigned int key = get_key();
        if (key == 0) {
            yield();
            continue;
        }
        switch (key) {
        case HOME:  if (readline) return 1; break; // home == ^a
        case UP:    return ZC_ARROW_UP;    // up
        case LEFT:  return ZC_ARROW_LEFT;  // left
        case RIGHT: return ZC_ARROW_RIGHT; // right
        case END:   if (readline) return 5; break; // end == ^e
        case DOWN:  return ZC_ARROW_DOWN;  // down
        };

        if (!readline && (key & (ALT_FLAG | CTRL_FLAG)))
        {
            return ZC_BAD;
        }

        if (key > 0) {
            return key & ~SHIFT_FLAG;
        }
    } while (timeout == 0 || seconds() < endseconds);

    return ZC_TIME_OUT;
}

zchar
os_read_key(int timeout, int show_cursor)
{
    return read_key(timeout, show_cursor, FALSE);
}

zchar
os_read_line(int max, zchar *buf, int timeout, int width, int continued)
{
    int j;
    int i = strlen(buf); // allow preloaded input
    int start_x = cursor_x;

    if (continued) {
        start_x -= i + 1;
    }

    const double endseconds = seconds() + ((double) timeout)/10;

    do {
        int remainingTime = 0;
        if (timeout != 0) {
            remainingTime = (endseconds - seconds())*10;
            if (remainingTime <= 0) {
                remainingTime = 1;
            }
        }
        unsigned int ch = read_key(remainingTime, TRUE, TRUE);

        if (ch & ALT_FLAG) {
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

        if ((ch & SHIFT_MASK) == CTRL_FLAG) {
            ch &= 0xff;
            if (ch >= 'a' && ch <= 'z') {
                ch -= 'a' - 1;    // so ^a == 1
            } else if (ch >= '[' && ch <= '_') {
                ch -= '[' - 27;
            }
        }

        switch (ch)
        {
        case 8:  // bksp
            if (i > 0) { strcpy(&buf[i-1], &buf[i]); --i; } break;

        case 13: return ch;                         // enter

        case 1: i = 0; break;                      // ^a go to start

        case LEFT:
        case 2: if (i > 0) i -= 1; break;          // ^b move left

        case 3: return -1;                         // ^c break

        case 4: strcpy(&buf[i], &buf[i+1]); break; // ^d remove char
        case 5: i = strlen(buf); break;            // ^e go to end

        case RIGHT:
        case 6: if (buf[i]) { i += 1; } break;     // ^f move right

        case 11: memset(&buf[i], 0, max-i); break;  // ^k delete to end of line
        case 20:                      // ^t swap char with previous
            j=buf[i]; buf[i]=buf[i-1]; buf[i-1]=j;
            ++i;
            break;

        case 21: memset(buf, 0, max); i=0; break;   // ^u erase to start
        case 23: break;                             // ^w erase word
        case UP: break;
        case DOWN: break;
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
    } while (timeout == 0 || seconds() < endseconds);

    return ZC_TIME_OUT;
}

int
os_read_file_name (char *fn, const char *default_fn, int flag)
{
    char prompt[80] = { 0 };

    snprintf(prompt, 80, "Filename to save as [%s]: ", default_fn);
    os_display_string(prompt);

    // XXX: check for quit characters
    os_read_line(MAX_FILE_NAME, fn, 0, 80 - strlen(prompt), 0);

    os_display_string("\n");

    if (!fn[0]) {
        int len = strlen(default_fn);
        strncpy(fn, default_fn, MAX_FILE_NAME);
        fn[len] = 0; 
    }

    DPRINT(1, "using filename '%s'", fn);
    return 1;
}

