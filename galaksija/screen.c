/* This file is part of fpga-spec by ZXMicroJack - see LICENSE.txt for moreinfo */
#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include "Z80.h"

#include <stdio.h>
#include <stdint.h>
#include "galkeys.h"

static SDL_Surface *screen;
//extern byte mem[0x10000];
//uint8_t specKeys[8];
int quit = 0;
int debug = 0;

#if 0
unsigned long zxColours[] = {
  0x000000, 0x0000d7, 0xd70000, 0xd700d7,
  0x00d700, 0x00d7d7, 0xd7d700, 0xd7d7d7,
  0x000000, 0x0000ff, 0xff0000, 0xff00ff,
  0x00ff00, 0x00ffff, 0xffff00, 0xffffff
};


Uint32 colourLut[16];
#endif

static void (*pupdateScreen)() = (void (*)())NULL;

static void initScreenReal();

void pollKeyboard();
void updateScreen() {
  /* Lock the screen for direct access to the pixels */
  if ( SDL_MUSTLOCK(screen) ) {
      if ( SDL_LockSurface(screen) < 0 ) {
          fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
          return;
      }
  }

  pupdateScreen();

  if ( SDL_MUSTLOCK(screen) ) {
      SDL_UnlockSurface(screen);
  }

  /* Update just the part of the display that we've changed */
  SDL_UpdateRect(screen, 0, 0, 256*2, 192*2);

	pollKeyboard();
}

void initScreen(void (*scrUpd)()) {
	pupdateScreen = scrUpd;
	initScreenReal();
}

static void initScreenReal() {
  /* Initialize the SDL library */
  if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
     fprintf(stderr,
             "Couldn't initialize SDL: %s\n", SDL_GetError());
     exit(1);
  }

  /* Clean up on exit */
  atexit(SDL_Quit);
  /*
  * Initialize the display in a 640x480 8-bit palettized mode,
  * requesting a software surface
  */
  screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE);
  if ( screen == NULL ) {
     fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n",
                     SDL_GetError());
     exit(1);
  }

#if 0
  for (int i=0; i<sizeof zxColours / sizeof zxColours[0]; i++) {
    /* Map the color yellow to this display (R=0xff, G=0xFF, B=0x00)
       Note:  If the display is palettized, you must set the palette first.
    */
    colourLut[i] = SDL_MapRGB(screen->format, zxColours[i]>>16,
      (zxColours[i]>>8) & 0xff, zxColours[i] & 0xff);
  }

  memset(specKeys, 0xff, sizeof specKeys);
#endif
  quit = 0;
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel_ll(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

void putpixel(int x, int y, uint32_t pixel) {
	uint32_t _pixel = pixel ? 0xd7d7d7 : 0;
  putpixel_ll(screen, x*2, y*2, _pixel);
  putpixel_ll(screen, x*2+1, y*2, _pixel);
  putpixel_ll(screen, x*2, y*2+1, _pixel);
  putpixel_ll(screen, x*2+1, y*2+1, _pixel);
}

//0xfefe  SHIFT, Z, X, C, V            0xeffe  0, 9, 8, 7, 6
//0xfdfe  A, S, D, F, G                0xdffe  P, O, I, U, Y
//0xfbfe  Q, W, E, R, T                0xbffe  ENTER, L, K, J, H
//0xf7fe  1, 2, 3, 4, 5                0x7ffe  SPACE, SYM SHFT, M, N, B
#if 0
uint8_t scancodeLut[8][5] = {
	{SDLK_LSHIFT, SDLK_z, SDLK_x, SDLK_c, SDLK_v},
	{SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g},
	{SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t},
	{SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5},

	{SDLK_0, SDLK_9, SDLK_8, SDLK_7, SDLK_6},
	{SDLK_p, SDLK_o, SDLK_i, SDLK_u, SDLK_y},
	{SDLK_RETURN, SDLK_l, SDLK_k, SDLK_j, SDLK_h},
	{SDLK_SPACE, SDLK_LCTRL, SDLK_m, SDLK_n, SDLK_b}
};
#endif
#if 0
uint16_t keymap(uint8_t scancode) {
#undef KEYMAP
#define KEYMAP(a,b) case SDLK_##b: return GKEY_##a;

	switch(scancode) {
#include "../keymap2.h"
	}
}
#else
uint16_t keymap(uint16_t scancode) {
	switch(scancode) {
		case SDLK_a: return GKEY_A;
		case SDLK_b: return GKEY_B;
		case SDLK_c: return GKEY_C;
		case SDLK_d: return GKEY_D;
		case SDLK_e: return GKEY_E;
		case SDLK_f: return GKEY_F;
		case SDLK_g: return GKEY_G;
		case SDLK_h: return GKEY_H;
		case SDLK_i: return GKEY_I;
		case SDLK_j: return GKEY_J;
		case SDLK_k: return GKEY_K;
		case SDLK_l: return GKEY_L;
		case SDLK_m: return GKEY_M;
		case SDLK_n: return GKEY_N;
		case SDLK_o: return GKEY_O;
		case SDLK_p: return GKEY_P;
		case SDLK_q: return GKEY_Q;
		case SDLK_r: return GKEY_R;
		case SDLK_s: return GKEY_S;
		case SDLK_t: return GKEY_T;
		case SDLK_u: return GKEY_U;
		case SDLK_v: return GKEY_V;
		case SDLK_w: return GKEY_W;
		case SDLK_x: return GKEY_X;
		case SDLK_y: return GKEY_Y;
		case SDLK_z: return GKEY_Z;
		case SDLK_SPACE: return GKEY_SPACE;
		case SDLK_0: return GKEY_0;
		case SDLK_1: return GKEY_1;
		case SDLK_2: return GKEY_2;
		case SDLK_3: return GKEY_3;
		case SDLK_4: return GKEY_4;
		case SDLK_5: return GKEY_5;
		case SDLK_6: return GKEY_6;
		case SDLK_7: return GKEY_7;
		case SDLK_8: return GKEY_8;
		case SDLK_9: return GKEY_9;
		case SDLK_SEMICOLON: return GKEY_SEMI;
		case SDLK_COLON: return GKEY_COLON;
		case SDLK_COMMA: return GKEY_COMMA;
		case SDLK_EQUALS: return GKEY_EQU;
		case SDLK_PERIOD: return GKEY_DOT;
		case SDLK_RETURN: return GKEY_RETURN;
		case SDLK_LSHIFT: return GKEY_LSHIFT;
		case SDLK_RSHIFT: return GKEY_RSHIFT;
		case SDLK_SLASH: return GKEY_SLASH1;
		case SDLK_UP: return GKEY_UP;
		case SDLK_DOWN: return GKEY_DOWN;
		case SDLK_LEFT: return GKEY_LEFT;
		case SDLK_RIGHT: return GKEY_RIGHT;
//		case SDLK_SLASH: return GKEY_SLASH2;
		case SDLK_HOME: return GKEY_BREAK;
		case SDLK_END: return GKEY_REPEAT;
		case SDLK_PAGEUP: return GKEY_DELETE;
		case SDLK_PAGEDOWN: return GKEY_LIST;
	}
	return 0xff;
}
#endif
extern void processKey(uint16_t scancode, int pressed);
#if 0
void processKey(uint8_t scancode, int pressed) {
	printf("processKey %02X pressed %d\n", scancode, pressed);
	for (int r = 0; r < 8; r++) {
		uint8_t mask = 0x01;
		for (int c = 0; c<5; c++) {
			if (scancode == scancodeLut[r][c]) {
				if (pressed) {
					specKeys[r] &= ~mask;
				} else {
					specKeys[r] |= mask;
				}
				printf("row %d column %d kb %02X\n", r, c, specKeys[r]);
				return;
			}
			mask <<= 1;
		}
	}
}
#endif

void pollKeyboard() {
	SDL_Event event;

	/* Poll for events */
	while( SDL_PollEvent( &event ) ){
			switch( event.type ){
					/* Keyboard event */
					/* Pass the event data onto PrintKeyInfo() */
					case SDL_KEYDOWN:
					case SDL_KEYUP:
							if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
								quit = 1;
							} else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_TAB) {
								debug = !debug;
							}

							processKey(keymap(event.key.keysym.sym), event.type == SDL_KEYDOWN);
							break;

					/* SDL_QUIT event (window close) */
					case SDL_QUIT:
							quit = 1;
							break;

					default:
							break;
			}

	}
}
