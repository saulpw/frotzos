#ifndef _IO_H_
#define _IO_H_

// x86 hardware I/O primitives

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

static inline void
out8(unsigned short port, u8 val)
{
    asm volatile( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline u8
in8(unsigned int port)
{
   u8 ret;
   asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
   return ret;
}

static inline void
out16(unsigned short port, u16 val)
{
    asm volatile( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

static inline u16
in16(unsigned int port)
{
   u16 ret;
   asm volatile ("inw %%dx,%%ax":"=a" (ret):"d" (port));
   return ret;
}

static inline void
out32(unsigned short port, u32 val)
{
    asm volatile( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

static inline u32
in32(unsigned int port)
{
   u32 ret;
   asm volatile ("inl %%dx,%%eax":"=a" (ret):"d" (port));
   return ret;
}

#endif
