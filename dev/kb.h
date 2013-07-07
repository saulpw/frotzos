#ifndef DEV_KB_H_
#define DEV_KB_H_

// special keys from get_key()
enum {
    BKSP=8,
    TAB=9,
    ENTER=13,
    ESC=27,
    CAPSLK=0x80,
    NUMLK,
    SCRLLK,
    DEL, INS,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    PAD0, PAD1, PAD2, PAD3, PAD4, PAD5, PAD6, PAD7, PAD8, PAD9,
    UP, DOWN, LEFT, RIGHT, CENTER, PGUP, PGDN, HOME, END
};

// shift flags from get_key()
#define CTRL_FLAG (1 << 8)
#define ALT_FLAG (1 << 9)
#define SHIFT_FLAG (1 << 10)
#define SHIFT_MASK  (CTRL_FLAG | ALT_FLAG | SHIFT_FLAG)

// returns -1 if no key has been pressed
// returns 0 if no 'interesting' key has been pressed (
// otherwise returns key:
//    ascii if < 127, mnemonic as above enum
//    OR'ed with the pressed shift flags (above)
unsigned int get_key();

// scancodes for shift keys from get_scancode()
#define ALT    0x38
#define CTRL   0x1d
#define LSHIFT 0x2a
#define RSHIFT 0x36

unsigned int get_scancode();

void isr_keyboard();

#endif
