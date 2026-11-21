#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include "../include/boot_info.h"

void memory_init(boot_info_t *boot);
uint32_t memory_get_free_mib(void);
uint32_t memory_get_reserved_mib(void);
void memory_print_stats(void);

#endif /* KERNEL_MEMORY_H */
