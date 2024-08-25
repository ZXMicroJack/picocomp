#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>

#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "connections.h"
#include "cvideo.h"
#include "machine.h"


#define CVIDEO_DATA_PIN 2
#define CVIDEO_SYNC_PIN 3

#define LINE_WORD_COUNT CVIDEO_PIX_PER_LINE / 32

#define CVIDEO_MAX_WORDS (CVIDEO_LINES*LINE_WORD_COUNT)

uint32_t pix[CVIDEO_LINES*LINE_WORD_COUNT];
uint32_t current_pix = 0;
// uint32_t current_line = 0;

int vsyncs = 0;
void machine_Event(uint8_t event) {
  switch(event) {
    case EVENT_VSYNC:
      vsyncs ++;
  }
}

uint32_t data_callback(void) {
  if (current_pix >= CVIDEO_MAX_WORDS) current_pix = 0;
  return pix[current_pix++];
}

int main()
{
  // Defaults: UART 0, TX pin 0, RX pin 1, baud rate 115200
  stdio_init_all();

  printf("PICOCOMP Microjack\'23\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");

  /* create pattern */
  for (int i=0; i<CVIDEO_LINES; i++)
    for (int j=0; j<LINE_WORD_COUNT; j++)
      pix[i*LINE_WORD_COUNT+j] = (i&0x8) ? 0xffff0000 : 0x0000ffff;

  current_pix = 0;

  //cvideo_init_irq(pio0, CVIDEO_DATA_PIN, CVIDEO_SYNC_PIN, data_callback);
  cvideo_init_dma(pio0, CVIDEO_DATA_PIN, CVIDEO_SYNC_PIN, &pix[0]);

//   struct repeating_timer timer;
//   add_repeating_timer_ms(PONG_FRAME_INTERVAL_ms, pong_gametick_callback, NULL, &timer);

  while(getchar_timeout_us(1000000) != 'q') {
    printf("[vsyncs = %d]\n", vsyncs);
  }


  reset_usb_boot(0, 0);

  return 0;
}
