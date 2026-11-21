[bits 16]
[org 0x0000]

stage2_entry:
    ; CS = 0x1000, IP = 0x0000
    cli
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFF0
    sti

    mov [boot_drive], dl

    ; 1. Detect memory via BIOS INT 15h, AX=0xE820
    ; Target buffer: physical 0x8000 -> segment 0x0000, offset 0x8000
    xor ax, ax
    mov es, ax
    mov di, 0x8000
    xor ebx, ebx
    xor bp, bp               ; bp = entry count

.e820_loop:
    mov eax, 0x0000E820
    mov edx, 0x534D4150      ; 'SMAP'
    mov ecx, 24              ; ask for 24 bytes
    int 0x15
    jc .e820_done
    cmp eax, 0x534D4150
    jne .e820_done

    inc bp                   ; valid entry
    add di, 24
    test ebx, ebx
    jnz .e820_loop

.e820_done:
    mov [mmap_entries], bp

    ; Restore ES to CS (0x1000)
    mov ax, cs
    mov es, ax

    ; 2. Load InsaneOS Kernel from disk (LBA 32, 128 sectors)
    ; Target: segment 0x2000:0x0000 (physical 0x20000)
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap_kernel
    int 0x13
    jc .load_error

    ; 3. Load CFS Image from disk (LBA 256, 128 sectors)
    ; Target: segment 0x4000:0x0000 (physical 0x40000)
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap_cfs
    int 0x13
    jc .load_error

    ; 4. Enable A20 Line
    call enable_a20

    ; 5. Enter 32-bit Protected Mode
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Far jump to flush prefetch queue and load CS with 0x08, EIP with physical 0x10000 + pmode_entry
    jmp dword 0x08:(0x10000 + pmode_entry)

.load_error:
    mov si, msg_load_err
    call print_string_16
.halt_loop:
    cli
    hlt
    jmp .halt_loop

print_string_16:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    mov dx, 0x3F8
    out dx, al
    jmp print_string_16
.done:
    ret

enable_a20:
    mov ax, 0x2401
    int 0x15
    jnc .done_a20
    in al, 0x92
    or al, 2
    out 0x92, al
.done_a20:
    ret

; =========================================================================
; 32-bit Protected Mode Entry Point (Physical address 0x10000 + pmode_entry)
; =========================================================================
[bits 32]
pmode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Copy Kernel from staging buffer (0x20000) to 1 MiB (0x100000)
    ; 128 sectors * 512 bytes = 65536 bytes = 16384 dwords
    mov esi, 0x20000
    mov edi, 0x100000
    mov ecx, 16384
    cld
    rep movsd

    ; Copy CFS image from staging buffer (0x40000) to 2 MiB (0x200000)
    mov esi, 0x40000
    mov edi, 0x200000
    mov ecx, 16384
    rep movsd

    ; Set up boot_info_t structure at physical 0x7000
    mov dword [0x7000], 0x494E534E      ; magic "INSN"
    mov dword [0x7004], 0x8000          ; mmap_addr
    movzx eax, word [0x10000 + mmap_entries]
    mov dword [0x7008], eax             ; mmap_count
    mov dword [0x700C], 0x200000        ; cfs_addr
    mov dword [0x7010], 65536           ; cfs_size
    mov dword [0x7014], 2               ; cursor_row

    ; Pass pointer to boot_info (0x7000) on stack
    push dword 0x7000

    ; Call InsaneOS Kernel entry point at 0x100000
    mov eax, 0x100000
    call eax

.hang:
    cli
    hlt
    jmp .hang

; =========================================================================
; Data and Structures (in segment 0x1000)
; =========================================================================
[bits 16]
boot_drive:     db 0
mmap_entries:   dw 0
msg_load_err:   db "[InBoot] Error: Failed to load kernel or CFS!", 13, 10, 0

align 4
dap_kernel:
    db 0x10       ; size
    db 0          ; reserved
    dw 128        ; 128 sectors (64 KiB)
    dw 0x0000     ; offset
    dw 0x2000     ; segment (0x2000:0x0000 = 0x20000)
    dq 32         ; start at LBA 32

align 4
dap_cfs:
    db 0x10       ; size
    db 0          ; reserved
    dw 128        ; 128 sectors (64 KiB)
    dw 0x0000     ; offset
    dw 0x4000     ; segment (0x4000:0x0000 = 0x40000)
    dq 256        ; start at LBA 256

align 8
gdt_start:
    ; 0x00: Null descriptor
    dq 0x0000000000000000

    ; 0x08: Kernel 32-bit Code Segment (Base=0, Limit=0xFFFFF, G=1, D=1, P=1, DPL=0, Type=0xA)
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    ; 0x10: Kernel 32-bit Data Segment (Base=0, Limit=0xFFFFF, G=1, B=1, P=1, DPL=0, Type=0x2)
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd 0x10000 + gdt_start     ; physical address of GDT
