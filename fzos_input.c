#include "frotzos.h"

static unsigned char kbdus[128] =
{
    0,  ZC_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', ZC_BACKSPACE,	/* Backspace */
  ZC_INDENT,			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', ZC_RETURN,	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
    '*',
    0,	/* Alt */
    ' ',	/* Space bar */
    0,	/* Caps lock */
    ZC_FKEY_MIN,	/* 59 - F1 key ... > */
    ZC_FKEY_MIN+1,   ZC_FKEY_MIN+2,   ZC_FKEY_MIN+3,   ZC_FKEY_MIN+4,
    0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    ZC_HKEY_RESTART,	/* Home key */
    ZC_ARROW_UP,	/* Up Arrow */
    ZC_HKEY_RECORD,	/* Page Up */
    '-',
    ZC_ARROW_LEFT,	/* Left Arrow */
    0,
    ZC_ARROW_RIGHT,	/* Right Arrow */
    '+',
    ZC_HKEY_QUIT,	/* 79 - End key*/
    ZC_ARROW_DOWN,	/* Down Arrow */
    ZC_HKEY_PLAYBACK,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

static unsigned char kbdus_shift[128] =
{
    0,  ZC_ESCAPE, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', ZC_BACKSPACE,	/* Backspace */
  ZC_INDENT,			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', ZC_RETURN,	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '\"', '~',   0,		/* Left shift */
 '\\', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
    '*',
    0,	/* Alt */
    ' ',	/* Space bar */
    0,	/* Caps lock */
    ZC_FKEY_MIN,	/* 59 - F1 key ... > */
    ZC_FKEY_MIN+1,   ZC_FKEY_MIN+2,   ZC_FKEY_MIN+3,   ZC_FKEY_MIN+4,
    0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    ZC_HKEY_RESTART,	/* Home key */
    ZC_ARROW_UP,	/* Up Arrow */
    ZC_HKEY_RECORD,	/* Page Up */
    '-',
    ZC_ARROW_LEFT,	/* Left Arrow */
    0,
    ZC_ARROW_RIGHT,	/* Right Arrow */
    '+',
    ZC_HKEY_QUIT,	/* 79 - End key*/
    ZC_ARROW_DOWN,	/* Down Arrow */
    ZC_HKEY_PLAYBACK,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

static unsigned char kbdus_ctrl[128] =
{
    0,  ZC_ESCAPE, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', 31, '+', ZC_BACKSPACE,	/* Backspace */
  ZC_INDENT,			/* Tab */
  17, 23, 5, 18,	/* 19 */
  20, 25, 21, 9, 15, 16, 27, 29, ZC_RETURN,	/* Enter key */
    0,			/* 29   - Control */
  1, 19, 4, 6, 7, 8, 10, 11, 12, ':',	/* 39 */
 0, 0,   0,		/* Left shift */
 '\\', 26, 24, 3, 22, 2, 14,			/* 49 */
  13, '<', '>', '?',   0,				/* Right shift */
    '*',
    0,	/* Alt */
    ' ',	/* Space bar */
    0,	/* Caps lock */
    ZC_FKEY_MIN,	/* 59 - F1 key ... > */
    ZC_FKEY_MIN+1,   ZC_FKEY_MIN+2,   ZC_FKEY_MIN+3,   ZC_FKEY_MIN+4,
    0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    ZC_HKEY_RESTART,	/* Home key */
    ZC_ARROW_UP,	/* Up Arrow */
    ZC_HKEY_RECORD,	/* Page Up */
    '-',
    ZC_ARROW_LEFT,	/* Left Arrow */
    0,
    ZC_ARROW_RIGHT,	/* Right Arrow */
    '+',
    ZC_HKEY_QUIT,	/* 79 - End key*/
    ZC_ARROW_DOWN,	/* Down Arrow */
    ZC_HKEY_PLAYBACK,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

static char keyqueue[32];       // classic ring queue
static int kqfront=0, kqback=0; // (kqend - kqstart) % 16 == size

static void
push_key(char c)
{
    if (kqback != kqfront - 1) {    // queue is not full
        keyqueue[kqback++] = c;
        kqback %= sizeof(keyqueue);
    }
}

static char
pop_key(void)
{
    while (kqback == kqfront) {     // queue is empty
        yield();
    }

    char k = keyqueue[kqfront++];
    kqfront %= sizeof(keyqueue);
    return k;
}

zchar
os_read_key (int timeout, int show_cursor)
{
    return pop_key();
}

static int shifts[4];

static int isshift(char scancode)
{
    switch (scancode & ~0x80)
    {
        case 0x2a: return 1; // left-shift
        case 0x36: return 1; // right-shift
        case 0x1d: return 2; // left-ctrl
        case 0x38: return 3; // left-alt
        default:   break;
    };
    return 0;
}

static char
scancode_to_char(int sc)
{
    if (shifts[2]) {
        return kbdus_ctrl[sc];
    } else if (shifts[0] || shifts[1]) {
        return kbdus_shift[sc];
    } else {
        return kbdus[sc];
    }
}

void
isr_keyboard()
{
    int scancode = inportb(0x60);

    if (scancode & 0x80) { // released
        int s = isshift(scancode & ~0x80);
        if (s) {
            shifts[s] = 0;
        }
    } else {
        int s = isshift(scancode);
        if (s) {
            shifts[s] = 1;
        } else {
            push_key(scancode_to_char(scancode));
        }
    }
}

