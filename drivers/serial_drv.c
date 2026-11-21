#include "../include/driver.h"

int driver_init(void);

__attribute__((section(".header"), used))
const driver_header_t header = {
    .magic = DRIVER_MAGIC,
    .name = "16550 UART Serial",
    .version = "0.1",
    .type = DRIVER_TYPE_SERIAL,
    .init_offset = sizeof(driver_header_t),
    .reserved = 0
};

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

int driver_init(void) {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
    return 0; // Success
}
