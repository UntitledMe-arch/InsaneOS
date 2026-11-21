[bits 32]
[global _start]
[extern kmain]

section .text
_start:
    ; Set up clean 32-bit stack
    mov esp, 0x90000

    ; Enable SSE in CR0 and CR4
    mov eax, cr0
    and ax, 0xFFFB      ; Clear EM (bit 2)
    or ax, 0x0002       ; Set MP (bit 1)
    mov cr0, eax

    mov eax, cr4
    or ax, 0x0600       ; Set OSFXSR (bit 9) and OSXMMEXCPT (bit 10)
    mov cr4, eax

    ; Pointer to boot_info_t is at 0x7000
    push dword 0x7000
    call kmain

.halt:
    cli
    hlt
    jmp .halt
