#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pc.h"

uint8_t *readIn(const char *file, uint8_t *buffer, int max, unsigned long *actual) {
  FILE *f = fopen(file, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    unsigned long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    int toRead = max > size ? size : max;

    if (buffer == NULL) {
      buffer = (uint8_t *)malloc(toRead);
    }
    int l = fread(buffer, 1, toRead, f);
    fclose(f);
    printf("Read %d bytes from %s\n", l, file);

    if (actual != NULL) *actual = l;

    return buffer;
  } else {
    printf("Couldn't open file %s\n", file);
  }
  return NULL;
}

void writeOut(const char *filename, void *data, int len) {
  FILE *f = fopen(filename, "wb");
  if (f) {
    fwrite(data, 1, len, f);
    fclose(f);
  }
}


