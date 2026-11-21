#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>

#define DRIVER_MAGIC 0x44525631 /* "DRV1" */

#define DRIVER_TYPE_CONSOLE 1
#define DRIVER_TYPE_TIMER   2
#define DRIVER_TYPE_KBD     3
#define DRIVER_TYPE_SERIAL  4

typedef struct {
    uint32_t magic;         /* 4 bytes */
    char name[32];          /* 32 bytes */
    char version[16];       /* 16 bytes */
    uint32_t type;          /* 4 bytes */
    uint32_t init_offset;   /* 4 bytes (always 64 / 0x40) */
    uint32_t reserved;      /* 4 bytes -> total 64 bytes */
} __attribute__((packed)) driver_header_t;

int driver_subsystem_init(void);
int driver_load_from_cfs(void);
uint32_t driver_get_loaded_count(void);
const driver_header_t* driver_get_by_index(uint32_t index);

#endif /* DRIVER_H */
