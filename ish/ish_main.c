#include "../include/insane_api.h"

#define MAX_LINE_LEN 128
#define SHELL_PROMPT "ish-0.1# "

static insane_api_t *sys = (insane_api_t *)0x12345678;

static int str_len(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static int str_cmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static int str_ncmp(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0') break;
    }
    return 0;
}

static void print_uint(uint32_t val) {
    char buf[16];
    int idx = 0;
    if (val == 0) {
        sys->putc('0');
        return;
    }
    while (val > 0) {
        buf[idx++] = '0' + (val % 10);
        val /= 10;
    }
    for (int i = idx - 1; i >= 0; i--) {
        sys->putc(buf[i]);
    }
}

static void cmd_help(void) {
    sys->print("InsaneOS Shell (iSH) v0.1\n");
    sys->print("Available commands:\n");
    sys->print("  help        - Display this help message\n");
    sys->print("  uname       - Show system information\n");
    sys->print("  ls          - List files in initial CFS\n");
    sys->print("  cat <file>  - Display contents of a file from CFS\n");
    sys->print("  mem         - Display system memory statistics\n");
    sys->print("  drivers     - List loaded system drivers\n");
    sys->print("  echo [args] - Print arguments\n");
    sys->print("  clear       - Clear screen\n");
    sys->print("  reboot      - Reboot InsaneOS\n");
    sys->print("  halt        - Halt system\n");
}

static void cmd_uname(void) {
    sys->print("InsaneOS 0.1 i686\n");
}

static void list_cfs_cb(const char *name, uint32_t size, uint32_t type) {
    sys->print("  ");
    sys->print(name);
    // Pad for alignment
    int len = str_len(name);
    for (int i = len; i < 24; i++) {
        sys->putc(' ');
    }
    print_uint(size);
    sys->print(" B");
    for (int i = (size > 9999 ? 5 : (size > 999 ? 4 : (size > 99 ? 3 : 2))); i < 8; i++) {
        sys->putc(' ');
    }
    if (type == 2) {
        sys->print("[EXEC]\n");
    } else if (type == 3) {
        sys->print("[DRIVER]\n");
    } else if (type == 4) {
        sys->print("[CONFIG]\n");
    } else if (type == 5) {
        sys->print("[LIB]\n");
    } else {
        sys->print("[FILE]\n");
    }
}

static void cmd_ls(void) {
    sys->print("Core File System (CFS) contents:\n");
    sys->cfs_list(list_cfs_cb);
}

static void cmd_cat(const char *filename) {
    while (*filename == ' ') filename++;
    if (*filename == '\0') {
        sys->print("Usage: cat <filename>\n");
        return;
    }
    char buf[1024];
    int res = sys->cfs_read(filename, buf, sizeof(buf));
    if (res < 0) {
        sys->print("cat: file not found: ");
        sys->print(filename);
        sys->print("\n");
    } else {
        sys->print(buf);
        if (res > 0 && buf[res - 1] != '\n') {
            sys->putc('\n');
        }
    }
}

static void cmd_mem(void) {
    uint32_t free_mib = 0, reserved_mib = 0;
    sys->get_mem_info(&free_mib, &reserved_mib);
    sys->print("System Memory:\n");
    sys->print("  Reserved: ");
    print_uint(reserved_mib);
    sys->print(" MiB\n");
    sys->print("  Free:     ");
    print_uint(free_mib);
    sys->print(" MiB\n");
    sys->print("  Total:    ");
    print_uint(free_mib + reserved_mib);
    sys->print(" MiB\n");
}

static void list_driver_cb(const char *name, const char *ver) {
    sys->print("  * ");
    sys->print(name);
    sys->print(" (v");
    sys->print(ver);
    sys->print(")\n");
}

static void cmd_drivers(void) {
    sys->print("Loaded Drivers from CFS:\n");
    sys->get_drivers(list_driver_cb);
}

static void cmd_echo(const char *args) {
    while (*args == ' ') args++;
    sys->print(args);
    sys->putc('\n');
}

static void execute_command(char *cmd) {
    while (*cmd == ' ') cmd++;
    if (*cmd == '\0') return;

    if (str_cmp(cmd, "help") == 0) {
        cmd_help();
    } else if (str_cmp(cmd, "uname") == 0) {
        cmd_uname();
    } else if (str_cmp(cmd, "ls") == 0) {
        cmd_ls();
    } else if (str_ncmp(cmd, "cat", 3) == 0 && (cmd[3] == ' ' || cmd[3] == '\0')) {
        cmd_cat(cmd + 3);
    } else if (str_cmp(cmd, "mem") == 0) {
        cmd_mem();
    } else if (str_cmp(cmd, "drivers") == 0) {
        cmd_drivers();
    } else if (str_ncmp(cmd, "echo", 4) == 0 && (cmd[4] == ' ' || cmd[4] == '\0')) {
        cmd_echo(cmd + 4);
    } else if (str_cmp(cmd, "clear") == 0) {
        sys->clear_screen();
    } else if (str_cmp(cmd, "reboot") == 0) {
        sys->print("Rebooting InsaneOS...\n");
        sys->reboot();
    } else if (str_cmp(cmd, "halt") == 0 || str_cmp(cmd, "exit") == 0) {
        sys->print("Halting InsaneOS. System halted.\n");
        sys->halt();
    } else {
        sys->print("ish: command not found: ");
        sys->print(cmd);
        sys->putc('\n');
    }
}

int ish_main(insane_api_t *api) {
    sys = api;

    char line_buf[MAX_LINE_LEN];
    int line_len = 0;

    // Display initial shell prompt
    sys->print(SHELL_PROMPT);

    while (1) {
        char c = sys->getchar();

        if (c == '\n') {
            sys->putc('\n');
            line_buf[line_len] = '\0';
            execute_command(line_buf);
            line_len = 0;
            sys->print(SHELL_PROMPT);
        } else if (c == '\b') {
            if (line_len > 0) {
                line_len--;
                sys->putc('\b');
            }
        } else if (c >= ' ' && c <= '~') {
            if (line_len < MAX_LINE_LEN - 1) {
                line_buf[line_len++] = c;
                sys->putc(c);
            }
        }
    }

    return 0;
}
