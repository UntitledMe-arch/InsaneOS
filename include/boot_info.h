#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>

#define INBOOT_MAGIC 0x494E534E /* "INSN" */

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;       /* 1 = Usable, 2 = Reserved, 3 = ACPI, etc. */
    uint32_t acpi_attrs;
} __attribute__((packed)) e820_entry_t;

typedef struct {
    uint32_t magic;          /* INBOOT_MAGIC */
    uint32_t mmap_addr;      /* Physical address of e820 entries (0x8000) */
    uint32_t mmap_count;     /* Number of entries */
    uint32_t cfs_addr;       /* Physical address of CFS image (0x200000) */
    uint32_t cfs_size;       /* CFS image size in bytes */
    uint32_t cursor_row;     /* Cursor row after InBoot messages */
} __attribute__((packed)) boot_info_t;

#endif /* BOOT_INFO_H */
