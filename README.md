# InsaneOS

An x86 operating system built from scratch.

## Features
- Custom bootloader
- Basic kernel setup

## Prerequisites
To build and run InsaneOS, you will need:
- `nasm` (Assembler)
- `gcc` / cross-compiler (`i686-elf-gcc`)
- `make`
- `qemu-system-i386` (for testing)

## Building & Running
```bash
# Build the OS image
make

# Run in QEMU
make run
```

## License
This project is licensed under the GNU General Public License v3.0.
