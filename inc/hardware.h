#ifndef _SCREEN_H
#define _SCREEN_H

extern int quit;
extern int debug;

enum {
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_SPACE,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_SEMI,
	KEY_COLON,
	KEY_COMMA,
	KEY_EQU,
	KEY_DOT,
//	KEY_COMMA,
	KEY_RETURN,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_SLASH1,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_SLASH2,
	KEY_BREAK,
	KEY_REPEAT,
	KEY_DELETE,
	KEY_LIST,
	KEY_ENDSTOP,
	KEY_MENU,
	KEY_RESET
};

void hw_ClearScreen();
void hw_InitScreen();
void hw_PutPixel(int x, int y, uint32_t pixel);
void hw_UpdateScreen();
void hw_Poll();
uint8_t *hw_ReadIn(const char *file, uint8_t *buffer, int max, unsigned long *actual);
void hw_WriteOut(const char *filename, void *data, int len);
void hw_Update();

#endif
