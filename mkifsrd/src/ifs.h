#ifndef IFS_H
#define IFS_H

#include <stdint.h>

typedef enum {
	MOUNT_POINT = 0,
	DIRECTORY = 1,
	LINK = 2,
	SOCKET = 3,
	PIPE = 4,
	BLOCK_DEVICE = 6,
	CHAR_DEVICE = 7,
	REG_FILE = 8

} FileType;

typedef enum
{
	BLOCK_FREE = 1,
	BLOCK_ALLOCATED = 2,
	BLOCK_RESERVED = 3,
	BLOCK_NONEXISTENT = 4
}  BlockState;

typedef struct
{
	uint8_t mag0;
	uint8_t mag1;
	uint8_t mag2;
	uint8_t mag3;
	char volume_name[128];
	uint32_t id;
	uint32_t volume_size;
	uint32_t file_block_size;
	uint32_t file_block_bount;
	uint32_t block_pool;
	uint32_t block_pool_size;
	uint32_t root_directory;
	uint32_t file_count;
}  __attribute__((packed))IFSVolume;

typedef struct
{
	char file_name[128];
	int created_time;
	int modified_time;
	uint32_t block_index;
	uint32_t data_index;
	uint32_t file_size;
	FileType file_type;
	uint32_t mode;
	uint32_t UID;
	uint32_t GID;
}  __attribute__((packed))IFSFileEntry;

typedef struct
{
	uint32_t data;
	uint32_t size;
	uint32_t next;
	uint32_t state;
} __attribute__((packed)) IFSBlock;

extern void create_image(void* image, int size);
extern void make_dir(char* dir);
extern void add_file(char* dir, char* contents, int size);
#endif
