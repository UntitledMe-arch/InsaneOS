#include "../include/boot_info.h"
#include "../include/cfs.h"
#include "../include/driver.h"
#include "../include/insane_api.h"
#include "console.h"
#include "memory.h"
#include "kbd.h"
#include "io.h"

static void sys_reboot(void) {
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);
    while (1) {
        __asm__ volatile ("hlt");
    }
}

static void sys_halt(void) {
    outw(0x604, 0x2000);
    while (1) {
        __asm__ volatile ("cli; hlt");
    }
}

static int sys_cfs_list(cfs_list_cb_t cb) {
    if (!cb) return -1;
    uint32_t count = cfs_get_count();
    for (uint32_t i = 0; i < count; i++) {
        cfs_entry_t *e = cfs_get_entry(i);
        if (e) {
            cb(e->name, e->size, e->type);
        }
    }
    return (int)count;
}

static int sys_cfs_read(const char *path, char *buf, uint32_t max_size) {
    if (!path || !buf || max_size == 0) return -1;
    cfs_file_t *f = cfs_open(path);
    if (!f) return -1;
    uint32_t to_copy = f->size;
    if (to_copy >= max_size) {
        to_copy = max_size - 1;
    }
    for (uint32_t i = 0; i < to_copy; i++) {
        buf[i] = (char)f->data[i];
    }
    buf[to_copy] = '\0';
    return (int)to_copy;
}

static void sys_get_mem_info(uint32_t *free_mib, uint32_t *reserved_mib) {
    if (free_mib) *free_mib = memory_get_free_mib();
    if (reserved_mib) *reserved_mib = memory_get_reserved_mib();
}

static void sys_get_drivers(driver_list_cb_t cb) {
    if (!cb) return;
    uint32_t count = driver_get_loaded_count();
    for (uint32_t i = 0; i < count; i++) {
        const driver_header_t *drv = driver_get_by_index(i);
        if (drv) {
            cb(drv->name, drv->version);
        }
    }
}

static insane_api_t g_api = {
    .print = console_puts,
    .putc = console_putc,
    .getchar = kbd_getchar,
    .haschar = kbd_haschar,
    .clear_screen = console_clear,
    .cfs_list = sys_cfs_list,
    .cfs_read = sys_cfs_read,
    .get_mem_info = sys_get_mem_info,
    .get_drivers = sys_get_drivers,
    .reboot = sys_reboot,
    .halt = sys_halt
};

void kmain(boot_info_t *boot) {
    // 1. Initialize console starting at row 2 (after InBoot's message)
    uint32_t cursor_row = (boot && boot->magic == INBOOT_MAGIC) ? boot->cursor_row : 2;
    console_init(cursor_row);

    // Validate InBoot magic
    if (!boot || boot->magic != INBOOT_MAGIC) {
        console_puts("[Kernel] Error: Invalid boot information from InBoot!\n");
        sys_halt();
    }

    // 2. Initialize memory map and print memory statistics
    memory_init(boot);
    memory_print_stats();

    // 3. Initialize CFS filesystem
    if (cfs_init(boot->cfs_addr, boot->cfs_size) != 0) {
        console_puts("[Kernel] Error: Failed to initialize Core File System (CFS)!\n");
        sys_halt();
    }

    // 4. Initialize driver subsystem and load drivers from CFS
    console_puts("[Kernel] Loading drivers from initial CFS...\n");
    driver_subsystem_init();
    kbd_init();

    int loaded_drivers = driver_load_from_cfs();
    kprintf("[Kernel] Loaded %d drivers.\n\n", loaded_drivers);

    // 5. Load the shell from CFS
    console_puts("[Kernel] Loading the shell\n\n");

    cfs_file_t *shell_file = cfs_open("bin/ish");
    if (!shell_file || shell_file->size == 0) {
        console_puts("[Kernel] Error: /bin/ish not found in initial CFS!\n");
        sys_halt();
    }

    // Load iSH shell executable to userspace address (0x400000)
    uint8_t *shell_load_addr = (uint8_t *)0x400000;
    for (uint32_t i = 0; i < 65536; i++) {
        shell_load_addr[i] = 0;
    }
    for (uint32_t i = 0; i < shell_file->size; i++) {
        shell_load_addr[i] = shell_file->data[i];
    }

    // 6. Transfer control to iSH shell
    int (*shell_entry)(insane_api_t *) = (int (*)(insane_api_t *))0x400000;
    shell_entry(&g_api);

    // If shell ever exits, safely halt
    sys_halt();
}
