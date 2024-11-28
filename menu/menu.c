#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//#include "keymap.h"
#include "hardware.h"
#include "machine.h"
#include "menu.h"
// #include "fat32.h"
#include "font.xbm"

#define CHAR_WIDTH font_width / 16
#define CHAR_HEIGHT font_height / 16

static void draw_char(unsigned int x, unsigned int y, unsigned int scale, char character) {
    uint8_t row = character / 16;
    uint8_t column = character % 16;
    uint32_t font_x = column * 10;
    uint32_t font_y = row * 12;

    for (int j = font_y; j < font_y + CHAR_HEIGHT; j++) {
      for  (int i = font_x; i <  font_x + CHAR_WIDTH; i++) {
        // XBM images pad rows with zeros
        uint32_t array_position;
        if (font_width %8 != 0) {
          array_position = (j * (font_width + (8 - font_width % 8))) + i;
        }
        else {
          array_position = (j * font_width) + i;
        }
        uint32_t array_index = array_position / 8;
        // XBM image, low bits are first
        uint32_t byte_position = array_position % 8;

        uint8_t value = font_bits[array_index] >> byte_position & 1 ;

        for (int sx = 0; sx < scale; sx++) {
          for (int sy = 0; sy < scale; sy++) {
            hw_PutPixel(x + ((i - font_x) * scale) + sx, y + ((j- font_y) * scale) + sy, !value);
          }
        }
      }
    }
}

#define MENU_LINES 16

static void draw_string(unsigned int x, unsigned int y, unsigned int scale, char *str) {
  while (*str) {
    draw_char(x, y, scale, *str);
    str++;
    x += scale * CHAR_WIDTH;
  }
}

static int item = 0;
static int maxitem = 0;
static int menu_active = 0;
static int menu_id = 0;

uint8_t menuActive() {
  return menu_active;
}

void draw_cursor(int item, int show) {
  draw_char(20 - CHAR_WIDTH*2, 20 + (CHAR_HEIGHT*item), 1, show ? '>' : ' ');
}

void menuOpen(int id, const char *menu[]) {
  int x, y, i;

  x = 20;
  y = 20;
  i = 0;

  hw_ClearScreen();

  maxitem = 0;
  while (menu[i]) {
    draw_string(x, y, 1, menu[i]);
    i++;
    y+=CHAR_HEIGHT;
    maxitem ++;
  }
  item = 0;
  menu_active = 1;
  draw_cursor(0, 1);
}

void menuClose() {
  hw_ClearScreen();
  menu_active = 0;
  machine_RedrawScreen();
}

int menuAction(uint16_t act) {
  printf("menuAction: act %d active %d\n", act, menu_active);
  switch(act) {
    case KEY_MENU:
//       menuClose();
      break;
    case KEY_BREAK:
      menuClose();
      break;
    case KEY_UP:
      if (item) {
        draw_cursor(item, 0);
        item --;
        draw_cursor(item, 1);
      }
      break;
    case KEY_DOWN:
      if ((item + 1) < maxitem) {
        draw_cursor(item, 0);
        item ++;
        draw_cursor(item, 1);
      }
      break;
    case KEY_LEFT:
    case KEY_RIGHT:
      break;
    case KEY_RETURN:
      machine_MenuCommand(menu_id, item);
      break;
  }
  return menu_active;
}

