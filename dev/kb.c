#include "x86.h"
#include "kernel.h"
#include "dev/kb.h"

static const unsigned char kbdus[128] = {
  0,ESC,'1','2','3','4','5','6','7','8','9','0','-','=',BKSP,          // 00-0E
  TAB,'q','w','e','r','t','y','u','i','o','p','[',']',ENTER,           // 0F-1C
  0/*CTRL*/,'a','s','d','f','g','h','j','k','l',';','\'','`',          // 1D-29
  0/*LSHIFT*/,'\\','z','x','c','v','b','n','m',',','.','/',0/*RSHIFT*/,// 2A-36
  0/*PTSCR*/,0/*ALT*/,' ', CAPSLK,                                     // 37-3A
  F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLK,SCRLLK,                         // 3B-46
  PAD7,PAD8,PAD9,'-', PAD4,PAD5,PAD6,'+', PAD1,PAD2,PAD3,PAD0,DEL,     // 47-53
  0,0,0,F11,F12,                                                       // 54-58
  0, /* All other keys are undefined */
};

static const unsigned char kbdus_shift[128] = {
  0,ESC,'!','@','#','$','%','^','&','*','(',')','_','+',BKSP,          // 00-0E
  TAB,'Q','W','E','R','T','Y','U','I','O','P','{','}',ENTER,           // 0F-1C
  0/*CTRL*/,'A','S','D','F','G','H','J','K','L',':','\"','~',          // 1D-29
  0/*LSHIFT*/,'\\','Z','X','C','V','B','N','M','<','>','?',0/*RSHIFT*/,// 2A-36
  0/*PTSCR*/,0/*ALT*/,' ', CAPSLK,                                     // 37-3A
  F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLK,SCRLLK,                         // 3B-46
  '7','8','9','-', '4','5','6','+', '1','2','3','0','.',               // 47-53
  0,0,0,F11,F12,                                                       // 54-58
  0,
};

static unsigned char keyqueue[128];       // classic ring queue
static unsigned int kqfront=0, kqback=0; // (kqend - kqstart) % 16 == size

void
isr_keyboard()
{
    int scancode = in8(0x60);

    if ((kqback + 1) % sizeof(keyqueue) != kqfront)
    {
        keyqueue[kqback++] = scancode;
        kqback %= sizeof(keyqueue);
    } 
    // else "kb queue is full";
}

unsigned int
get_scancode()
{
    if (kqback == kqfront) { // queue is empty
        return 0;
    }

    unsigned char k = keyqueue[kqfront++];
    kqfront %= sizeof(keyqueue);
    return k;
}

unsigned int
get_key()
{    
    static int depressed[256] = { 0 }; // currently depressed keys
    static int extended = 0;

    while (1)
    {
        int scancode = get_scancode();
        if (scancode == 0) {
            return 0;
        } else if (scancode == 0xe0) {
            extended = 1;
            continue;
        } else if (scancode & 0x80) { // released
            depressed[scancode & 0x7f] = 0;
            extended = 0;
            continue;
        }

        depressed[scancode] = 1;

        int shifts = 0;

        if (depressed[CTRL]) shifts |= CTRL_FLAG;
        if (depressed[ALT]) shifts |= ALT_FLAG;
        if (depressed[LSHIFT] || depressed[RSHIFT]) shifts |= SHIFT_FLAG;


        unsigned char ch = 0;

        if (extended) {
            switch (scancode) {
            case 0x47: ch = HOME; break;
            case 0x48: ch = UP; break;
            case 0x49: ch = PGUP; break;
//            case 0x4a: ch = ???; break;
            case 0x4b: ch = LEFT; break;
            case 0x4c: ch = CENTER; break;
            case 0x4d: ch = RIGHT; break;
//            case 0x4e: ch = ???; break;
            case 0x4f: ch = END; break;
            case 0x50: ch = DOWN; break;
            case 0x51: ch = PGDN; break;
            case 0x52: ch = INS; break;
            case 0x53: ch = DEL; break;
            default: break;
            };
        } else {
            if (depressed[LSHIFT] || depressed[RSHIFT]) {
                ch = kbdus_shift[scancode];
            } else {
                ch = kbdus[scancode];
            }
        }

        if (ch > 0)
            return ch | shifts;

//        DEBUG("unknown scancode 0x%x\r\n", scancode);
    }
}
