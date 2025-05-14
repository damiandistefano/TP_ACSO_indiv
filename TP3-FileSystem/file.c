#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNo, void *buf) {
    if (fs == NULL || buf == NULL) return -1;

    // Leer el inodo correspondiente
    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;  // error leyendo el inodo
    }

    // Calcular la cantidad total de bloques del archivo
    int file_size = inode_getsize(&in);
    int total_blocks = (file_size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

    if (blockNo < 0 || blockNo >= total_blocks) {
        return -1;  // bloque fuera de rango
    }

    // Obtener el número de bloque físico en disco
    int diskBlockNum = inode_indexlookup(fs, &in, blockNo);
    if (diskBlockNum < 0) {
        return -1;  // error al resolver la dirección del bloque
    }

    // Leer el bloque desde disco
    int bytesRead = diskimg_readsector(fs->dfd, diskBlockNum, buf);
    if (bytesRead < 0) {
        return -1;  // error leyendo del disco
    }

    // Si es el último bloque del archivo, puede que no esté lleno
    int bytes_remaining = file_size - blockNo * DISKIMG_SECTOR_SIZE;
    if (bytes_remaining < DISKIMG_SECTOR_SIZE) {
        return bytes_remaining;
    }

    return DISKIMG_SECTOR_SIZE;
}
