#ifndef KERNEL_KBD_H
#define KERNEL_KBD_H

#include <stdint.h>

void kbd_init(void);
int kbd_haschar(void);
char kbd_getchar(void);

#endif /* KERNEL_KBD_H */
