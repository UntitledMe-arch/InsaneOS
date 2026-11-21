#ifndef CFS_H
#define CFS_H

#include <stdint.h>

#define CFS_MAGIC 0x43465331 /* "CFS1" */

#define CFS_TYPE_FILE       0x01
#define CFS_TYPE_EXEC       0x02
#define CFS_TYPE_DRIVER     0x03
#define CFS_TYPE_CONFIG     0x04
#define CFS_TYPE_LIB        0x05

#define CFS_MAX_NAME        64
#define CFS_MAX_FILES       32

typedef struct {
    char name[CFS_MAX_NAME];
    uint32_t offset;
    uint32_t size;
    uint32_t type;
    uint32_t checksum;
} __attribute__((packed)) cfs_entry_t;

typedef struct {
    uint32_t magic;
    uint32_t total_size;
    uint32_t num_files;
    uint32_t reserved;
    cfs_entry_t entries[CFS_MAX_FILES];
} __attribute__((packed)) cfs_header_t;

typedef struct {
    const char *name;
    const uint8_t *data;
    uint32_t size;
    uint32_t type;
} cfs_file_t;

int cfs_init(uint32_t cfs_addr, uint32_t cfs_size);
cfs_file_t* cfs_open(const char *path);
uint32_t cfs_get_count(void);
cfs_entry_t* cfs_get_entry(uint32_t index);
const uint8_t* cfs_get_file_data(uint32_t offset);

#endif /* CFS_H */
