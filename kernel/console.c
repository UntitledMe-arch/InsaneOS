#include "console.h"
#include "io.h"
#include <stdarg.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_BUFFER 0xB8000
#define COM1_PORT  0x3F8

static uint16_t *vga_mem = (uint16_t *)VGA_BUFFER;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t text_color = 0x07; /* Light grey on black */

static void serial_init(void) {
    outb(COM1_PORT + 1, 0x00); // Disable interrupts
    outb(COM1_PORT + 3, 0x80); // Enable DLAB
    outb(COM1_PORT + 0, 0x03); // 38400 baud
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03); // 8 bits, no parity, 1 stop bit
    outb(COM1_PORT + 2, 0xC7); // Enable FIFO
    outb(COM1_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

static void serial_putc(char c) {
    while ((inb(COM1_PORT + 5) & 0x20) == 0);
    outb(COM1_PORT, c);
}

static void update_hardware_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void console_init(uint32_t start_row) {
    serial_init();
    if (start_row < VGA_HEIGHT) {
        cursor_y = (uint8_t)start_row;
    } else {
        cursor_y = 0;
    }
    cursor_x = 0;
    update_hardware_cursor();
}

static void scroll(void) {
    if (cursor_y >= VGA_HEIGHT) {
        for (int y = 0; y < VGA_HEIGHT - 1; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                vga_mem[y * VGA_WIDTH + x] = vga_mem[(y + 1) * VGA_WIDTH + x];
            }
        }
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_mem[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ((uint16_t)text_color << 8) | ' ';
        }
        cursor_y = VGA_HEIGHT - 1;
    }
}

void console_putc(char c) {
    if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        serial_putc('\r');
        serial_putc('\n');
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_mem[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)text_color << 8) | ' ';
            serial_putc('\b');
            serial_putc(' ');
            serial_putc('\b');
        }
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~(8 - 1);
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    } else {
        vga_mem[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)text_color << 8) | (uint8_t)c;
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
        serial_putc(c);
    }
    scroll();
    update_hardware_cursor();
}

void console_puts(const char *str) {
    while (*str) {
        console_putc(*str++);
    }
}

void console_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_mem[i] = ((uint16_t)text_color << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
    update_hardware_cursor();
}

static void print_uint(uint32_t val, int base) {
    char buf[32];
    int idx = 0;
    if (val == 0) {
        console_putc('0');
        return;
    }
    while (val > 0) {
        int rem = val % base;
        buf[idx++] = (rem < 10) ? ('0' + rem) : ('a' + rem - 10);
        val /= base;
    }
    for (int i = idx - 1; i >= 0; i--) {
        console_putc(buf[i]);
    }
}

static void print_int(int32_t val) {
    if (val < 0) {
        console_putc('-');
        print_uint((uint32_t)(-val), 10);
    } else {
        print_uint((uint32_t)val, 10);
    }
}

void kprintf(const char *fmt, ...) {
    // man idk what this does
    va_list args;
    va_start(args, fmt);
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 's') {
                const char *s = va_arg(args, const char *);
                console_puts(s ? s : "(null)");
            } else if (*fmt == 'd' || *fmt == 'i') {
                print_int(va_arg(args, int32_t));
            } else if (*fmt == 'u') {
                print_uint(va_arg(args, uint32_t), 10);
            } else if (*fmt == 'x' || *fmt == 'X') {
                print_uint(va_arg(args, uint32_t), 16);
            } else if (*fmt == 'c') {
                console_putc((char)va_arg(args, int));
            } else if (*fmt == '%') {
                console_putc('%');
            }
        } else {
            console_putc(*fmt);
        }
        fmt++;
    }
    va_end(args);
}
