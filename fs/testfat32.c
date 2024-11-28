#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "fat32.h"
#include "bitfile.h"
#define DEBUG
#include "debug.h"

// #include "../drivers/debug.c"

// #define REAL_CARD

void hexdump(uint8_t *buf, int len) {
  for (int i=0; i<len; i+=16) {
    int j;
    for (j=0; j<16 && (i+j)<len; j++) {
      printf("%02X ", buf[i+j]);
    }
    for (; j<16; j++) {
      printf("   ");
    }
    for (j=0; j<16 && (i+j)<len; j++) {
      printf("%c", buf[i+j] >= ' ' && buf[i+j] < 128 ? buf[i+j] : '?');
    }
    printf("\n");
  }
  printf("\n");
}

int failed = 0;

#define passif(a) { int r = (a); printf("[%d]%s " #a "\n", __LINE__, r ? "PASSED" : "FAILED" ); failed += r ? 0 : 1; }
#define passifeq(a,b) { int r, rb = (b); r = (a == rb); printf("[%d]%s " #a " != %d\n", __LINE__, r ? "PASSED" : "FAILED", rb ); failed += r ? 0 : 1; }


#ifndef REAL_CARD
#if defined(FAT16)
#define CARDFILE    "card16"
#elif defined(FAT)
#define CARDFILE    "card.pt"
#else
#define CARDFILE    "card"
#endif
#else
#define CARDFILE    "/dev/sdb"
// #define CARDFILE    "/dev/mmcblk0"
#endif


int read_sector(int sector, uint8_t *buff) {
  FILE *f = fopen(CARDFILE, "rb");
  if (!f) return 1;
  fseeko64(f, (uint64_t)sector*(uint64_t)512, SEEK_SET);
  fread(buff, 1, 512, f);
  fclose(f);
  return 0;
}

#ifndef REAL_CARD
int write_sector(int sector, uint8_t *buff) {
  FILE *f = fopen(CARDFILE, "wb+");
  fseek(f, sector*512, SEEK_SET);
  fwrite(buff, 1, 512, f);
  fclose(f);
  return 0;
}
#endif

//47920
#ifndef REAL_CARD
#define FIRST_LBA 47920
#define FIRST_CLUSTER 45872
#else
#define FIRST_LBA 5536594
#define FIRST_CLUSTER 171764
#endif

int is_in(uint8_t *data, int datalen, uint8_t *match, int matchlen) {
  int pos = 0;
  for (int i=0; i<datalen; i++) {
    if (data[i] == match[pos]) {
      pos ++;
      if (pos == matchlen) {
        return 1;
      }
    } else {
      pos = 0;
    }
  }
  return 0;
}

int closed_at_block_nr = 0;
void test_closing_fn(void *ud) {
  closed_at_block_nr = *(int *)ud;
}

void DoFakeCardTests() {
  fat32_t *fat32;  
  
  uint8_t block[512];
  read_sector(FIRST_LBA, block);
#if !defined(FAT16) && !defined(FAT)
  passif(bitfile_get_length(block, 0) != 0);
#endif
  
  passif(init_fs() == 0);
  fat32 = get_partition(0);
#ifdef FAT
  passif(get_partition(1) != NULL);
  passif(get_partition(2) == NULL);
  passif(get_partition(3) == NULL);
#endif
  
  uint32_t cluster;
  cluster=get_cluster_from_filename(fat32, "/bitfiles/tld_test_placa.zx3.bit", NULL);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  char str[] = "tld_test_placa_a35t";
  passif(is_in(block, 512, (uint8_t *)str, strlen(str)));

  uint32_t filesize = 0;
  cluster=get_cluster_from_filename(fat32, "/bitfiles/squash_top.bit", &filesize);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  char str2[] = "bagman_top";
  passif(is_in(block, 512, (uint8_t *)str2, strlen(str2)));
  passifeq(740238, filesize);
//   hexdump(block, 512);

  cluster=get_cluster_from_filename(fat32, "/bitfiles/asteroids_top.bit", NULL);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  char str3[] = "asteroids_top";
  passif(is_in(block, 512, (uint8_t *)str3, strlen(str3)));

  cluster=get_cluster_from_filename(fat32, "bitfiles/squash_top.bit", NULL);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  char str4[] = "bagman_top";
  passif(is_in(block, 512, (uint8_t *)str4, strlen(str4)));
  
  cluster=get_cluster_from_filename(fat32, "bitfiles/a2600.bit", NULL);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  char str5[] = "a2600";
  passif(is_in(block, 512, (uint8_t *)str5, strlen(str5)));

  memset(block, 0, 512);
  cluster=get_cluster_from_filename(fat32, "bitfiles/A2600   BIT", NULL);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  passif(is_in(block, 512, (uint8_t *)str5, strlen(str5)));
  
  memset(block, 0, 512);
  cluster=get_cluster_from_filename(fat32, "bitfiles/A2600.BIT", NULL);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  passif(is_in(block, 512, (uint8_t *)str5, strlen(str5)));
  //   hexdump(block, 512);
  
  
#ifdef FAT
  uint32_t lba;
  cluster=get_cluster_from_filename(get_partition(1), "bitfiles/top_level_zxtres.bit", NULL);
  passifeq(0, cluster);

  cluster=get_cluster_from_filename(get_partition(0), "bitfiles/robotron_top.bit", NULL);
  passifeq(0, cluster);

  cluster=get_cluster_from_filename(get_partition(0), "bitfiles/top_level_zxtres.bit", NULL);
  passif(cluster != 0);
  lba = get_lba_from_cluster(get_partition(0), cluster);
  passif(get_partition(0) == get_partition_by_lba(lba));

  cluster=get_cluster_from_filename(get_partition(1), "bitfiles/robotron_top.bit", NULL);
  passif(cluster != 0);
  lba = get_lba_from_cluster(get_partition(1), cluster);
  passif(get_partition(1) == get_partition_by_lba(lba));
#endif  
  
  memset(block, 0, 512);
  char str5a[] = "top_level_zxtres";
  cluster=get_cluster_from_filename(fat32, "bitfiles/top_level_zxtres.bit", NULL);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  passif(is_in(block, 512, (uint8_t *)str5a, strlen(str5a)));
  
#ifndef FAT
  char str6[] = "robotron";

  memset(block, 0, 512);
  cluster=get_cluster_from_filename(fat32, "/bitfiles/robotron_top.bit", NULL);
  passif(cluster != 0);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  passif(is_in(block, 512, (uint8_t *)str6, strlen(str6)));

  memset(block, 0, 512);
  cluster=get_cluster_from_filename(fat32, "/bitfiles/ROBOTR~1.BIT", NULL);
  passif(cluster != 0);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  passif(is_in(block, 512, (uint8_t *)str6, strlen(str6)));

  memset(block, 0, 512);
  cluster=get_cluster_from_filename(fat32, "/BITFILES/robotron_top.bit", NULL);
  passif(cluster != 0);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  passif(is_in(block, 512, (uint8_t *)str6, strlen(str6)));
  
  memset(block, 0, 512);
  cluster=get_cluster_from_filename(fat32, "/BITFILES/ROBOTR~1.BIT", NULL);
  passif(cluster != 0);
  read_sector(get_lba_from_cluster(fat32, cluster), block);
  passif(is_in(block, 512, (uint8_t *)str6, strlen(str6)));

  read_lba_t rl;
  get_next_lba_init(fat32, &rl, get_lba_from_cluster(fat32, cluster), 0);

  int nr_blocks = 0;
  
  while (get_next_lba(&rl, block)) {
#if 0
    {
      FILE *f = fopen("data.bin", "ab");
      if (f) {
        fwrite(block, 1, 512, f);
        fclose(f);
      }
    }
#endif
    nr_blocks ++;
  }
  int should_be = ((1623 + fat32->sectorsPerCluster - 1) / fat32->sectorsPerCluster) * fat32->sectorsPerCluster;
  printf("Should be %d\n", should_be);
  passifeq(should_be, nr_blocks);

  // test buffered version
  buffered_read_lba_t brl;
  closed_at_block_nr = 0;
  get_next_lba_init_buffered(fat32, &brl, get_lba_from_cluster(fat32, cluster), 0);
  brl.closing_fn = test_closing_fn;
  brl.closing_user = &nr_blocks;
  nr_blocks = 0;
  
  while (get_next_lba_buffered(&brl, block)) {
    nr_blocks ++;
  }
  printf("nr_blocks = %d\n", nr_blocks);
  passifeq(should_be, nr_blocks);
  passifeq((should_be - BUFFER_SIZE), closed_at_block_nr);

  // test reduced block read
  get_next_lba_init(fat32, &rl, get_lba_from_cluster(fat32, cluster), 1200);
  nr_blocks = 0;
  while (get_next_lba(&rl, block)) {
    nr_blocks ++;
  }
  passifeq(1200, nr_blocks);
  

#endif
  
#ifdef FAT
  for (int q=0; q<2; q++) {
    fat32_t *fat32 = get_partition(q);
#endif  
  uint32_t testcluster0 = get_cluster_from_lba(fat32, fat32->lbaOffset + 65536);
  uint32_t testlba = get_lba_from_cluster(fat32, testcluster0);
  uint32_t testcluster = get_cluster_from_lba(fat32, testlba);
  printf("from cluster %08X: testlba = %08X testcluster = %08X\n", testcluster0, testlba, testcluster);
  passifeq(testcluster0, testcluster);
#ifdef FAT
  }
#endif
  
}

void ExploreRealCard() {
  fat32_t *fat32;  

  passif(init_fs() == 0);
  
  fat32 = get_partition(0);
  
  uint8_t block[512];
  read_sector(0, block);
  hexdump(block, 512);

  
//   uint32_t cluster=get_cluster_from_filename(fat32, "/xxxx", NULL);
  
#if 1
  uint32_t cluster=get_cluster_from_filename(fat32, "/RP2MAPP.BIN", NULL);
  
  read_lba_t rl;
  get_next_lba_init(fat32, &rl, get_lba_from_cluster(fat32, cluster), 0);
  int nr_blocks = 0;
  
  while (get_next_lba(&rl, block)) {
    nr_blocks ++;
  }
  printf("nr_blocks = %d\n", nr_blocks);
#endif
  
  
#if 0
  // get_lba_from_cluster: cluster=0002A819
// get_cluster_from_lba: lba=00559FF2 returns cluster 0002A719
  uint32_t testlba = get_lba_from_cluster(fat32, 0x0002A819);
  uint32_t testcluster = get_cluster_from_lba(fat32, testlba);
  printf("from cluster 0x0002A819: testlba = %08X testcluster = %08X\n", testlba, testcluster);
#endif
}

int main(void) {
#ifndef REAL_CARD
  DoFakeCardTests();
#else
  ExploreRealCard();
#endif
  return failed;
}
