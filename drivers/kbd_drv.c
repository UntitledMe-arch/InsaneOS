#include "../include/driver.h"

int driver_init(void);

__attribute__((section(".header"), used))
const driver_header_t header = {
    .magic = DRIVER_MAGIC,
    .name = "PS/2 Keyboard",
    .version = "0.1",
    .type = DRIVER_TYPE_KBD,
    .init_offset = sizeof(driver_header_t),
    .reserved = 0
};

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

int driver_init(void) {
    while (inb(0x64) & 1) {
        inb(0x60);
    }
    outb(0x64, 0xAE);
    return 0; // Success
}
