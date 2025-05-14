#include <stdlib.h>
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int pathname_lookup(struct unixfilesystem *fs, const char *path) {
    if (path[0] != '/') return -1;
    if (path[1] == '\0') return ROOT_INUMBER;

    int current = ROOT_INUMBER;
    char *dup = strdup(path + 1);
    if (!dup) return -1;

    char *segment = strtok(dup, "/");
    while (segment) {
        struct direntv6 found;
        if (directory_findname(fs, segment, current, &found) < 0) {
            free(dup);
            return -1;
        }
        current = found.d_inumber;
        segment = strtok(NULL, "/");
    }

    free(dup);
    return current;
}
