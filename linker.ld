ENTRY (kmain)
STARTUP(kmain.o)

_TEXTMODE_BUFFER = 0xb8000;
. = 0x00008000;

SECTIONS
{
    .text ALIGN (0x1000) :
    {
        *(.text)
    }

    .rodata ALIGN (0x1000) :
    {
        *(.rodata*)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)
    }

    .bss :
    {
        START_BSS = .;
        *(COMMON)
        *(.bss)
        END_BSS = .;
    }

}
