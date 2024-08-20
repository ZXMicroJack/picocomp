/* This file is part of fpga-spec by ZXMicroJack - see LICENSE.txt for moreinfo */
#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include "Z80.h"

#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include "galkeys.h"
#include "hardware.h"
#include "machine.h"

static SDL_Surface *screen;
int quit = 0;
int debug = 0;

void sighandler(int *n) {
	quit = 1;
}

void hw_UpdateScreen() {
  /* Lock the screen for direct access to the pixels */
  if ( SDL_MUSTLOCK(screen) ) {
      if ( SDL_LockSurface(screen) < 0 ) {
          fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
          return;
      }
  }

  machine_UpdateScreen();

  if ( SDL_MUSTLOCK(screen) ) {
      SDL_UnlockSurface(screen);
  }

  /* Update just the part of the display that we've changed */
  SDL_UpdateRect(screen, 0, 0, 256*2, 192*2);

	hw_Poll();
}

void hw_InitScreen() {
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

  quit = 0;
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
static void putpixel_ll(SDL_Surface *surface, int x, int y, Uint32 pixel)
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

void hw_PutPixel(int x, int y, uint32_t pixel) {
	uint32_t _pixel = pixel ? 0xd7d7d7 : 0;
  putpixel_ll(screen, x*2, y*2, _pixel);
  putpixel_ll(screen, x*2+1, y*2, _pixel);
  putpixel_ll(screen, x*2, y*2+1, _pixel);
  putpixel_ll(screen, x*2+1, y*2+1, _pixel);
}

static uint16_t keymap(uint16_t scancode) {
	switch(scancode) {
		case SDLK_a: return KEY_A;
		case SDLK_b: return KEY_B;
		case SDLK_c: return KEY_C;
		case SDLK_d: return KEY_D;
		case SDLK_e: return KEY_E;
		case SDLK_f: return KEY_F;
		case SDLK_g: return KEY_G;
		case SDLK_h: return KEY_H;
		case SDLK_i: return KEY_I;
		case SDLK_j: return KEY_J;
		case SDLK_k: return KEY_K;
		case SDLK_l: return KEY_L;
		case SDLK_m: return KEY_M;
		case SDLK_n: return KEY_N;
		case SDLK_o: return KEY_O;
		case SDLK_p: return KEY_P;
		case SDLK_q: return KEY_Q;
		case SDLK_r: return KEY_R;
		case SDLK_s: return KEY_S;
		case SDLK_t: return KEY_T;
		case SDLK_u: return KEY_U;
		case SDLK_v: return KEY_V;
		case SDLK_w: return KEY_W;
		case SDLK_x: return KEY_X;
		case SDLK_y: return KEY_Y;
		case SDLK_z: return KEY_Z;
		case SDLK_SPACE: return KEY_SPACE;
		case SDLK_0: return KEY_0;
		case SDLK_1: return KEY_1;
		case SDLK_2: return KEY_2;
		case SDLK_3: return KEY_3;
		case SDLK_4: return KEY_4;
		case SDLK_5: return KEY_5;
		case SDLK_6: return KEY_6;
		case SDLK_7: return KEY_7;
		case SDLK_8: return KEY_8;
		case SDLK_9: return KEY_9;
		case SDLK_SEMICOLON: return KEY_SEMI;
		case SDLK_COLON: return KEY_COLON;
		case SDLK_COMMA: return KEY_COMMA;
		case SDLK_EQUALS: return KEY_EQU;
		case SDLK_PERIOD: return KEY_DOT;
		case SDLK_RETURN: return KEY_RETURN;
		case SDLK_LSHIFT: return KEY_LSHIFT;
		case SDLK_RSHIFT: return KEY_RSHIFT;
		case SDLK_SLASH: return KEY_SLASH1;
		case SDLK_UP: return KEY_UP;
		case SDLK_DOWN: return KEY_DOWN;
		case SDLK_LEFT: return KEY_LEFT;
		case SDLK_RIGHT: return KEY_RIGHT;
//		case SDLK_SLASH: return GKEY_SLASH2;
		case SDLK_HOME: return KEY_BREAK;
		case SDLK_END: return KEY_REPEAT;
		case SDLK_PAGEUP: return KEY_DELETE;
		case SDLK_PAGEDOWN: return KEY_LIST;
	}
	return 0xff;
}

void hw_Poll() {
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

							machine_ProcessKey(keymap(event.key.keysym.sym), event.type == SDL_KEYDOWN);
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


uint8_t *hw_readIn(const char *file, uint8_t *buffer, int max, unsigned long *actual) {
  FILE *f = fopen(file, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    unsigned long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    int toRead = max > size ? size : max;

    if (buffer == NULL) {
      buffer = (uint8_t *)malloc(toRead);
    }
    int l = fread(buffer, 1, toRead, f);
    fclose(f);
    printf("Read %d bytes from %s\n", l, file);

    if (actual != NULL) *actual = l;

    return buffer;
  } else {
    printf("Couldn't open file %s\n", file);
  }
  return NULL;
}

void hw_writeOut(const char *filename, void *data, int len) {
  FILE *f = fopen(filename, "wb");
  if (f) {
    fwrite(data, 1, len, f);
    fclose(f);
  }
}

int main(int argc, char **argv) {

  signal(SIGINT, sighandler);

  hw_InitScreen();

  while (!quit) {
  	machine_Poll();
    hw_UpdateScreen();

  }

	machine_Kill();
//  writeOut("GALXram.bin", &mem[0x2000], 0x2000);

  return 0;
}


