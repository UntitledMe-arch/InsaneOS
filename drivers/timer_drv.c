#include "../include/driver.h"

int driver_init(void);

__attribute__((section(".header"), used))
const driver_header_t header = {
    .magic = DRIVER_MAGIC,
    .name = "PIT 8254 Timer",
    .version = "0.1",
    .type = DRIVER_TYPE_TIMER,
    .init_offset = sizeof(driver_header_t),
    .reserved = 0
};

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

int driver_init(void) {
    // 100 Hz timer
    outb(0x43, 0x36);
    outb(0x40, 0x9C);
    outb(0x40, 0x2E);
    return 0; // Success
}
