#include "kbd.h"
#include "io.h"

#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64
#define COM1_PORT       0x3F8

static int shift_active = 0;
static int caps_lock = 0;

static const char kbd_scancode_lower[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* Ctrl */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, /* Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, /* Right Shift */
    '*',
    0, /* Alt */
    ' ', /* Space */
    0, /* Caps lock */
};

static const char kbd_scancode_upper[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, /* Ctrl */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, /* Left Shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0, /* Right Shift */
    '*',
    0, /* Alt */
    ' ', /* Space */
    0, /* Caps lock */
};

void kbd_init(void) {
    // Flush any leftover bytes in 8042 buffer
    while (inb(KBD_STATUS_PORT) & 1) {
        inb(KBD_DATA_PORT);
    }
}

int kbd_haschar(void) {
    // Check PS/2 controller
    if (inb(KBD_STATUS_PORT) & 1) {
        return 1;
    }
    // Check COM1 serial port (Data Ready = bit 0 of Line Status Register)
    if (inb(COM1_PORT + 5) & 1) {
        return 1;
    }
    return 0;
}

char kbd_getchar(void) {
    while (1) {
        // First check serial COM1
        if (inb(COM1_PORT + 5) & 1) {
            char c = (char)inb(COM1_PORT);
            if (c == '\r') {
                return '\n';
            }
            if (c == 127) { // DEL -> backspace
                return '\b';
            }
            return c;
        }

        // Then check PS/2 keyboard
        if (inb(KBD_STATUS_PORT) & 1) {
            uint8_t scancode = inb(KBD_DATA_PORT);

            if (scancode == 0x2A || scancode == 0x36) {
                shift_active = 1;
                continue;
            }
            if (scancode == 0xAA || scancode == 0xB6) {
                shift_active = 0;
                continue;
            }
            if (scancode == 0x3A) {
                caps_lock = !caps_lock;
                continue;
            }

            // Key release (scancode & 0x80)
            if (scancode & 0x80) {
                continue;
            }

            if (scancode < 128) {
                char c = 0;
                if (shift_active) {
                    c = kbd_scancode_upper[scancode];
                } else {
                    c = kbd_scancode_lower[scancode];
                    if (caps_lock && c >= 'a' && c <= 'z') {
                        c -= 32;
                    }
                }
                if (c != 0) {
                    return c;
                }
            }
        }
    }
}
