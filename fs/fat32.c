#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "fat32.h"
#define DEBUG
#include "debug.h"


//TODO support partitions - DONE
//TODO support fat16 - DONE
//TODO support different disk reads

#ifdef DEBUG
void dump_sector(uint8_t *buff) {
  int i, j;
  for (i=0; i<32; i++) {
    printf("%04X : ", i*16);
    for (j=0; j<16; j++) {
      printf("%02X ", buff[i*16+j]);
    }
    printf(" : ");
    for (j=0; j<16; j++) {
      printf("%c", buff[i*16+j] < ' ' ? '.' : buff[i*16+j]);
    }
    printf("\n");
  }
}

void dump_fat32(fat32_t *pfat32) {
  printf("sectorsPerCluster %u\n", pfat32->sectorsPerCluster);
  printf("reservedSectorCount %u\n", pfat32->reservedSectorCount);
  printf("sectorsPerFAT %u\n", pfat32->sectorsPerFAT);
  printf("rootDirectoryFirstCluster %u\n", pfat32->rootDirectoryFirstCluster);
}
#else
#define dump_sector(a) {}
#endif

////////////////////////////////////////////////////////////////////////////
// main part of code
uint32_t get_next_cluster(fat32_t *pfat32, int cluster) {
  debug(("get_next_cluster: pfat32->fat.offset %08X cluster %08X\n", pfat32->fat.offset, cluster));
  if (pfat32->fat.offset == (uint32_t)-1 || cluster < pfat32->fat.offset || cluster > pfat32->fat.offset+127) {
    uint32_t lba = cluster / 128 + pfat32->reservedSectorCount + pfat32->lbaOffset;
    if (read_sector(lba, (uint8_t *)pfat32->fat.rec.r32)) {
      pfat32->fat.offset = 0xffffff80;
      debug(("get_next_cluster: Failed to read cluster %08X at lba %08X\n", cluster, lba));
      return 0;
    }
    pfat32->fat.offset = cluster & 0xffffff80;

#ifdef DEBUG
    printf("Fat offset: %d\n", pfat32->fat.offset);
    for (int i=0; i<128; i++) {
      printf("[%08X] ", pfat32->fat.rec.r32[i] );
      if ((i & 0xf) == 0xf) printf("\n");
    }
//     dump_sector((uint8_t *)pfat32->fat.rec.r32);
#endif
  }
  return pfat32->fat.rec.r32[cluster & 0x7f];
}

uint32_t get_next_cluster16(fat32_t *pfat32, int cluster) {
  debug(("get_next_cluster: pfat32->fat.offset %08X cluster %08X\n", pfat32->fat.offset, cluster));
  if (pfat32->fat.offset == (uint32_t)-1 || cluster < pfat32->fat.offset || cluster > pfat32->fat.offset+255) {
    uint32_t lba = cluster / 256 + pfat32->reservedSectorCount + pfat32->lbaOffset;
    if (read_sector(lba, (uint8_t *)pfat32->fat.rec.r32)) {
      pfat32->fat.offset = 0xffffff80;
      debug(("get_next_cluster: Failed to read cluster %08X at lba %08X\n", cluster, lba));
      return 0;
    }
    pfat32->fat.offset = cluster & 0xff00;

#ifdef DEBUG
    printf("Fat offset: %d\n", pfat32->fat.offset);
    for (int i=0; i<256; i++) {
      printf("[%04X] ", pfat32->fat.rec.r16[i] );
      if ((i & 0xf) == 0xf) printf("\n");
    }

//     dump_sector((uint8_t *)pfat32->fat.rec.r32);
#endif
  }
  return pfat32->fat.rec.r16[cluster & 0xff];
}

uint32_t get_lba_from_cluster(fat32_t *pfat32, uint32_t cluster) {
  debug(("get_lba_from_cluster: cluster=%08X\n", cluster));
  return pfat32->dataStart + pfat32->sectorsPerCluster * (cluster - 2);
}


static char *match = NULL;
static char matchlen = 0;
static char matchDOS[12] = "";
static uint32_t match_cluster = 0;
static uint32_t match_length = 0;

static char lfn[256];

static int process_dir_sector(fat32_t *pfat32, uint8_t *buff) {
  for (int i=0; i<512; i+=32) {
    if (buff[i] == 0x00) break;
    if (buff[i] == 0xe5) continue;

    if (buff[i+11] != 0x0f && !(buff[i+11] & 0x08)) { /* not lfn or volume label */
      char name[12];
      memcpy(name, &buff[i], 11);
      name[11] = '\0';
      debug(("name %s lfn %s match %s matchDOS %s\n", name, lfn, match, matchDOS));
      
      if (!strcmp(match, lfn) || !strcmp(match, name) || (matchDOS[0] && !strcmp(matchDOS, name))) {
        uint16_t cluster_hi = buff[i+20] | (buff[i+21]<<8);
        uint16_t cluster_lo = buff[i+26] | (buff[i+27]<<8);
        uint32_t file_size = buff[i+28] | (buff[i+29]<<8) | (buff[i+30]<<16) | (buff[i+31]<<24);
        match_cluster = (cluster_hi << 16) | cluster_lo;
        match_length = file_size;
        return 1;
      }
      
      memset(lfn, 0, sizeof lfn);
    } else if (buff[i+11] == 0x0f) {
      // lfn
      int off = ((buff[i] & 0x1f) - 1) * 13;
      uint8_t lut[] = {1,3,5,7,9,14,16,18,20,22,24,28,30};
      for (int q = 0; q < sizeof lut; q++) {
        lfn[off + q] = buff[i + lut[q]];
      }
    }
  }
  return 0;
}

uint8_t is_fat32(fat32_t *pfat32, uint32_t lba, uint8_t buff[512]) {  
  if (!read_sector(lba, buff) && buff[0x1fe] == 0x55 && buff[0x1ff] == 0xaa) {
#ifdef DEBUG
    dump_sector(buff);
#endif
    if (!memcmp(&buff[0x52], "FAT32", 5)) {
      pfat32->sectorsPerCluster = buff[0x0d];
      pfat32->reservedSectorCount = buff[0x0e] | (buff[0x0f]<<8);
      pfat32->sectorsPerFAT = buff[0x24] | (buff[0x25]<<8) | (buff[0x26]<<16) | (buff[0x27]<<24);
      pfat32->rootDirectoryFirstCluster = buff[0x2c] | (buff[0x2d]<<8) | (buff[0x2e]<<16) | (buff[0x2f]<<24);
      pfat32->rootDirectoryFirstSector = 0; // not used
      pfat32->lbaOffset = lba;
      pfat32->fat32 = 1;
      pfat32->nrFat = buff[0x10];
      pfat32->dataStart = pfat32->reservedSectorCount + pfat32->nrFat * pfat32->sectorsPerFAT + pfat32->lbaOffset;
      
      debug(("nr copies of fat %d\n", buff[0x10]));
      return true;
    } else if (!memcmp(&buff[0x36], "FAT16", 5)) {
      pfat32->sectorsPerCluster = buff[0x0d];
      pfat32->reservedSectorCount = buff[0x0e] | (buff[0x0f]<<8);
      pfat32->sectorsPerFAT = buff[22] + (buff[23] << 8);
      pfat32->rootDirectoryEntries = buff[17] + (buff[18] << 8);
      pfat32->rootDirectoryFirstCluster = 0; // not used
      pfat32->lbaOffset = lba;
      pfat32->fat32 = 0;
      pfat32->nrFat = buff[0x10];
      
      pfat32->rootDirectoryFirstSector = pfat32->reservedSectorCount + pfat32->nrFat * pfat32->sectorsPerFAT + pfat32->lbaOffset;
      pfat32->rootDirectorySectors = ((pfat32->rootDirectoryEntries << 5) + 511) >> 9;
      pfat32->dataStart = pfat32->rootDirectoryFirstSector + pfat32->rootDirectorySectors;
      
      debug(("nr copies of fat %d\n", buff[0x10]));
      return true;
    }
  }
  return false;
}

struct {
  uint32_t lbaStart;
  uint32_t lbaEnd;
  fat32_t fat;
} partition_table[5];
static uint8_t nrPartitions = 0;

fat32_t *get_partition(uint8_t n) {
  return (n >= nrPartitions) ? NULL : &partition_table[n].fat;
}

fat32_t *get_partition_by_lba(uint32_t lba) {
  for (int i=0; i<nrPartitions; i++) {
    if (lba >= partition_table[i].lbaStart && lba <= partition_table[i].lbaEnd) {
      return &partition_table[i].fat;
    }
  }
  return NULL;
}

// find first file system
int init_fs() {
  uint8_t buff[512];
  
  if (nrPartitions) {
    debug(("init_fs: already scanned %d partitions found\n", nrPartitions));
  }
  
  memset(&partition_table, 0x00, sizeof partition_table);
  nrPartitions = 0;

  // check to see if fat32 partition starts at lba 0
  if (is_fat32(&partition_table[nrPartitions].fat, 0, buff)) {
    // reset fat cluster
    partition_table[nrPartitions].fat.fat.offset = -1;
    partition_table[nrPartitions].lbaStart = 0;
    partition_table[nrPartitions].lbaEnd = -1;
    nrPartitions++;
    debug(("init_fs: no partition table - whole disk is filesystem\n"));
    return 0;
  }
  
  // is this a partition table?
#ifdef DEBUG
  dump_sector(buff);
  printf("\n");
#endif
  if (buff[0x1fe] == 0x55 && buff[0x1ff] == 0xaa) {
    uint8_t part_tbl[64];
    memcpy(part_tbl, &buff[0x1be], 64);
    uint32_t offset = 0;
    for (int i=0; i<4; i++) {
      uint32_t lba_start = (part_tbl[offset + 11] << 24) | (part_tbl[offset + 10] << 16) | 
          (part_tbl[offset + 9] << 8) | part_tbl[offset + 8];
      uint32_t nr_lbas = (part_tbl[offset + 15] << 24) | (part_tbl[offset + 14] << 16) | 
          (part_tbl[offset + 13] << 8) | part_tbl[offset + 12];
          
      debug(("try lba %ld (0x%08X) - nr lbas %ld (0x%08X)\n", 
             lba_start, lba_start, nr_lbas, nr_lbas));
      if (is_fat32(&partition_table[nrPartitions].fat, lba_start, buff)) {
        // reset fat cluster
        partition_table[nrPartitions].fat.fat.offset = -1;
        partition_table[nrPartitions].lbaStart = lba_start;
        partition_table[nrPartitions].lbaEnd = lba_start + nr_lbas - 1;
        debug(("init_fs: partition %d is a FAT partition #%d\n", i, nrPartitions));
        nrPartitions ++;
      }
#ifdef DEBUG
      dump_sector(buff);
      printf("\n");
#endif
      offset += 16;
    }
  }

  debug(("init_fs: %d partitions found\n", nrPartitions));
  return nrPartitions == 0;
}

void read_file(fat32_t *pfat32, uint32_t cluster, int (*callback)(fat32_t *, uint8_t *)) {
  uint8_t buff[512];
  while (cluster < 0x0ffffff0 && cluster > 0) {
    uint32_t lba = get_lba_from_cluster(pfat32, cluster);
    for (int i=0; i<pfat32->sectorsPerCluster; i++) {
      debug(("read lba %08X (%d/%d)\n", lba+i, i, pfat32->sectorsPerCluster));
      read_sector(lba+i, buff);
      dump_sector(buff);
      if (callback(pfat32, buff)) {
        return;
      }
    }
    cluster = pfat32->fat32 ? get_next_cluster(pfat32, cluster) : get_next_cluster16(pfat32, cluster);
  }
}

void read_disk(fat32_t *pfat32, uint32_t lba, int nr, int (*callback)(fat32_t *, uint8_t *)) {
  uint8_t buff[512];
  for (int i=0; i<nr; i++) {
    debug(("read lba %08X (%d/%d)\n", lba+i, i, nr));
    read_sector(lba+i, buff);
    dump_sector(buff);
    if (callback(pfat32, buff)) {
      return;
    }
  }
}

uint32_t get_cluster_from_lba(fat32_t *pfat32, uint32_t lba) {
  uint32_t cluster = ((lba - pfat32->dataStart) / pfat32->sectorsPerCluster) + 2;
  debug(("get_cluster_from_lba: lba=%08X returns cluster %08X\n", lba, cluster));
  return cluster;
}

uint8_t get_next_lba(void *user_data, uint8_t *block) {
  read_lba_t *rl = (read_lba_t *)user_data;
  
  if (!rl->max_blocks) {
    debug(("get_next_lba: complete\n", rl->lba));
    return false;
  }

  if (read_sector(rl->lba, block)) {
    debug(("get_next_lba: failed to read sector %d\n", rl->lba));
    return false;
  }
  
  debug(("get_next_lba: read lba %d\n", rl->lba));
  rl->lba ++;
  rl->n ++;
  debug(("get_next_lba: rl->n %d\n", rl->n));
  if (rl->n >= rl->pfat32->sectorsPerCluster) {
    debug(("get_next_lba: next next cluster %08X\n", rl->cluster));
    rl->cluster = rl->pfat32->fat32 ? get_next_cluster(rl->pfat32, rl->cluster) : get_next_cluster16(rl->pfat32, rl->cluster);
    if ((rl->pfat32->fat32 && rl->cluster >= 0x0ffffff0) || (!rl->pfat32->fat32 && rl->cluster >= 0xffff) || rl->cluster == 0) {
      debug(("get_next_lba: reached end of cluster %08X\n", rl->cluster));
      rl->max_blocks = 0;
      return true; // this time a sector was returned, next time will fail
    }
    rl->n = 0;
    rl->lba = get_lba_from_cluster(rl->pfat32, rl->cluster);
  }
  rl->max_blocks --;
  return true;
}

void get_next_lba_init(fat32_t *pfat32, read_lba_t *rl, uint32_t lba, uint32_t max_blocks) {
  rl->cluster = get_cluster_from_lba(pfat32, lba);
  rl->lba = lba;
  rl->n = 0;
  rl->pfat32 = pfat32;
  rl->max_blocks = max_blocks ? max_blocks : -1;
}

#define brl_inc(a) (((a) + 1) & BUFFER_SIZE_MASK)

static void get_next_lba_buffered_fill(buffered_read_lba_t *brl) {
  uint8_t result;
  while (brl->c < BUFFER_SIZE) {
     result = get_next_lba(&brl->rl, brl->buff[brl->r]);
     if (result) {
       brl->r = brl_inc(brl->r);
       brl->c ++;
     } else {
       if (brl->closing_fn) {
         brl->closing_fn(brl->closing_user);
         brl->closing_fn = NULL;
       }
       break;
     }
  }
}

uint8_t get_next_lba_buffered(void *user_data, uint8_t *block) {
  uint8_t result = 0;
  
  buffered_read_lba_t *brl = (buffered_read_lba_t *)user_data;
  if (brl->c) {
    memcpy(block, brl->buff[brl->l], 512);
    brl->c --;
    brl->l = brl_inc(brl->l);
    result = 1;
  }
  get_next_lba_buffered_fill(brl);
  return result;
}

void get_next_lba_init_buffered(fat32_t *pfat32, buffered_read_lba_t *brl, uint32_t lba, uint32_t max_blocks) {
  brl->closing_fn = NULL;
  brl->closing_user = NULL;
  
  get_next_lba_init(pfat32, &brl->rl, lba, max_blocks);
  brl->l = brl->r = brl->c = 0;
  get_next_lba_buffered_fill(brl);
}


#define MAX_DEPTH 10

static void filename_normalise(char *out, const char *in) {
	int i=0, j=0;
  
  // is there a dot in the first 8 characters or is base greater than 8 chars
  for (i=0; i<9; i++) {
    if (in[i] == '.') break;
  }
  if (i >= 9 && strlen(in) > 8) {
    out[0] = '\0';
    return;
  }
  
  i = 0;
	while (in[i] && j < 11) {
		if (in[i] == '.') while(j<8) out[j++] = ' ';
		else out[j++] = toupper(in[i]);
		i++;
	}

	while(j<11) out[j++] = ' ';
	out[11] = '\0';
}


typedef void (*fat32_DirCB)(const char *filename, uint32_t filesize, uint32_t cluster);

static fat32_DirCB dircb;

static int fat32_DirProcess(fat32_t *pfat32, uint8_t *buff) {
  for (int i=0; i<512; i+=32) {
    if (buff[i] == 0x00) break;
    if (buff[i] == 0xe5) continue;

    if (buff[i+11] != 0x0f && !(buff[i+11] & 0x08)) { /* not lfn or volume label */
      char name[12];
      uint16_t cluster_hi = buff[i+20] | (buff[i+21]<<8);
      uint16_t cluster_lo = buff[i+26] | (buff[i+27]<<8);
      uint32_t file_size = buff[i+28] | (buff[i+29]<<8) | (buff[i+30]<<16) | (buff[i+31]<<24);

      memcpy(name, &buff[i], 11);
      name[11] = '\0';

      debug(("name %s lfn %s attr %02X\n", name, lfn, buff[i+11]));
      dircb(lfn[0] ? lfn : name, (buff[i+11] & 0x20) ? file_size : -1, (cluster_hi << 16) | cluster_lo);
      
      memset(lfn, 0, sizeof lfn);
    } else if (buff[i+11] == 0x0f) {
      // lfn
      int off = ((buff[i] & 0x1f) - 1) * 13;
      uint8_t lut[] = {1,3,5,7,9,14,16,18,20,22,24,28,30};
      for (int q = 0; q < sizeof lut; q++) {
        lfn[off + q] = buff[i + lut[q]];
      }
    }
  }
  return 0;
}

uint32_t fat32_Dir(fat32_t *pfat32, fat32_DirCB callback) {
  // find first level
  int fat16_root = !pfat32->fat32;
  uint32_t dir_cluster = pfat32->rootDirectoryFirstCluster;

  dircb = callback;
  memset(lfn, 0, sizeof lfn);

  if (fat16_root) {
    read_disk(pfat32, pfat32->rootDirectoryFirstSector, pfat32->rootDirectorySectors, fat32_DirProcess);
    fat16_root = 0;
  } else {
    read_file(pfat32, dir_cluster, fat32_DirProcess);
  }

  return 0;
}

#if 0
int read_sector(int sector, uint8_t *buff) {
  FILE *f = fopen(CARDFILE, "rb");
  if (!f) return 1;
  fseeko64(f, (uint64_t)sector*(uint64_t)512, SEEK_SET);
  fread(buff, 1, 512, f);
  fclose(f);
  return 0;
}
#endif


