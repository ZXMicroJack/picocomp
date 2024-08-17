/* This file is part of fpga-spec by ZXMicroJack - see LICENSE.txt for moreinfo */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

struct fn {
  char fn[23];
  char type;
  unsigned pos:32; // 24 in blocks
  unsigned size:32; // 28 in bytes
};

uint8_t chksum(uint8_t start, uint8_t *data, int len) {
  for (int i=0; i<len; i++) {
    start ^= data[i];
  }
  return start;
}

int main(int argc, char **argv) {
  FILE *fndx, *fbin;
  unsigned long pos = 0;
  int nr_entries = 0;

  fndx = fopen("index.xxx", "wb");
  fbin = fopen("data.xxx", "wb");

  int nr_spare_entries = atoi(argv[1]);

  printf("Info: adding %d spare entries + %d real ones\n", nr_spare_entries, argc - 2);

  int block_offset = ((argc - 2 + nr_spare_entries + 1) + 15) / 16;

  printf("Info: initial block_offset is %d\n", block_offset);

  for (int i=2; i<argc; i++) {

    FILE *f = fopen(argv[i], "rb");
    if (f) {
      struct fn fq;
      fq.pos = pos + block_offset;
      printf("Info: adding %s at block_offset %d ", argv[i], fq.pos);

      fseek(f, 0, SEEK_END);
      unsigned long data = ftell(f);
      fseek(f, 0, SEEK_SET);

      uint8_t csum = 0xff;
      while (!feof(f)) {
        char buf[512];
        memset(buf, 0xff, sizeof buf);
        int l = fread(buf, 1, sizeof buf, f);
        csum = chksum(csum, buf, 512);
        fwrite(buf, 1, sizeof buf, fbin);
        pos ++;
      }
      fclose(f);

      printf("chksum %02X\n", csum);

      // round up to nearest block - not required
      // fq.size = (data+511) & 0xfffffe00;
      fq.size = data;

      memset(fq.fn, ' ', sizeof fq.fn);
      int j;
      for (j = 0; j<strlen(argv[i]) && j < sizeof fq.fn; j++) {
        if (argv[i][j] == '.' || argv[i][j] == '\0')
          break;
        fq.fn[j] = argv[i][j];
      }

      fq.type = argv[i][j] == '.' ? tolower(argv[i][j+1]) : '?';
      fwrite(&fq, 1, sizeof fq, fndx);
      printf("Info: Checksum of dir entry for %s is %02X\n", argv[i], chksum(0xff, (uint8_t *)&fq, sizeof fq));
      nr_entries ++;
    } else nr_spare_entries ++;
  }

  for (int i=0; i<nr_spare_entries; i++) {
    struct fn fq;
    memset(&fq, 0xff, sizeof fq);
    fwrite(&fq, 1, sizeof fq, fndx);
  }

  int pos_in_block = (nr_entries + 1 + nr_spare_entries) & 15;

  if (pos_in_block > 0) {
    for (int i=pos_in_block; i<16; i++) {
      struct fn fq;
      memset(&fq, 0xff, sizeof fq);
      fwrite(&fq, 1, sizeof fq, fndx);
    }
  }

  // write terminator
  struct fn fq;
  memset(&fq, 0xfe, sizeof fq);
  fwrite(&fq, 1, sizeof fq, fndx);

  fclose(fndx);

  // const char catalog[] = "CATALOG";
  // memset(fq.fn, ' ', sizeof fq.fn);
  // memcpy(fq.fn, catalog, strlen(catalog));
  // fq.type = 'X';
  // fq.pos = 0;
  // fq.size = ((nr_entries + nr_spare_entries + 2) + 15) / 16;
  //
  // fndx = fopen("catalog.xxx", "wb");
  // fwrite(&fq, 1, sizeof fq, fndx);
  // fclose(fndx);

  fclose(fbin);
  return 0;
}
