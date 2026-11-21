#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../include/cfs.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <output.img> <file1:type> [file2:type ...]\n", argv[0]);
        fprintf(stderr, "Types: 1=file, 2=exec, 3=driver, 4=config, 5=lib\n");
        return 1;
    }

    const char *out_path = argv[1];
    int num_files = argc - 2;
    if (num_files > CFS_MAX_FILES) {
        fprintf(stderr, "Error: Too many files (max %d)\n", CFS_MAX_FILES);
        return 1;
    }

    cfs_header_t header;
    memset(&header, 0, sizeof(header));
    header.magic = CFS_MAGIC;
    header.num_files = num_files;

    uint32_t current_offset = sizeof(cfs_header_t);

    FILE *out = fopen(out_path, "wb");
    if (!out) {
        perror("fopen output");
        return 1;
    }

    // Write empty header placeholder first
    fwrite(&header, 1, sizeof(header), out);

    for (int i = 0; i < num_files; i++) {
        char spec[256];
        strncpy(spec, argv[i + 2], sizeof(spec) - 1);
        spec[sizeof(spec) - 1] = '\0';

        char *colon = strchr(spec, ':');
        int type = CFS_TYPE_FILE;
        if (colon) {
            *colon = '\0';
            type = atoi(colon + 1);
        }

        const char *filepath = spec;
        // Derive clean archive path, stripping leading dirs if needed or keep relative
        const char *archive_name = filepath;
        if (strncmp(archive_name, "cfs_root/", 9) == 0) {
            archive_name += 9;
        }

        FILE *in = fopen(filepath, "rb");
        if (!in) {
            fprintf(stderr, "Error opening input file: %s\n", filepath);
            fclose(out);
            return 1;
        }

        fseek(in, 0, SEEK_END);
        long file_size = ftell(in);
        fseek(in, 0, SEEK_SET);

        uint8_t *buf = malloc(file_size);
        if (file_size > 0 && fread(buf, 1, file_size, in) != (size_t)file_size) {
            fprintf(stderr, "Error reading file: %s\n", filepath);
            fclose(in);
            fclose(out);
            return 1;
        }
        fclose(in);

        strncpy(header.entries[i].name, archive_name, CFS_MAX_NAME - 1);
        header.entries[i].offset = current_offset;
        header.entries[i].size = file_size;
        header.entries[i].type = type;
        header.entries[i].checksum = 0;
        for (long b = 0; b < file_size; b++) {
            header.entries[i].checksum += buf[b];
        }

        fwrite(buf, 1, file_size, out);
        current_offset += file_size;
        free(buf);
    }

    header.total_size = current_offset;

    // Pad to 512-byte sector boundary (at least 64KB = 128 sectors)
    uint32_t target_size = 65536;
    if (header.total_size < target_size) {
        uint32_t pad_len = target_size - header.total_size;
        uint8_t zero = 0;
        for (uint32_t p = 0; p < pad_len; p++) {
            fwrite(&zero, 1, 1, out);
        }
        header.total_size = target_size;
    }

    // Rewind and write real header
    fseek(out, 0, SEEK_SET);
    fwrite(&header, 1, sizeof(header), out);
    fclose(out);

    printf("CFS archive created: %s (%d files, %u bytes)\n", out_path, num_files, header.total_size);
    return 0;
}
