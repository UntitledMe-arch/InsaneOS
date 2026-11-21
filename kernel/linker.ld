OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS {
    . = 0x100000;

    .text : {
        *(.text.entry)
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
