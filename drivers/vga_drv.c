#include "../include/driver.h"

int driver_init(void);

__attribute__((section(".header"), used))
const driver_header_t header = {
    .magic = DRIVER_MAGIC,
    .name = "VGA Text Display",
    .version = "0.1",
    .type = DRIVER_TYPE_CONSOLE,
    .init_offset = sizeof(driver_header_t),
    .reserved = 0
};

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

int driver_init(void) {
    // Enable cursor scanline 14 to 15
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x0E);
    outb(0x3D4, 0x0B);
    outb(0x3D5, 0x0F);
    return 0; // Success
}
