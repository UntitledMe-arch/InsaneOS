#include "../include/driver.h"
#include "../include/cfs.h"
#include "console.h"

#define MAX_LOADED_DRIVERS 16

static driver_header_t g_loaded_drivers[MAX_LOADED_DRIVERS];
static uint32_t g_loaded_count = 0;

int driver_subsystem_init(void) {
    g_loaded_count = 0;
    return 0;
}

uint32_t driver_get_loaded_count(void) {
    return g_loaded_count;
}

const driver_header_t* driver_get_by_index(uint32_t index) {
    if (index >= g_loaded_count) return 0;
    return &g_loaded_drivers[index];
}

int driver_load_from_cfs(void) {
    uint32_t total_files = cfs_get_count();
    g_loaded_count = 0;

    for (uint32_t i = 0; i < total_files; i++) {
        cfs_entry_t *entry = cfs_get_entry(i);
        if (!entry) continue;

        if (entry->type == CFS_TYPE_DRIVER || 
            (entry->name[0] == 'd' && entry->name[1] == 'r' && 
             entry->name[2] == 'i' && entry->name[3] == 'v' && 
             entry->name[4] == 'e' && entry->name[5] == 'r')) {
            
            cfs_file_t *file = cfs_open(entry->name);
            if (!file || file->size < sizeof(driver_header_t)) {
                continue;
            }

            const driver_header_t *hdr = (const driver_header_t *)file->data;
            if (hdr->magic != DRIVER_MAGIC) {
                continue;
            }

            // Call driver initialization routine
            int (*init_func)(void) = (int (*)(void))((const char *)file->data + hdr->init_offset);
            int res = init_func();
            if (res == 0) {
                if (g_loaded_count < MAX_LOADED_DRIVERS) {
                    g_loaded_drivers[g_loaded_count] = *hdr;
                    g_loaded_count++;
                }
            }
        }
    }

    return (int)g_loaded_count;
}
