#include "../include/cfs.h"
#include "console.h"

static cfs_header_t *g_cfs_header = 0;
static uint32_t g_cfs_base = 0;
static uint32_t g_cfs_len = 0;
static cfs_file_t g_current_file;

static int str_equal(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (*(const unsigned char *)s1 == *(const unsigned char *)s2);
}

static const char* normalize_path(const char *path) {
    if (path && path[0] == '/') {
        return path + 1;
    }
    return path;
}

int cfs_init(uint32_t cfs_addr, uint32_t cfs_size) {
    if (cfs_addr == 0) {
        return -1;
    }

    cfs_header_t *hdr = (cfs_header_t *)cfs_addr;
    if (hdr->magic != CFS_MAGIC) {
        return -2;
    }

    g_cfs_header = hdr;
    g_cfs_base = cfs_addr;
    g_cfs_len = cfs_size;
    return 0;
}

uint32_t cfs_get_count(void) {
    if (!g_cfs_header) return 0;
    return g_cfs_header->num_files;
}

cfs_entry_t* cfs_get_entry(uint32_t index) {
    if (!g_cfs_header || index >= g_cfs_header->num_files) {
        return 0;
    }
    return &g_cfs_header->entries[index];
}

const uint8_t* cfs_get_file_data(uint32_t offset) {
    if (!g_cfs_base) return 0;
    return (const uint8_t *)(g_cfs_base + offset);
}

cfs_file_t* cfs_open(const char *path) {
    if (!g_cfs_header || !path) {
        return 0;
    }

    const char *norm_path = normalize_path(path);

    for (uint32_t i = 0; i < g_cfs_header->num_files; i++) {
        cfs_entry_t *entry = &g_cfs_header->entries[i];
        const char *entry_norm = normalize_path(entry->name);

        if (str_equal(entry_norm, norm_path)) {
            g_current_file.name = entry->name;
            g_current_file.size = entry->size;
            g_current_file.type = entry->type;
            g_current_file.data = cfs_get_file_data(entry->offset);
            return &g_current_file;
        }
    }
    return 0;
}
