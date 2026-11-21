#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

#include <stdint.h>

void console_init(uint32_t start_row);
void console_putc(char c);
void console_puts(const char *str);
void console_clear(void);
void kprintf(const char *fmt, ...);

#endif /* KERNEL_CONSOLE_H */
