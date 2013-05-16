#include "frotzos.h"

#define TAB ZC_INDENT // 9
#define ENTER ZC_RETURN // 13
#define BKSP ZC_BACKSPACE // 8
#define ESC ZC_ESCAPE // 27
#define F(N) (ZC_FKEY_MIN+N-1)
#define UP ZC_ARROW_UP
#define DOWN ZC_ARROW_DOWN
#define RIGHT ZC_ARROW_RIGHT
#define LEFT ZC_ARROW_LEFT
#define CAPSLK 0
#define NUMLK 0
#define SCRLLK 0
#define P(N) (ZC_NUMPAD_MIN+N-1)
#define DEL BKSP

// scancodes for shift keys
#define ALT    0x38
#define CTRL   0x1d
#define LSHIFT 0x2a
#define RSHIFT 0x36

static int depressed[256] = { 0 };

static const unsigned char kbdus[128] = {
  0,ESC,'1','2','3','4','5','6','7','8','9','0','-','=',BKSP,          // 00-0E
  TAB,'q','w','e','r','t','y','u','i','o','p','[',']',ENTER,           // 0F-1C
  0/*CTRL*/,'a','s','d','f','g','h','j','k','l',';','\'','`',          // 1D-29
  0/*LSHIFT*/,'\\','z','x','c','v','b','n','m',',','.','/',0/*RSHIFT*/,// 2A-36
  0/*PTSCR*/,0/*ALT*/,' ', CAPSLK,                                     // 37-3A
  F(1),F(2),F(3),F(4),F(5),F(6),F(7),F(8),F(9),F(10),NUMLK,SCRLLK,     // 3B-46
  P(7),P(8),P(9),'-', P(4),P(5),P(6),'+', P(1),P(2),P(3),P(0),DEL,     // 47-53
  0,0,0,F(11),F(12),                                                   // 54-58
  0, /* All other keys are undefined */
};

static const unsigned char kbdus_shift[128] = {
  0,ESC,'!','@','#','$','%','^','&','*','(',')','_','+',BKSP,          // 00-0E
  TAB,'Q','W','E','R','T','Y','U','I','O','P','{','}',ENTER,           // 0F-1C
  0/*CTRL*/,'A','S','D','F','G','H','J','K','L',':','\"','~',          // 1D-29
  0/*LSHIFT*/,'\\','Z','X','C','V','B','N','M','<','>','?',0/*RSHIFT*/,// 2A-36
  0/*PTSCR*/,0/*ALT*/,' ', CAPSLK,                                     // 37-3A
  F(1),F(2),F(3),F(4),F(5),F(6),F(7),F(8),F(9),F(10),NUMLK,SCRLLK,     // 3B-46
  '7','8','9','-', '4','5','6','+', '1','2','3','0','.',               // 47-53
  0,0,0,F(11),F(12),                                                   // 54-58
  0,
};

zchar
os_read_key(int timeout, int show_cursor)
{
    return read_key(timeout, show_cursor, FALSE);
}

int
read_key (int timeout, int show_cursor, int readline)
{
    if (show_cursor)
        set_hw_cursor(cursor_x, cursor_y);

    const float endseconds = seconds + ((float) timeout)/10;

    int extended = FALSE;

    do {
        int scancode = pop_scancode();
        if (scancode == -1) {
            yield();
        } else if (scancode == 0xe0) {
            extended = TRUE;
        } else if (scancode & 0x80) { // released
            depressed[scancode & 0x7f] = 0;
            extended = FALSE;
        } else {
            depressed[scancode] = 1;

            int shifts = depressed[ALT] << 9
                       | depressed[CTRL] << 8;

            char ch = 0;

            if (depressed[LSHIFT] || depressed[RSHIFT]) {
                ch = kbdus_shift[scancode];
            } else {
                ch = kbdus[scancode];
            }

            if (extended) {
                switch (scancode) {
                case 0x47: if (readline) return 0x0161; break; // home == ^a
                case 0x48: return ZC_ARROW_UP;    // up
                case 0x49: break;                 // pgup
                case 0x4b: return ZC_ARROW_LEFT;  // left
                case 0x4d: return ZC_ARROW_RIGHT; // right
                case 0x4f: if (readline) return 0x0165; break; // end == ^e
                case 0x50: return ZC_ARROW_DOWN;  // down
                case 0x51: break;                 // pgdn
                case 0x52: break;                 // ins
                case 0x53: break;                 // del
                };
            } else {
                if (!readline && shifts) {
                    return ZC_BAD;
                }

                if (ch) {
                    return ch | shifts;
                }
            }
#ifdef DEBUG
            os_display_num(scancode, 16);
#endif
        }
    } while (timeout == 0 || seconds < endseconds);

    return ZC_TIME_OUT;
}
