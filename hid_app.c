/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board.h"
#include "tusb.h"
//#include "debug.h"
//#include "usbhost.h"
#include "hardware.h"
#include "machine.h"

#define MAX_USB 8

#ifndef debug
#define debug(a) printf a
#endif

#if 0
void usb_ToPS2Mouse(uint8_t report[], uint16_t len) {
  if (curr_legacy_mode != LEGACY_MODE) return;
  
  if (len >= 3) {
      uint8_t ps2[4];
      ps2[0] = 1;
      ps2[1] = (report[0] & 7) | 0x08;
      ps2[2] = report[1]; // x
      ps2[3] = -report[2]; // y

#ifdef MB2
      mb2_SendPS2(ps2, 4);
#else
      for (int i=1; i<4; i++) {
        ps2_SendChar(1, ps2[i]);
      }
#endif
  }
}
#endif

// remap modifiers to each other if requested
//  bit  0     1      2    3    4     5      6    7
//  key  LCTRL LSHIFT LALT LGUI RCTRL RSHIFT RALT RGUI

const static uint8_t mod_lut[] = {0xff,0xff,KEY_RSHIFT,0xff,0xff,0xff,KEY_LSHIFT,0xff}; 
static uint16_t keymap(uint16_t scancode) {
	switch(scancode) {
		case 0x04: return KEY_A;
		case 0x05: return KEY_B;
		case 0x06: return KEY_C;
		case 0x07: return KEY_D;
		case 0x08: return KEY_E;
		case 0x09: return KEY_F;
		case 0x0a: return KEY_G;
		case 0x0b: return KEY_H;
		case 0x0c: return KEY_I;
		case 0x0d: return KEY_J;
		case 0x0e: return KEY_K;
		case 0x0f: return KEY_L;
		case 0x10: return KEY_M;
		case 0x11: return KEY_N;
		case 0x12: return KEY_O;
		case 0x13: return KEY_P;
		case 0x14: return KEY_Q;
		case 0x15: return KEY_R;
		case 0x16: return KEY_S;
		case 0x17: return KEY_T;
		case 0x18: return KEY_U;
		case 0x19: return KEY_V;
		case 0x1a: return KEY_W;
		case 0x1b: return KEY_X;
		case 0x1c: return KEY_Y;
		case 0x1d: return KEY_Z;
		case 0x2c: return KEY_SPACE;
		case 0x27: return KEY_0;
		case 0x1e: return KEY_1;
		case 0x1f: return KEY_2;
		case 0x20: return KEY_3;
		case 0x21: return KEY_4;
		case 0x22: return KEY_5;
		case 0x23: return KEY_6;
		case 0x24: return KEY_7;
		case 0x25: return KEY_8;
		case 0x26: return KEY_9;
		case 0x33: return KEY_SEMI;
		case 0x34: return KEY_COLON;
		case 0x36: return KEY_COMMA;
		case 0x2e: return KEY_EQU;
		case 0x37: return KEY_DOT;
		case 0x28: return KEY_RETURN;
//		case 0x06: return KEY_LSHIFT;
//		case 0x06: return KEY_RSHIFT;
		case 0x38: return KEY_SLASH1;
		case 0x52: return KEY_UP;
		case 0x51: return KEY_DOWN;
		case 0x50: return KEY_LEFT;
		case 0x4f: return KEY_RIGHT;
//		case 0x06: return GKEY_SLASH2;
		case 0x29: return KEY_BREAK;
		case 0x2b: return KEY_REPEAT;
		case 0x2a: return KEY_DELETE;
		case 0x2f: return KEY_LIST; //[
// 0x39 clockpc
	}
	return 0xff;
}




static uint32_t pressed[256/32];
static uint8_t hidreport[6];
static uint8_t prev_modifier = 0;
static void usb_KeyboardDecode(uint8_t modifier, uint8_t keys[6]) {
  static int firsttime = 1;
  uint32_t newpressed[8] = {0,0,0,0,0,0,0,0};

  // first time initialise
  if (firsttime) {
    firsttime = 0;
    memset(pressed, 0, sizeof pressed);
    memset(hidreport, 0, sizeof hidreport);
  }

  // detect newly pressed
  for (int i=0; i<6; i++) {
    uint8_t off = keys[i] >> 5;
    uint32_t bit = 1 << (keys[i] & 0x1f);

    if ((pressed[off] & bit) == 0) {
      // not previously pressed
      pressed[off] |= bit;

      // indicate pressed
      machine_ProcessKey(keymap(keys[i]), 1);
    }
    newpressed[off] |= bit;
  }

  for (int i=0; i<6; i++) {
    uint8_t off = hidreport[i] >> 5;
    uint32_t bit = 1 << (hidreport[i] & 0x1f);

    if ((newpressed[off] & bit) == 0) {
      // indicate released
      pressed[off] &= ~bit;
      machine_ProcessKey(keymap(keys[i]), 0);
    }
    
  }

  // copy latest report
  memcpy(hidreport, keys, sizeof hidreport);

  // handle modifiers
  uint8_t m = 0x80;
  for (int i=0; i<8; i++) {
      if (!(prev_modifier&m) && (modifier&m)) {
        machine_ProcessKey(mod_lut[i], 1);
      }
      if ((prev_modifier&m) && !(modifier&m)) {
        machine_ProcessKey(mod_lut[i], 0);
      }
      m >>= 1;
  }
  prev_modifier = modifier;

}

void hid_app_task(void)
{
}

#if 0 //TODO MJ will need
void kbd_set_leds(uint8_t data) {
  if(data > 7) data = 0;
  leds = led2ps2[data];
  tuh_hid_set_report(kbd_addr, kbd_inst, 0, HID_REPORT_TYPE_OUTPUT, &leds, sizeof(leds));
}
#endif

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  uint8_t itf_protocol = tuh_hid_interface_protocol(dev_addr, instance); 
  //uint8_t type = itf_protocol == HID_ITF_PROTOCOL_KEYBOARD ? USB_TYPE_KEYBOARD :
  //  itf_protocol == HID_ITF_PROTOCOL_MOUSE ? USB_TYPE_MOUSE : USB_TYPE_HID;

  // request to receive report
  // request_report(dev_addr, instance);
  if ( !tuh_hid_receive_report(dev_addr, instance)) {
    debug(("Error: cannot request to receive report\n"));
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
    if (len == 8) {
      usb_KeyboardDecode(report[0], &report[2]);
    } else {
      usb_KeyboardDecode(report[1], &report[3]);
    }
  }

  if ( !tuh_hid_receive_report(dev_addr, instance)) {
    debug(("Error: cannot request to receive report\n"));
  }
}

