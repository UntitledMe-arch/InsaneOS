[bits 16]
[org 0x7c00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    mov [boot_drive], dl

    ; Display bootloader message
    mov si, msg_loading
    call print_string

    ; Check for INT 13h extensions
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc .disk_error
    cmp bx, 0xAA55
    jne .disk_error

    ; Load Stage 2 from LBA 1 (31 sectors) to 0x1000:0x0000
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap_stage2
    int 0x13
    jc .disk_error

    ; Transfer control to Stage 2 with boot drive in DL
    mov dl, [boot_drive]
    jmp 0x1000:0x0000

.disk_error:
    mov si, msg_err
    call print_string
.halt:
    cli
    hlt
    jmp .halt

print_string:
    push ax
    push bx
    push dx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    mov dx, 0x3F8
    out dx, al
    jmp .loop
.done:
    pop dx
    pop bx
    pop ax
    ret

boot_drive: db 0

msg_loading: db "[InBoot] Loading the kernel...", 13, 10, 13, 10, 0
msg_err:     db "[InBoot] Disk read error!", 13, 10, 0

align 4
dap_stage2:
    db 0x10       ; packet size
    db 0          ; reserved
    dw 31         ; sectors to read
    dw 0x0000     ; buffer offset
    dw 0x1000     ; buffer segment (0x1000:0x0000 = 0x10000)
    dq 1          ; starting LBA

times 510-($-$$) db 0
dw 0xAA55
