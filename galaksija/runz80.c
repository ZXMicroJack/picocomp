/* This file is part of fpga-spec by ZXMicroJack - see LICENSE.txt for moreinfo */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// 263299,28     39%

//052E: ld     hl,$0482      AT SP: 3E00 05FF  FLAGS: .....VNC
//                           AF:2A07 BC:FBFB DE:002A HL:0089 IX:028F IY:4000
//0531: add    hl,de         AT SP: 3E00 05FF  FLAGS: .....VNC
//                           AF:2A07 BC:FBFB DE:002A HL:0482 IX:028F IY:4000
//0532: add    hl,de         AT SP: 3E00 05FF  FLAGS: .....V..
//                           AF:2A04 BC:FBFB DE:002A HL:04AC IX:028F IY:4000
//0533: ld     c,(hl)        AT SP: 3E00 05FF  FLAGS: .....V..
//                           AF:2A04 BC:FBFB DE:002A HL:04D6 IX:028F IY:4000
//0534: inc    hl            AT SP: 3E00 05FF  FLAGS: .....V..
//                           AF:2A04 BC:FB40 DE:002A HL:04D6 IX:028F IY:4000
//0535: ld     b,(hl)        AT SP: 3E00 05FF  FLAGS: .....V..
//                           AF:2A04 BC:FB40 DE:002A HL:04D7 IX:028F IY:4000
//0536: push   bc            AT SP: 3E00 05FF  FLAGS: .....V..



#include "screen.h"
#include "Z80.h"

#define MAX_SAMPLES 2000000

byte mem[0x10000];

void updateScreen();
void initScreen();
void initScreenJupiterAce();

#define N_PAGES  9
byte pagemem[N_PAGES][32768];
int paging = 0;
int biosmode = 0;

// char wave[1574599];
char *wave;
unsigned long waveSize = 0;
double speed = 1.0;

int saveMode = 0;

#define BMPSIZE 147510

byte bmpHeader[] = {
  0x42, 0x4d, 0x36, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x23, 0x2e, 0x00, 0x00, 0x23, 0x2e, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

byte *readIn(const char *file, byte *buffer, int max, unsigned long *actual) {
  FILE *f = fopen(file, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    unsigned long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    int toRead = max > size ? size : max;

    if (buffer == NULL) {
      buffer = (byte *)malloc(toRead);
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

// 3494400 TSTATES / SECOND
  // Z80_ICount

#define TSS 3494400
//#define SAMPLE_RATE 44100.0
#define SAMPLE_RATE 48000.0

unsigned long tstates = 0;

unsigned long sdcardSize = 0;
unsigned char *sdcard;
unsigned char sdsect[512];
unsigned short sdsectwp = 0, sdsectrp = 0;

// 16'h00ff: cpu_din[7:0] <= {sdcard_rfifo_empty, sdcard_rfifo_full, 6'h00};
// 16'h01ff: begin
//   sdcard_rfifo_rclk <= 1;
//   cpu_din[7:0] <= sdcard_rfifo_data;

byte Z80_In (dword port) {
  Z80_Regs regs;
  Z80_GetRegs(&regs);

	uint8_t Port = port & 0xff;

  if (Port == 0xfe) {
    unsigned port = (regs.AF.W.l) | Port;
    unsigned long realTstates = tstates + Z80_IPeriod - Z80_ICount;
    unsigned long sampleNr = (unsigned)(((double)realTstates*SAMPLE_RATE*speed)/(double)TSS);

    if (!saveMode) {
      if (sampleNr < waveSize) {
        byte output = 0xbf | ((wave[sampleNr] > 0) ? 0x40 : 0x00);
        // printf("sampleNr %u out %02X sample %d\n", sampleNr, output, wave[sampleNr]);
        return output;
        // return 0xbf | ((wave[sampleNr] > 0) ? 0x40 : 0x00);
      } else {
        return 0xff;
      }
    } else {
      return 0xff;
    }
  } else if (Port == 0xff) {
    unsigned port = (regs.BC.W.l) | Port;
    byte data;
    if (port == 0x00ff) data = ((sdsectwp == sdsectrp) ? 0x80 : 0x00) |
        ((sdsectwp == 512 && sdsectrp == 0) ? 0x40 : 00) | 0x30;
    else if (port == 0x01ff) data = sdsect[sdsectrp++];
    if (port != 0x01ff) printf("in(port %04X) returns %02X\n", port, data);

    return data;
  } else return 0xff;

  return 0;
}

/****************************************************************************/
/* Output a byte to given I/O port                                          */
/****************************************************************************/
int lastSample = -1;
byte gotit[256] = {0};

// end else if (addr[15:0] == 16'h00ff) begin
//   sd_rd <= cpu_dout[0];
//
//   if (cpu_dout[1]) begin // reset fifo
//     sdcard_rfifo_reset <= 1;
//     sdcard_rfifo_rclk <= 1;
//     sdcard_rfifo_wclk <= 1;
//   end
// end else if (addr[15:0] == 16'h01ff) begin
//   sd_address[31:24] <= cpu_dout;
// end else if (addr[15:0] == 16'h02ff) begin
//   sd_address[23:16] <= cpu_dout;
// end else if (addr[15:0] == 16'h03ff) begin
//   sd_address[15:8] <= cpu_dout;
// end
byte sd_address[3] = {0};
void Z80_Out (dword port,byte Value) {
  Z80_Regs regs;
  Z80_GetRegs(&regs);

	uint8_t Port = port & 0xff;

  if (Port == 0xfe) {
    unsigned port = (regs.AF.W.l) | Port;
    unsigned long realTstates = tstates + Z80_IPeriod - Z80_ICount;
    unsigned long sampleNr = (unsigned)(((double)realTstates*SAMPLE_RATE*speed)/(double)TSS);
    if (saveMode && sampleNr < MAX_SAMPLES) {
      if (lastSample > 0) {
        lastSample ++;
        while (lastSample < sampleNr) {
          wave[lastSample] = wave[lastSample-1];
          lastSample++;
        }
        wave[sampleNr] = (Value & 0x08) ? 0x7f : 0x80;
        lastSample = sampleNr;
      }
      lastSample = sampleNr;
    }
  } else if (Port == 0xdb) {
    printf("dbg %02X\n", Value);
  } else if (Port == 0xff) {
    unsigned port = (regs.BC.W.l) | Port;
    printf("out(port %04X, data %02X)\n", port, Value);
    switch(port) {
      case 0x00ff: // cmd
        if (Value & 1) {
          unsigned long pos = (sd_address[0]<<24)|
            (sd_address[1]<<16)|
            (sd_address[2]<<8);

          printf("Reading from card pos %08x\n", pos);

          if (pos < (sdcardSize - 512)) memcpy(sdsect, sdcard + pos, 512);
          else                          memset(sdsect, 0xff, 512);
          sdsectwp = 512;
          sdsectrp = 0;
        } else if (Value & 2) {
          printf("Reset sdsector buffer\n");
          sdsectwp = sdsectrp = 0;
        }
        break;

      case 0x01ff:
        sd_address[0] = Value;
        break;

      case 0x02ff:
        sd_address[1] = Value;
        break;

      case 0x03ff:
        sd_address[2] = Value;
        break;

      case 0x04ff:
        printf("Switch upper 32kb RAM page to %d ... ", Value);
        if (Value >= N_PAGES) exit(-1);
        paging = Value;
        printf("Done.\n");
        break;
    }
  }
}

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
unsigned Z80_RDMEM(dword A) {
  if (paging && A >= 32768) {
    return pagemem[paging][A-32768];
  } else return mem[A];
}

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
void Z80_WRMEM(dword A,byte V) {
  // if (A >= 16384 && A < (16384+6912)) {
  //   printf("writemem(addr = %04X, value = %02X)\n", A, V);
  // }
  if (!biosmode && A < 16384) {
    Z80_Regs regs;
    Z80_GetRegs(&regs);
    printf("[Warning] Trying to write to location 0x%04X at PC 0x%04X\n", A, regs.PC.D);
  } else if (biosmode && A < 8192) {
    Z80_Regs regs;
    Z80_GetRegs(&regs);
    printf("[Warning] Trying to write to location 0x%04X at PC 0x%04X\n", A, regs.PC.D);
  } else {
    if (paging && A >= 32768) {
      pagemem[paging][A-32768] = V;
    } else mem[A] = V;
  }
}

//int quit = 0;
void Z80_Debug(Z80_Regs *R) {
  quit = 1;
}

void Z80_Reti (void) {
  Z80_Running = 0;
}
void Z80_Retn (void) {}

int Z80_IRQ = 0;

int Z80_Interrupt(void) {
  return Z80_IGNORE_INT;
}

void Z80_Patch (Z80_Regs *Regs) {
  // disk access?
}

void writeScreen(const char *filename) {
  byte *bmp = (byte *) malloc(BMPSIZE);
  memcpy(bmp, bmpHeader, sizeof bmpHeader);
  int p=sizeof bmpHeader;

  unsigned long colourLut[] = {
    0x000000, 0x0000d7, 0xd70000, 0xd700d7,
    0x00d700, 0x00d7d7, 0xd7d700, 0xd7d7d7,
    0x000000, 0x0000ff, 0xff0000, 0xff00ff,
    0x00ff00, 0x00ffff, 0xffff00, 0xffffff
  };

  for (int i=191; i>=0; i--) {
    for (int j=0; j<32; j++) {
      int l = (i/64) * 2048;
      l += (i & 7) * 256;
      l += ((i & 63) / 8) * 32;

      byte d = mem[16384+l+j];
      int y = i / 8;
      byte attr = mem[16384+2048*3+y*32+j];
      unsigned long rgbInk = colourLut[(attr & 7) + ((attr&0x40) ? 8 : 0)];// ^ 0xffffff;
      unsigned long rgbPaper = colourLut[((attr >> 3) & 7) + ((attr&0x40) ? 8 : 0)];// ^ 0xffffff;
      for (int k=0; k<8; k++) {
        unsigned long rgb = (d & 0x80) ? rgbInk : rgbPaper;
        bmp[p++] = rgb & 0xff;
        bmp[p++] = (rgb >> 8) & 0xff;
        bmp[p++] = rgb >> 16;

        d <<= 1;
      }
    }
  }

  FILE *f = fopen(filename, "wb");
  if (f) {
    fwrite(bmp, 1, BMPSIZE, f);
    fclose(f);
  }
  free(bmp);
}

void writeOut(const char *filename, void *data, int len) {
  FILE *f = fopen(filename, "wb");
  if (f) {
    fwrite(data, 1, len, f);
    fclose(f);
  }
}

int main(int argc, char **argv) {

  memset(&mem[16384+3*2048], 0x38, 768);
  if (argc < 5) {
    printf("Usage: load <rom.bin> <loader.bin> <location> <testrawfile> [<speed_x>]\n");
    printf("Usage: save <rom.bin> <saver.bin> <location> <testfile>  [<speed_x>]\n");
    printf("Usage: run <rom.bin> <routine.bin> <location> <sdcard>\n");
    return 1;
  }

  speed = argc > 7 ? atof(argv[6]) : 1.0;
  dword location = atoi(argv[4]);
  if (strcmp(argv[2], "-")) readIn(argv[2], mem, 16384, NULL);
  if (strcmp(argv[3], "-")) readIn(argv[3], &mem[location], 4096, NULL);

  if (!strcmp(argv[1], "load")) {
    wave = readIn(argv[5], NULL, MAX_SAMPLES, &waveSize);
  } else if (!strcmp(argv[1], "save")) {
    int loc = 16384;
    readIn(argv[5], &mem[loc], 65536-loc, NULL);
    wave = malloc(MAX_SAMPLES);
    saveMode = 1;
  } else if (!strcmp(argv[1], "run")) {
    wave = NULL;
    sdcard = readIn(argv[5], NULL, 32*1024*1024, &sdcardSize);
  } else if (!strcmp(argv[1], "bios")) {
    wave = NULL;
    biosmode = 1;
    sdcard = readIn(argv[5], NULL, 32*1024*1024, &sdcardSize);

    for (int i=1; i<N_PAGES; i++) {
      char fn[256];
      sprintf(fn, "rampage%d.bin", i);
      readIn(fn, pagemem[i], 32*1024, NULL);
    }
    Z80_Trap = 0x120;
  }


  initScreen();

  Z80_Regs regs;
  Z80_GetRegs(&regs);
  regs.PC.D = location;
  Z80_SetRegs(&regs);

  while (!quit && Z80_Execute()) {
    updateScreen();
    tstates += Z80_IPeriod;

    if (tstates > 500000000) {
      break;
    }
  }

  Z80_RegisterDump();

  unsigned long realTstates = tstates + Z80_IPeriod - Z80_ICount;

  printf("Terminated after %lu tstates (%lu seconds)\n", realTstates, realTstates/3500000);
  printf("Speed is set to x %.3f", speed);

  if (!strcmp(argv[1], "load")) {
    writeScreen("output.bmp");
    writeOut("output.bin", &mem[16384], 6912);
    free(wave);
  } else if (!strcmp(argv[1], "save")) {
    writeOut("output.raw", wave, lastSample+1);
    free(wave);
  } else if (!strcmp(argv[1], "run") || !strcmp(argv[1], "bios")) {
    writeScreen("output.bmp");

    writeOut("specram.bin", &mem[16384], 48*1024);
    for (int i=1; i<N_PAGES; i++) {
      char fn[256];
      sprintf(fn, "rampage%d.bin", i);
      writeOut(fn, pagemem[i], 32*1024);
    }

    free(sdcard);
  }

  return 0;
}
