#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>

#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "connections.h"
#include "cvideo.h"


// bool pong_gametick_callback(struct repeating_timer* t) {
//   pong_tick();
//   return true;
// }

#define CVIDEO_DATA_PIN 2
#define CVIDEO_SYNC_PIN 3


#define LINE_WORD_COUNT CVIDEO_PIX_PER_LINE / 32

uint32_t pix[CVIDEO_LINES][LINE_WORD_COUNT];
uint32_t current_pix = 0;
uint32_t current_line = 0;

uint32_t data_callback(void) {
  uint32_t *line = pix[current_line];
  uint32_t data = line[current_pix];
  current_pix++;

  if (current_pix == LINE_WORD_COUNT) {
    current_pix = 0;
    current_line = current_line + 2;

    if (current_line == CVIDEO_LINES + 1){
      current_line = 0;
    }
    else if (current_line == CVIDEO_LINES){
      current_line = 1;
    }
  }
  return data;
}



int main()
{
  // Defaults: UART 0, TX pin 0, RX pin 1, baud rate 115200
  stdio_init_all();
//   gpio_set_function(SERIAL_RX_PIN, GPIO_FUNC_UART);
//   gpio_set_function(SERIAL_TX_PIN, GPIO_FUNC_UART);

  printf("PICOCOMP Microjack\'23\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");


  for (int i=0; i<CVIDEO_LINES; i++)
    for (int j=0; j<LINE_WORD_COUNT; j++)
      pix[i][j] = (i&0x8) ? 0xffff0000 : 0x0000ffff;

      // PIO starts with odd lines
  current_line = 1;
  current_pix = 0;
  cvideo_init(pio0, CVIDEO_DATA_PIN, CVIDEO_SYNC_PIN, data_callback);


//   pong_init();

//   struct repeating_timer timer;
//   add_repeating_timer_ms(PONG_FRAME_INTERVAL_ms, pong_gametick_callback, NULL, &timer);

  while(getchar_timeout_us(1000000) != 'q') {
    printf("!\n");
  }


  reset_usb_boot(0, 0);

  return 0;
}
