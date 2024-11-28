#ifndef _MACHINE_H
#define _MACHINE_H

#define EVENT_VSYNC     0
#define EVENT_RESET     1

void machine_ProcessKey(uint8_t key, int pressed);
void machine_UpdateScreen();
void machine_Poll();
void machine_Kill();
void machine_Init();
void machine_Event(uint8_t event);
uint32_t machine_GetAudioOut(void);
void machine_AudioIn(uint32_t samples);
void machine_MenuCommand(int id, int item);
void machine_RedrawScreen();
#endif
	
