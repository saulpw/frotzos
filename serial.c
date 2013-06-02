#include <sys/types.h>
#include "io.h"

#define PORT 0x3f8   /* COM1 */

static int serial_initialized = 0;
 
void serial_init()
{
   out8(PORT + 1, 0x00);    // Disable all interrupts
   out8(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   out8(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   out8(PORT + 1, 0x00);    //                  (hi byte)
   out8(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   out8(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   out8(PORT + 4, 0x03);    // IRQs disabled, RTS/DSR set
}

static inline int serial_received()
{
   return in8(PORT + 5) & 1;
}
 
char serial_readbyte()
{
   if (!serial_initialized) {
       serial_init();
   }

   while (serial_received() == 0);
 
   return in8(PORT);
}

static inline int is_transmit_empty()
{
   return in8(PORT + 5) & 0x20;
}
 
static void serial_writebyte(char a)
{
   while (is_transmit_empty() == 0);
 
   out8(PORT,a);
}

void serial_write(const char *s)
{
    if (!serial_initialized) {
        serial_init();
    }

    char ch;
    while ((ch = *s++)) {
        serial_writebyte(ch);
    }
}

