#ifndef _MACHINE_H
#define _MACHINE_H

void machine_ProcessKey(uint16_t scancode, int pressed);
void machine_UpdateScreen();
void machine_Poll();
void machine_Kill();
void machine_Init();

#endif
	
