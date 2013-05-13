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
#define PAD(N) 0
#define DEL BKSP

static const unsigned char kbdus[128] = {
  0,ESC,'1','2','3','4','5','6','7','8','9','0','-','=',BKSP,          // 00-0E
  TAB,'q','w','e','r','t','y','u','i','o','p','[',']',ENTER,           // 0F-1C
  0/*CTRL*/,'a','s','d','f','g','h','j','k','l',';','\'','`',          // 1D-29
  0/*LSHIFT*/,'\\','z','x','c','v','b','n','m',',','.','/',0/*RSHIFT*/,// 2A-36
  0/*PTSCR*/,0/*ALT*/,' ', CAPSLK,                                     // 37-3A
  F(1),F(2),F(3),F(4),F(5),F(6),F(7),F(8),F(9),F(10),NUMLK,SCRLLK,     // 3B-46
  PAD(7),PAD(8),PAD(9),'-',
  PAD(4),PAD(5),PAD(6),'+',
  PAD(1),PAD(2),PAD(3),PAD(0),DEL,                                     // 47-53
  0,0,0,F(11),F(12),                                                   // 54-58
  0,/* All other keys are undefined */
};

static const unsigned char kbdus_shift[128] = {
  0,ESC,'!','@','#','$','%','^','&','*','(',')','_','+',BKSP,          // 00-0E
  TAB,'Q','W','E','R','T','Y','U','I','O','P','{','}',ENTER,           // 0F-1C
  0/*CTRL*/,'A','S','D','F','G','H','J','K','L',':','\"','~',          // 1D-29
  0/*LSHIFT*/,'\\','Z','X','C','V','B','N','M','<','>','?',0/*RSHIFT*/,// 2A-36
  0/*PTSCR*/,0/*ALT*/,' ', CAPSLK,                                     // 37-3A
  F(1),F(2),F(3),F(4),F(5),F(6),F(7),F(8),F(9),F(10),NUMLK,SCRLLK,     // 3B-46
  '7','8','9','-',
  '4','5','6','+',
  '1','2','3','0','.',                                                 // 47-53
  0,0,0,F(11),F(12),                                                   // 54-58
  0, };

static const unsigned char kbdus_ctrl[128] = {
  0,0,0,0,0,0,0,0,0,0,0,0,31,0,0,                                      // 00-0E
  0,17,23,5,18,20,25,21,9,15,16,27,29,0,                               // 0F-1C
  0/*CTRL*/,1,19,4,6,7,8,10,11,12,0,0,0,                               // 1D-29
  0/*LSHIFT*/,0,26,24,3,22,2,14,13,0,0,0,0/*RSHIFT*/,                  // 2A-36
  0/*PTSCR*/,0/*ALT*/,0, CAPSLK,                                       // 37-3A
  F(1),F(2),F(3),F(4),F(5),F(6),F(7),F(8),F(9),F(10),NUMLK,SCRLLK,     // 3B-46
  0,0,0,0, 0,0,0,0, 0,0,0,0,0,                                         // 47-53
  0,0,0,F(11),F(12),                                                   // 54-58
  0, };

static const unsigned char kbdus_alt[128] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                       // 00-0E
  0,ZC_HKEY_QUIT,0,0,ZC_HKEY_RECORD,0,0,ZC_HKEY_UNDO,0,0,0,0,0,0,      // 0F-1C
  0/*CTRL*/,0,ZC_HKEY_SEED,ZC_HKEY_DEBUG,0,0,ZC_HKEY_HELP,0,0,0,0,0,0, // 1D-29
  0/*LSHIFT*/,0,0,ZC_HKEY_QUIT,0,0,0,ZC_HKEY_RESTART,0,0,0,0,0,        // 2A-36
  0/*PTSCR*/,0/*ALT*/,0, CAPSLK,                                       // 37-3A
  F(1),F(2),F(3),F(4),F(5),F(6),F(7),F(8),F(9),F(10),NUMLK,SCRLLK,     // 3B-46
  0,0,0,0, 0,0,0,0, 0,0,0,0,0,                                         // 47-53
  0,0,0,F(11),F(12),                                                   // 54-58
  0, };

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
pop_key(int timeout)
{
    int startticks = ticks;

    while (kqback == kqfront) {     // queue is empty
        if (timeout && ticks - startticks > timeout) return ZC_TIME_OUT;
        yield();
    }

    char k = keyqueue[kqfront++];
    kqfront %= sizeof(keyqueue);
    return k;
}

zchar
os_read_key (int timeout, int show_cursor)
{
    if (show_cursor)
        set_hw_cursor(cursor_x, cursor_y);

    return pop_key(timeout*10);
}

static int shifts[8] = { 0 };

static int isshift(char scancode)
{
    switch (scancode & ~0x80)
    {
        case 0x2a: return 1; // left-shift
        case 0x36: return 2; // right-shift
        case 0x1d: return 3; // left-ctrl
        case 0x38: return 4; // left-alt
        default:   break;
    };
    return 0;
}

static char
scancode_to_char(int sc)
{
    if (shifts[4]) {
        return kbdus_alt[sc];
    } else if (shifts[3]) {
        return kbdus_ctrl[sc];
    } else if (shifts[1] || shifts[2]) {
        return kbdus_shift[sc];
    } else {
        return kbdus[sc];
    }
}

void key_released(int scancode)
{
    int s = isshift(scancode);
    if (s) {
        shifts[s] = 0;
    }
}

void key_pressed(int scancode)
{
    int s = isshift(scancode);
    if (s) {
        shifts[s] = 1;
    } else {
        int c = scancode_to_char(scancode);
        if (c)
            push_key(c);
        else 
            os_display_num(scancode, 16);
    }
}
