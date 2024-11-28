#ifndef _MENU_H
#define _MENU_H

int menuAction(uint16_t act);
void menuOpen(int id, const char *menu[]);
uint8_t menuActive();
void menuClose();

#endif


