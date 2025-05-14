#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name, int dirinum, struct direntv6 *result) {
    struct inode node;
    if (inode_iget(fs, dirinum, &node) < 0) return -1;
    if ((node.i_mode & IFMT) != IFDIR) return -1;

    int32_t total_bytes = (node.i_size0 << 16) | node.i_size1;
    int blocks = (total_bytes + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

    for (int i = 0; i < blocks; i++) {
        uint8_t data[DISKIMG_SECTOR_SIZE];
        int bytes = file_getblock(fs, dirinum, i, data);
        if (bytes < sizeof(struct direntv6)) return -1;

        int count = bytes / sizeof(struct direntv6);
        for (int j = 0; j < count; j++) {
            struct direntv6 *entry = (struct direntv6 *)(data + j * sizeof(struct direntv6));
            size_t len = strlen(name);
            if (len < sizeof(entry->d_name) &&
                strncmp(entry->d_name, name, len) == 0 &&
                entry->d_name[len] == '\0') {
                *result = *entry;
                return 0;
            }
        }
    }

    return -1;
}
