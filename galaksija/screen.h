#ifndef _SCREEN_H
#define _SCREEN_H

extern int quit;
extern int debug;

void hw_InitScreen();
void hw_PutPixel(int x, int y, uint32_t pixel);
void hw_UpdateScreen();
void hw_Poll();
uint8_t *hw_ReadIn(const char *file, uint8_t *buffer, int max, unsigned long *actual);
void hw_WriteOut(const char *filename, void *data, int len);

#endif
