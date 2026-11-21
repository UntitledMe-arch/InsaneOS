#include "memory.h"
#include "console.h"

static uint32_t g_free_mib = 0;
static uint32_t g_reserved_mib = 0;
static uint64_t g_total_bytes = 0;
static uint64_t g_free_bytes = 0;
static uint64_t g_reserved_bytes = 0;

void memory_init(boot_info_t *boot) {
    if (!boot || boot->mmap_count == 0) {
        g_free_mib = 127;
        g_reserved_mib = 1;
        return;
    }

    e820_entry_t *entries = (e820_entry_t *)boot->mmap_addr;
    uint64_t usable = 0;
    uint64_t reserved = 0;
    uint64_t highest_ram_addr = 0;

    for (uint32_t i = 0; i < boot->mmap_count; i++) {
        uint64_t base = entries[i].base;
        uint64_t len = entries[i].length;
        uint64_t end = base + len;

        // Ignore entries mapped above 4GB or ROM space at high memory (0xFFFC0000)
        // when tracking standard system physical RAM bounds
        if (base < 0xE0000000ULL) {
            if (end > highest_ram_addr) {
                highest_ram_addr = end;
            }
        }

        if (entries[i].type == 1) {
            usable += len;
        } else {
            reserved += len;
        }
    }

    g_free_bytes = usable;
    g_free_mib = (uint32_t)(usable / (1024 * 1024));

    // Calculate reserved memory:
    // Any address space up to highest RAM address not usable is reserved memory
    if (highest_ram_addr > usable) {
        uint64_t actual_reserved = highest_ram_addr - usable;
        g_reserved_bytes = actual_reserved;
        g_reserved_mib = (uint32_t)((actual_reserved + (1024 * 1024 - 1)) / (1024 * 1024));
    } else {
        g_reserved_bytes = reserved;
        g_reserved_mib = (uint32_t)((reserved + (1024 * 1024 - 1)) / (1024 * 1024));
    }

    if (g_reserved_mib == 0 && g_reserved_bytes > 0) {
        g_reserved_mib = 1;
    }

    g_total_bytes = highest_ram_addr;
}

uint32_t memory_get_free_mib(void) {
    return g_free_mib;
}

uint32_t memory_get_reserved_mib(void) {
    return g_reserved_mib;
}

void memory_print_stats(void) {
    kprintf("[Kernel] Memory Map:\n");
    kprintf("  Reserved: %u MiB\n", g_reserved_mib);
    kprintf("  Free:     %u MiB\n\n", g_free_mib);
}
