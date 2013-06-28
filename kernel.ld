ENTRY (kmain)
STARTUP(kmain.o)

. = 0x008200;

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
}
