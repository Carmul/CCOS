#ifndef RAMFS_H
#define RAMFS_H

#include "../ramdisk/ramdisk.h"

#define MAX_FILES 128
#define MAX_FILENAME 32
#define MAX_FDS 64

// File types
#define TYPE_FREE 0
#define TYPE_FILE 1
#define TYPE_DIR  2


// Simple file entry - each file uses exactly one 4KB block
typedef struct {
    char name[MAX_FILENAME];    // Filename
    bool used;                  // Whether this file slot is allocated
    int block;             // Block number for file data (-1 = no block allocated)
} file_t;

// File descriptor for open files
typedef struct {
    uint32_t file_id;           // Index into files[] array
    uint32_t position;          // Current read/write position in bytes
    bool used;                  // Whether this FD slot is allocated
} fd_t;

// Public API
void ramfs_init(void);
int ramfs_open(const char* filename);
int ramfs_create(const char* filename);
int ramfs_close(int fd);
int ramfs_read(int fd, void* buffer, uint32_t count);
int ramfs_write(int fd, const void* buffer, uint32_t count);
int ramfs_seek(int fd, uint32_t offset);
int ramfs_delete(const char* filename);

void ls(void);


#endif