ENTRY(_start)

SECTIONS
{
    . = 1M;

    .text : {
        *(.multiboot)
        *(.text)
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .stack (NOLOAD) : {
        _stack_start = .;
        . = . + 0x4000; /* 16 KB kernel stack */
        _stack_end = .;
    }

    .bss : {
        *(.bss)
    }
}