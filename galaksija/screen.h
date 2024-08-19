#ifndef _SCREEN_H
#define _SCREEN_H

void putpixel(int x, int y, uint32_t pixel);

//void updateScreenSpectrum();
//void updateScreenJupiterAce();
//void updateScreenZX80();
//void initScreenJupiterAce();
//void initScreenZX80();
//void pollKeyboard();

extern uint8_t specKeys[8];
extern int quit;
extern int debug;
//void initScreen(void (*scrUpd)());


void hw_initScreen(void (*scrUpd)());
void hw_PollKeyboard();
void hw_PutPixel(int x, int y, uint32_t pixel);
void hw_UpdateScreen();

#endif
