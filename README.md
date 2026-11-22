# InsaneOS

A 32-bit x86 operating system built from scratch in C and x86 Assembly.

## Architecture & Features
- **InBoot Bootloader:** Custom 2-stage bootloader (`inboot_stage1` & `inboot_stage2`) loading the kernel and filesystem into memory.
- **32-bit Kernel:** Freestanding i386 kernel with console output, memory management, keyboard handling, and driver loading.
- **Loadable Drivers (`.drv`):** Modular drivers for VGA, PIT Timer, PS/2 Keyboard, and Serial I/O.
- **Core File System (CFS):** Custom read/pack filesystem image generated via the `mkcfs` host tool.
- **iSH Shell:** Custom userspace shell binary (`/bin/ish`) backed by `libcfs`.

## Disk Image Layout (`insaneos.img`)
The build process generates a 10 MiB raw disk image partitioned by LBA sectors:

| LBA Range | Size | Component |
| :--- | :--- | :--- |
| `0` | 1 sector (512 B) | InBoot Stage 1 |
| `1 – 31` | 31 sectors (15.5 KiB) | InBoot Stage 2 |
| `32 – 159` | 128 sectors (64 KiB) | Kernel (`kernel.bin`) |
| `256 – 383` | 128 sectors (64 KiB) | Core File System (`cfs.img`) |

## Prerequisites
To build and run InsaneOS on a Linux host, you will need:
- `gcc` (with 32-bit multilib support, e.g., `gcc-multilib`)
- `ld` (GNU Binutils)
- `nasm` (Netwide Assembler)
- `make` & standard coreutils (`dd`, `truncate`)
- `qemu-system-x86_64` (for virtualization/testing)

## Building & Running

```bash
# Build the disk image (build/insaneos.img)
make

# Build and launch in QEMU (with serial output in terminal)
make qemu

# Build and run in headless mode (serial console only)
make qemu-headless

# Clean all build artifacts and compiled binaries
make clean
```

## License
This project is licensed under the GNU General Public License v3.0 — see the [LICENSE](LICENSE) file for details.
