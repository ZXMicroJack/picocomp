#ifndef _MACHINE_H
#define _MACHINE_H

#define EVENT_VSYNC     0

void machine_ProcessKey(uint8_t key, int pressed);
void machine_UpdateScreen();
void machine_Poll();
void machine_Kill();
void machine_Init();
void machine_Event(uint8_t event);

#endif
	
