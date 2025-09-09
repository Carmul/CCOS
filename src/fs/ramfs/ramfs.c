#include "ramfs.h"
#include "../../stdlib/memutil/memutil.h"
#include "../../stdlib/string/string.h"
#include "../../stdlib/stdio/stdio.h"

// Filesystem state
static file_t files[MAX_FILES];    // File table - all files in root
static fd_t fds[MAX_FDS];          // File descriptor table


void ramfs_init(void) {
    ramdisk_init();
    
    // Clear all file entries
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = false;
        files[i].block = -1;
    }
    
    // Clear all file descriptors
    for (int i = 0; i < MAX_FDS; i++) {
        fds[i].used = false;
    }

    int fd = ramfs_create("welcome");
	const char *text = 
		"****************************************\n"
		"*                                      *\n"
		"*       Welcome to My OS!              *\n"
		"*      Enjoy your stay :)              *\n"
		"*                                      *\n"
		"****************************************\n";
	ramfs_write(fd, text, strlen(text));
	ramfs_close(fd);
}

// Find a file by name, return file index or -1 if not found
static int find_file(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

// Find a free file slot, return index or -1 if none available
static int alloc_file(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) return i;
    }
    return -1;
}

// Find a free file descriptor slot
static int alloc_fd(void) {
    for (int i = 0; i < MAX_FDS; i++) {
        if (!fds[i].used) return i;
    }
    return -1;
}


// Open an existing file
int ramfs_open(const char* filename) {
    int file_id = find_file(filename);
    if (file_id == -1) return -1;  // File not found
    
    int fd = alloc_fd();
    if (fd == -1) return -1;  // No free descriptors
    
    fds[fd].used = true;
    fds[fd].file_id = file_id;
    fds[fd].position = 0;

    // printf("[DEBUG] open(%s) -> fd=%d file_id=%d block=%d\n", filename, fd, file_id, files[file_id].block);

    return fd;
}

// Create a new file and open it for writing
int ramfs_create(const char* filename) {
    if (find_file(filename) != -1) return -1;  // File already exists
    
    int file_id = alloc_file();
    if (file_id == -1) return -1;  // No free file slots
    
    // Initialize the new file
    strcpy(files[file_id].name, filename);
    files[file_id].used = true;
    files[file_id].block = ramdisk_alloc_block();
    
    int fd = alloc_fd();
    if (fd == -1) return -1;  // No free descriptors
    
    fds[fd].used = true;
    fds[fd].file_id = file_id;
    fds[fd].position = 0;
    return fd;
}

// Close a file descriptor
int ramfs_close(int fd) {
    if (fd < 0 || fd >= MAX_FDS || !fds[fd].used) return -1;
    fds[fd].used = false;
    fds[fd].file_id = -1;
    fds[fd].position = 0;
    return 0;
}

// Read data from a file
int ramfs_read(int fd, void* buffer, uint32_t count) {
    if (fd < 0 || fd >= MAX_FDS || !fds[fd].used) return -1;
    
    file_t* file = &files[fds[fd].file_id];
    uint32_t pos = fds[fd].position;
    
    // No data block allocated = empty file
    if (file->block == -1) return 0;
    
    // Get the file's data block
    uint8_t* block_ptr = (uint8_t*)ramdisk_get_block_ptr(file->block);
    if (!block_ptr) return -1;
    
    // Read until null terminator or block boundary
    uint32_t available = 0;
    for (uint32_t i = pos; i < RAMDISK_BLOCK_SIZE && block_ptr[i] != 0; i++) {
        available++;
    }
    
    if (count > available) count = available;
    if (count == 0) return 0;
    
    // Copy data to user buffer
    uint8_t* dest = (uint8_t*)buffer;
    for (uint32_t i = 0; i < count; i++) {
        dest[i] = block_ptr[pos + i];
    }
    
    fds[fd].position = pos + count;

    // printf("[DEBUG] read(%s) -> fd=%d file_id=%d block=%d\n", files[fds[fd].file_id].name, fd, fds[fd].file_id, files[fds[fd].file_id].block);

    return count;
}

// Write data to a file
int ramfs_write(int fd, const void* buffer, uint32_t count) {
    if (fd < 0 || fd >= MAX_FDS || !fds[fd].used) return -1;
    
    file_t* file = &files[fds[fd].file_id];
    uint32_t pos = fds[fd].position;
    
    // Check if write would exceed single block limit
    if (pos + count > RAMDISK_BLOCK_SIZE) {
        count = RAMDISK_BLOCK_SIZE - pos;
        if (count == 0) return 0;
    }
    
    // Write data to the block
    uint8_t* block_ptr = (uint8_t*)ramdisk_get_block_ptr(file->block);
    if (!block_ptr) return -1;
    
    const uint8_t* src = (const uint8_t*)buffer;
    for (uint32_t i = 0; i < count; i++) {
        block_ptr[pos + i] = src[i];
    }
    
    fds[fd].position = pos + count;
    return count;
}

// Seek to a position in a file
int ramfs_seek(int fd, uint32_t offset) {
    if (fd < 0 || fd >= MAX_FDS || !fds[fd].used) return -1;
    if (offset >= RAMDISK_BLOCK_SIZE) return -1;
    
    fds[fd].position = offset;
    return 0;
}

// Delete a file
int ramfs_delete(const char* filename) {
    int file_id = find_file(filename);
    if (file_id == -1) return -1;  // File not found
    
    // Free the data block if allocated
    if (files[file_id].block != -1) {
        ramdisk_free_block(files[file_id].block);
    }
    
    // Mark file slot as free
    files[file_id].used = false;
    files[file_id].block = 0;
    return 0;
}

// ls shell command
void ls(void) {
    for (size_t i = 0; i < MAX_FILES; i++) {
        if(files[i].used == true) {
            printf("    %s" , files[i].name);
            for (uint16_t j = 0; j < 20 - strlen(files[i].name); j++) { putc(' '); }
            printf("4KB (block %d)\n", files[i].block);
        }
    }
}