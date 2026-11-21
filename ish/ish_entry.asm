[bits 32]
[global _start]
[extern ish_main]

section .text
_start:
    push dword [esp + 4]
    call ish_main
    add esp, 4
    ret
