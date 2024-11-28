#ifndef _FAT32_H
#define _FAT32_H

typedef struct {
  uint32_t offset;
  union {
    uint32_t r32[128];
    uint16_t r16[256];
  } rec;
} fat_t;

typedef struct {
    uint8_t sectorsPerCluster;
    uint16_t reservedSectorCount;
    uint32_t sectorsPerFAT;
    uint32_t rootDirectoryFirstCluster;
    uint32_t rootDirectoryFirstSector;
    uint32_t rootDirectorySectors;
    uint32_t rootDirectoryEntries; // fat16 only
    uint32_t lbaOffset;
    uint32_t dataStart;
    uint8_t nrFat;
    uint8_t fat32;
    fat_t fat;
} fat32_t;

// External functionality
int read_sector(int sector, uint8_t *buff);
int write_sector(int sector, uint8_t *buff);

// debug functionality
void dump_sector(uint8_t *buff);
void dump_fat32(fat32_t *pfat32);

// int init_fs(fat32_t *pfat32);
int init_fs();
fat32_t *get_partition(uint8_t n);
fat32_t *get_partition_by_lba(uint32_t lba);
uint32_t get_cluster_from_lba(fat32_t *pfat32, uint32_t lba);
uint32_t get_lba_from_cluster(fat32_t *pfat32, uint32_t cluster);
uint32_t get_next_cluster(fat32_t *pfat32, int cluster);
void read_file(fat32_t *pfat32, uint32_t cluster, int (*callback)(fat32_t *, uint8_t *));
uint8_t get_next_lba(void *user_data, uint8_t *block);

// higher functionality
typedef struct {
  fat32_t *pfat32;
  uint32_t cluster;
  uint32_t lba;
  uint32_t n;
  uint32_t max_blocks;
} read_lba_t;

uint8_t get_next_lba(void *user_data, uint8_t *block);
void get_next_lba_init(fat32_t *pfat32, read_lba_t *rl, uint32_t lba, uint32_t max_blocks);

#define BUFFER_SIZE     16 // PACKETS
#define BUFFER_SIZE_MASK  0xf
#define NR_BLOCKS(a) (((a)+511)>>9)
typedef struct {
  read_lba_t rl;
  uint8_t buff[BUFFER_SIZE][512];
  uint8_t l, r, c;
  void (*closing_fn)(void *);
  void *closing_user;
} buffered_read_lba_t;

uint8_t get_next_lba_buffered(void *user_data, uint8_t *block);
void get_next_lba_init_buffered(fat32_t *pfat32, buffered_read_lba_t *rl, uint32_t lba, uint32_t max_blocks);


uint32_t get_cluster_from_filename(fat32_t *pfat32, const char *filename, uint32_t *filesize);

typedef void (*fat32_DirCB)(const char *filename, uint32_t filesize, uint32_t cluster);
uint32_t fat32_Dir(fat32_t *pfat32, fat32_DirCB callback);



#endif
