#include "frotzos.h"

static unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};		

char keyqueue[32];       // classic ring queue
int kqfront=0, kqback=0; // (kqend - kqstart) % 16 == size

void
push_key(char c)
{
    if (kqback != kqfront - 1) {    // queue is not full
        keyqueue[kqback++] = c;
        kqback %= sizeof(keyqueue);
    }
}

char
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

int shifts[4];
int shift=0, ctrl=0, alt=0;

int isshift(char scancode)
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

char
scancode_to_char(int sc)
{
    return kbdus[sc];
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

