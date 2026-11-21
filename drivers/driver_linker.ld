OUTPUT_FORMAT("binary")
ENTRY(driver_init)

SECTIONS {
    . = 0x0;
    .header : {
        *(.header)
    }
    . = ALIGN(16);
    .text : {
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
        *(.note*)
        *(.eh_frame*)
        *(.comment*)
    }
}
