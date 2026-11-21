#ifndef INSANE_API_H
#define INSANE_API_H

#include <stdint.h>

typedef void (*cfs_list_cb_t)(const char *name, uint32_t size, uint32_t type);
typedef void (*driver_list_cb_t)(const char *name, const char *ver);

typedef struct {
    void (*print)(const char *str);
    void (*putc)(char c);
    char (*getchar)(void);
    int  (*haschar)(void);
    void (*clear_screen)(void);
    int  (*cfs_list)(cfs_list_cb_t cb);
    int  (*cfs_read)(const char *path, char *buf, uint32_t max_size);
    void (*get_mem_info)(uint32_t *free_mib, uint32_t *reserved_mib);
    void (*get_drivers)(driver_list_cb_t cb);
    void (*reboot)(void);
    void (*halt)(void);
} insane_api_t;

#endif /* INSANE_API_H */
