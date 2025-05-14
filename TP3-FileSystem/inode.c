#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"

#define INDIR_ADDR 7


int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    inumber--;
    int inodes_per_sector = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int sector_idx = inumber / inodes_per_sector;
    int inode_idx = inumber % inodes_per_sector;

    int fd = fs->dfd;
    struct inode sector_inodes[inodes_per_sector];
    int result = diskimg_readsector(fd, INODE_START_SECTOR + sector_idx, sector_inodes);
    if (result < 0) {
        return -1;
    }

    *inp = sector_inodes[inode_idx];
    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    int fd = fs->dfd;
    int is_small_file = (inp->i_mode & ILARG) == 0;

    if (is_small_file) {
        return inp->i_addr[blockNum];
    }

    int block_count_per_sector = DISKIMG_SECTOR_SIZE / sizeof(uint16_t);
    int indirect_limit = block_count_per_sector * INDIR_ADDR;

    if (blockNum < indirect_limit) {
        int sector_idx = blockNum / block_count_per_sector;
        int addr_idx = blockNum % block_count_per_sector;

        uint16_t addr_array[block_count_per_sector];
        int result = diskimg_readsector(fd, inp->i_addr[sector_idx], addr_array);
        if (result < 0) {
            return -1;
        }

        return addr_array[addr_idx];
    } else {
        int blockNum_in_double = blockNum - indirect_limit;
        int first_level_idx = INDIR_ADDR;
        int second_level_sector_idx = blockNum_in_double / block_count_per_sector;
        uint16_t first_level_addrs[block_count_per_sector];

        int result = diskimg_readsector(fd, inp->i_addr[first_level_idx], first_level_addrs);
        if (result < 0) {
            return -1;
        }

        int second_level_sector = first_level_addrs[second_level_sector_idx];
        int second_level_addr_idx = blockNum_in_double % block_count_per_sector;

        uint16_t second_level_addrs[block_count_per_sector];
        result = diskimg_readsector(fd, second_level_sector, second_level_addrs);
        if (result < 0) {
            return -1;
        }

        return second_level_addrs[second_level_addr_idx];
    }
}


int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
