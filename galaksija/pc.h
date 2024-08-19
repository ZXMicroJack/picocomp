#ifndef _PC_H
#define _PC_H

uint8_t *readIn(const char *file, uint8_t *buffer, int max, unsigned long *actual);
void writeOut(const char *filename, void *data, int len);

#endif


