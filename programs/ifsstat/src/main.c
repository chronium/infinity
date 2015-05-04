#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>
#include "ifs.h"

static void print_vol_header(int fd);
static void stat_blocks(struct ifs_volume_hdr *hdr, int fd, int *freeblocks, int *usedblocks, int *neblocks);
int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    
    if(argc != 2) {
        printf("ifsstat: invalid argument(s)!\n");
        sys_exit(-1);
    }
    int f = open(argv[1], O_RDONLY);
    print_vol_header(f);
    close(f);
    sys_exit(0);
}


static void print_vol_header(int fd)
{
    struct ifs_volume_hdr header;
    int free_blocks = 0;
    int used_blocks = 0;
    
    read(fd, &header, sizeof(struct ifs_volume_hdr));
    stat_blocks(&header, fd, &free_blocks, &used_blocks, &used_blocks);
    printf("mag0:            %02x\n", header.mag0);
    printf("mag1:            %02x\n", header.mag1);
    printf("mag2:            %02x\n", header.mag2);
    printf("mag3:            %02x\n", header.mag3);
    printf("volume name:     %s\n", header.volume_name);
    printf("osid:            %04x\n", header.id);
    printf("block size:      %d\n", header.file_block_size);
    printf("block pool size: %d\n", header.block_pool_size);
    printf("used blocks:     %d\n", used_blocks);
    printf("free blocks:     %d\n", free_blocks);
}


static void stat_blocks(struct ifs_volume_hdr *hdr, int fd, int *freeblocks, int *usedblocks, int *neblocks)
{
    for(int i = 0; i < hdr->block_pool_size; i += sizeof(struct ifs_block)) {
        struct ifs_block block;
        read(fd, &block, sizeof(struct ifs_block));
        if(block.state == IFS_BLOCK_ALLOCATED) *usedblocks = *usedblocks + 1;
        if(block.state == IFS_BLOCK_FREE) *freeblocks = *freeblocks + 1;
        if(block.state == IFS_BLOCK_NONEXISTENT) *neblocks = *neblocks + 1;
    }
}
