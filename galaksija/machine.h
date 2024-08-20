#ifndef _MACHINE_H
#define _MACHINE_H

void machine_ProcessKey(uint8_t key, int pressed);
void machine_UpdateScreen();
void machine_Poll();
void machine_Kill();
void machine_Init();

#endif
	
