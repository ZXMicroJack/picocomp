/* This file is part of fpga-spec by ZXMicroJack - see LICENSE.txt for moreinfo */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"

#if defined(USB) && !defined (USBFAKE)
#include "bsp/board.h"
#include "tusb.h"
#endif


#include "connections.h"
#include "cvideo.h"

#include "galkeys.h"
#include "hardware.h"
#include "machine.h"

#include "audio.pio.h"

#include "Z80.h"


// static SDL_Surface *screen;
int quit = 0;
int debug = 0;

#define CVIDEO_DATA_PIN 2
#define CVIDEO_SYNC_PIN 3

#define LINE_WORD_COUNT CVIDEO_PIX_PER_LINE / 32

#define CVIDEO_MAX_WORDS (CVIDEO_LINES*LINE_WORD_COUNT)

uint32_t pix[CVIDEO_LINES*LINE_WORD_COUNT];


void hw_UpdateScreen() {
	hw_Poll();
}

void hw_InitScreen() {
}

void hw_ClearScreen() {
  memset(pix, 0, sizeof pix);
}

static void putpixel_ll(int x, int y, uint8_t pixel) {
  uint32_t *p = &pix[((y>>1) + (y&1)*(CVIDEO_LINES>>1))*LINE_WORD_COUNT+(x>>5)];
  if (pixel) {
    *p |= 1 << (x&31);
  } else {
    *p &= ~(1 << (x&31));
  }
}

void hw_PutPixel(int x, int y, uint32_t pixel) {
  y += 32;
  x += 48;

#if 0
  putpixel_ll(x*3, y*2, pixel);
  putpixel_ll(x*3+1, y*2, pixel);
  putpixel_ll(x*3+2, y*2, pixel);
  putpixel_ll(x*3, y*2+1, pixel);
  putpixel_ll(x*3+1, y*2+1, pixel);
  putpixel_ll(x*3+2, y*2+1, pixel);
#else
  putpixel_ll(x*2, y*2, pixel);
  putpixel_ll(x*2+1, y*2, pixel);
  putpixel_ll(x*2, y*2+1, pixel);
  putpixel_ll(x*2+1, y*2+1, pixel);
#endif
}

static uint16_t keymap(uint16_t scancode) {
	switch(scancode) {
		case 'a': return KEY_A;
		case 'b': return KEY_B;
		case 'c': return KEY_C;
		case 'd': return KEY_D;
		case 'e': return KEY_E;
		case 'f': return KEY_F;
		case 'g': return KEY_G;
		case 'h': return KEY_H;
		case 'i': return KEY_I;
		case 'j': return KEY_J;
		case 'k': return KEY_K;
		case 'l': return KEY_L;
		case 'm': return KEY_M;
		case 'n': return KEY_N;
		case 'o': return KEY_O;
		case 'p': return KEY_P;
		case 'q': return KEY_Q;
		case 'r': return KEY_R;
		case 's': return KEY_S;
		case 't': return KEY_T;
		case 'u': return KEY_U;
		case 'v': return KEY_V;
		case 'w': return KEY_W;
		case 'x': return KEY_X;
		case 'y': return KEY_Y;
		case 'z': return KEY_Z;
		case ' ': return KEY_SPACE;
		case '0': return KEY_0;
		case '1': return KEY_1;
		case '2': return KEY_2;
		case '3': return KEY_3;
		case '4': return KEY_4;
		case '5': return KEY_5;
		case '6': return KEY_6;
		case '7': return KEY_7;
		case '8': return KEY_8;
		case '9': return KEY_9;
    case ';': return KEY_SEMI;
		case '\'': return KEY_COLON;
		case ',': return KEY_COMMA;
		case '=': return KEY_EQU;
		case '.': return KEY_DOT;
		case '\r': return KEY_RETURN;
		case '`': return KEY_LSHIFT;
		case '#': return KEY_RSHIFT;
		case '/': return KEY_SLASH1;
		case 'U': return KEY_UP;
		case 'D': return KEY_DOWN;
		case 'L': return KEY_LEFT;
		case 'R': return KEY_RIGHT;
//		case 'SLASH': return GKEY_SLASH2;
		case 'B': return KEY_BREAK;
		case 'E': return KEY_REPEAT;
		case 'P': return KEY_DELETE;
		case 'O': return KEY_LIST;
    case '!': return KEY_MENU;
    case '?': return KEY_RESET;
	}
	return 0xff;
}

void hw_Poll() {
}


uint8_t *hw_readIn(const char *file, uint8_t *buffer, int max, unsigned long *actual) {
  return NULL;
}

void hw_writeOut(const char *filename, void *data, int len) {
}

#define AUDIO_SM_ID     0
#define AUDIO_IRQ PIO1_IRQ_1

int irqs = 0;

static void audio_isr(void) {
    pio_sm_put(pio1, AUDIO_SM_ID, machine_GetAudioOut());
    machine_AudioIn(pio_sm_get(pio1, AUDIO_SM_ID));
    irq_clear(AUDIO_IRQ);
    irqs ++;
}

uint32_t machine_GetAudioOut(void) {
  return 0;
}

void machine_AudioIn(uint32_t samples) {
//   machine_Poll();
}

static void z80_core() {
  for(;;) {
    machine_Poll();
  }

}

static uint32_t runs = 0;

int main()
{
  // Defaults: UART 0, TX pin 0, RX pin 1, baud rate 115200
  stdio_init_all();

#if defined(USB)
  board_init();
  tusb_init();
#endif
  printf("PICOCOMP Hardware layer for RP2040 Microjack\'23\n\n");

  /* clear screen */
  memset(pix, 0, sizeof pix);

//   current_pix = 0;

  cvideo_init_dma(pio0, CVIDEO_DATA_PIN, CVIDEO_SYNC_PIN, &pix[0]);

  uint offset = pio_add_program(pio1, &audio_program);
  audio_program_init(pio1, AUDIO_SM_ID, offset, AUDIO_IN_PIN, AUDIO_OUT_PIN);
  pio_sm_put(pio1, AUDIO_SM_ID, 0);

  irq_set_enabled(AUDIO_IRQ, true);
  irq_set_exclusive_handler(AUDIO_IRQ, audio_isr);
  irq_set_priority(AUDIO_IRQ,  0);
  pio1->inte1 = 1 << 4 + AUDIO_SM_ID;


  machine_Init();

//   multicore_reset_core1();
//   multicore_launch_core1(z80_core);


  int c;
  uint16_t k = 0;
  uint16_t lastkey = 0;
  uint64_t lastpress = 0;
  uint64_t lastdbg = 0;
  int lshift = 0;
  for(;;) {
#ifndef USB
    c = getchar_timeout_us(2);
    if (c == '|') break;
    k = keymap(c);
    if (c == KEY_LSHIFT) { // sticky lshift
      lshift = !lshift;
      machine_ProcessKey(k, lshift);
    } else {
      if (k != 0xff) {
        machine_ProcessKey(k, 1);
        lastkey = k;
        lastpress = time_us_64();
      }
    }

    if (lastkey && (time_us_64() -lastpress) > 50000) {
      lastpress = 0;
      machine_ProcessKey(lastkey, 0);
      lastkey = 0;
    }

    if ((time_us_64() - lastdbg) > 1000000) {
      lastdbg = time_us_64();
      printf("irqs = %d; runs = %d\n", irqs, runs);
    }
#else
    tuh_task();
#endif

    if (runs < irqs) {
      machine_Poll();
      runs ++;
    }

    machine_UpdateScreen();
  }

  reset_usb_boot(0, 0);

  return 0;
}



