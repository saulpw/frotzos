ENTRY (kmain)
STARTUP(kmain.o)

_TEXTMODE_BUFFER = 0xb8000;
. = 0x00008020;

SECTIONS
{
    .text ALIGN (0x04) :
    {
        *(.text)
    }

    .rodata ALIGN (0x04) :
    {
        *(.rodata*)
    }

    .data ALIGN (0x04) :
    {
        *(.data)
    }

    .bss ALIGN (0x04) :
    {
        START_BSS = .;
        *(COMMON)
        *(.bss)
        END_BSS = .;
    }

    .heap ALIGN (0x1000):
    {
        HEAP = .;
    }

}
