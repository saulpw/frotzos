#include "io.h"
#include "kernel.h"

static unsigned char keyqueue[128];       // classic ring queue
static unsigned int kqfront=0, kqback=0; // (kqend - kqstart) % 16 == size

int
pop_scancode()
{
    if (kqback == kqfront) { // queue is empty
        return -1;
    }

    unsigned char k = keyqueue[kqfront++];
    kqfront %= sizeof(keyqueue);
    return k;
}

void
isr_keyboard()
{
    int scancode = in8(0x60);

    if ((kqback + 1) % sizeof(keyqueue) != kqfront)
    {
        keyqueue[kqback++] = scancode;
        kqback %= sizeof(keyqueue);
    } 
    // else os_fatal("kb queue is full");
}


