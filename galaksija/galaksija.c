/* This file is part of fpga-spec by ZXMicroJack - see LICENSE.txt for moreinfo */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "Z80.h"
#include "hardware.h"
#include "machine.h"

/*******************************  ROMS ***************************/
#define UKUS 0x40 // uk is 0x40 us is 0x00
#define HALT 0x76
#define NOP 0x00
#define TAPE 0x00 // bit 0x80

#ifdef ZX81
#define FONT_POS 0x1e00
#else
#define FONT_POS 0xe00
#endif
#define TOP 56
#define BOTTOM 248




/*******************************  ROMS ***************************/
const static byte charrom[] = {
#include "charrom.h"
  };

const static byte rom[] = {
#include "rom.h"
};

/*******************************  STATE ***************************/
static byte mem[0x10000];
static uint8_t keymatrix[8];
static uint8_t latch_d5 = 0;
static uint8_t latch_byte = 0;


/*******************************  PROTOTYPES ***************************/

//void update(uint8_t d);

//int nmiPending = 0;
//int nmiGeneration = 0;
//int vsyncPulse = 0; 


/*******************************  ADDRESS DECODING ***************************/
static uint32_t at(uint16_t a) {
	uint8_t ram1 = (a & 0xf800) == 0x2800; //00101
	uint8_t ram2 = (a & 0xf800) == 0x3000; //00110
	uint8_t ram3 = (a & 0xf800) == 0x3800; //00110
	uint8_t sram1 = ((a & 0xc000) == 0x4000) || ((a & 0xc000) == 0x8000); // 01000 + 10000
	uint8_t rom = (a & 0xe000) == 0x00000;
	uint16_t ram_a7 = 0x0080 ^ ((a ^ 0x0080) && (latch_d5 ? 0x0080 : 0x0000));

	uint32_t a_ = 
		rom ? (a & 0x1fff) :
		ram1 ? (a & 0x7ff) + 0x2000 :
		ram2 ? (a & 0x7ff) + 0x2800 :
		ram3 ? (a & 0x7ff) + 0x3000 :
		0xffffffff;



//	DECODER_EN <= (not(MREQ_n) and not(A(14))) and not(A(15));
//    LATCH_KBD_CS_n <= '0' when ((A(11)='0') and (A(12)='0') and (A(13)='1') and (DECODER_EN = '1')) else '1';
//	ROM_OE_n <= '0' when ((A(13)='0') and (DECODER_EN='1') and (RFSH = '0')) and apply_rom_patch = '0' else
//					'1';
//	ROM_A <= A(12 downto 0);
//	RAM_CS1_n <= '0' when ((DECODER_EN='1') and (A(11)='1') and (A(12)='0') and (A(13)='1')) else '1';  -- 00 101 
//	RAM_CS2_n <= '0' when ((DECODER_EN='1') and (A(11)='0') and (A(12)='1') and (A(13)='1')) else '1';
//	RAM_CS3_n <= '0' when ((DECODER_EN='1') and (A(11)='1') and (A(12)='1') and (A(13)='1')) else '1';
//	SRAM_CS1_n <= '0' when MREQ_n = '0' and (A(15 downto 14)="01" or A(15 downto 14)="10") else '1';
//  	SRAM_CS2_n <= '0' when MREQ_n = '0' and A(15 downto 14)/="00" and A(15 downto 4) /= X"FFF" else '1';
//	RAM_CS_n <= RAM_CS1_n and RAM_CS2_n and RAM_CS3_n and RAM_CS4_n;
//	RAM_A7 <= not(not(A(7)) and LATCH_D5);
//	RAM_A <= "00" & A(10 downto 8) & RAM_A7 & A(6 downto 0) when RAM_CS1_n = '0' else
//				"01" & A(10 downto 8) & RAM_A7 & A(6 downto 0) when RAM_CS2_n = '0' else
//				"10" & A(10 downto 8) & RAM_A7 & A(6 downto 0) when RAM_CS3_n = '0' else
//				"11" & A(10 downto 8) & RAM_A7 & A(6 downto 0);
//	SRAM_ADDR(15 downto 0) <= A(15 downto 8) & RAM_A7 & A(6 downto 0);

	return a_;
}

unsigned long tstates = 0;

byte Z80_In (dword Port) {
#if 0
   //0xfefe  SHIFT, Z, X, C, V            0xeffe  0, 9, 8, 7, 6
   //0xfdfe  A, S, D, F, G                0xdffe  P, O, I, U, Y
   //0xfbfe  Q, W, E, R, T                0xbffe  ENTER, L, K, J, H
   //0xf7fe  1, 2, 3, 4, 5                0x7ffe  SPACE, SYM SHFT, M, N, B
  if ((Port & 0xfe) == 0xfe) {
  	vsyncPulse = 1;
   	uint16_t mask = 0x0100;
   	uint8_t data = UKUS | 0x1f;
		for (int i=0; i<8; i++) {
			if ((~Port & mask) == mask) {
				data &= specKeys[i];
			}
			mask <<= 1;
		}
		return data;
  }

	return 0x1f | UKUS;
#else
  return 0xff;
#endif
}

/****************************************************************************/
/* Output a byte to given I/O port                                          */
/****************************************************************************/

void Z80_Out (dword Port,byte Value) {
//  printf("out %04X,%02X\n", Port, Value);
#if 0
  if ((Port & 0xff) == 0xff) {
  	// pulse audio
		reset();
		vsyncPulse = 0;
  	return;
  }
#endif
#ifdef ZX81
  if (Port == 0xfd) {
  	nmiGeneration = 0;
  }

  if (Port == 0xfe) {
  	nmiGeneration = 1;
  }
#endif
}

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/

//LATCH_KBD_CS_n <= '0' when ((A(11)='0') and (A(12)='0') and (A(13)='1') and (DECODER_EN = '1')) else '1';
//KRsel <= A(5) & A(4) & A(3);
 
//if (LATCH_KBD_CS_n = '0') then
//	case (KRsel) is
//		when "000" => KR <= "11111110";
//		when "001" => KR <= "11111101";
//		when "010" => KR <= "11111011";
//		when "011" => KR <= "11110111";
//		when "100" => KR <= "11101111";
//		when "101" => KR <= "11011111";
//		when "110" => KR <= "10111111";
//		when "111" => KR <= "01111111";
//		when others => KR <= "11111111";
//	end case;
//else
//	KR <= "11111111";
//end if;
//KSBUF_en <= LATCH_KBD_CS_n when RD_n = '0' else '1';

//KSBUF_en <= LATCH_KBD_CS_n when RD_n = '0' else '1';
//KSsel <= A(2) & A(1) & A(0);
//-- Multiplex the keyboard scanlines
//process(KSsel, LATCH_KBD_CS_n, KS, RD_n)
//begin
//		case KSsel is
//			when "000" => KSout <= KS(0);
//			when "001" => KSout <= KS(1);
//			when "010" => KSout <= KS(2);
//			when "011" => KSout <= KS(3);
//			when "100" => KSout <= KS(4);
//			when "101" => KSout <= KS(5);
//			when "110" => KSout <= KS(6);
//			when "111" => KSout <= KS(7);
//			when others => KSout <= '1';
//		end case;
//end process;

//CPU CLK 6.144 MHz
//PIX_CLK_COUNTER = 12M288
//CHROM_A <= LATCH_DATA(3 downto 0) & TMP(7) & TMP(5 downto 0);

#ifndef TESTS
uint16_t lastpc = 0;
unsigned Z80_RDMEM(dword A) {
	uint16_t pc = Z80_GetPC();

  if (pc == 0x008b) {
    Z80_Regs regs;
    Z80_GetRegs(&regs);
    //printf("[R = %02X %02X]\n", regs.R, regs.R2);
//    printf("[R = %04X]\n", (regs.R&127)|(regs.R2&128));
    uint16_t r = (regs.R&127)|(regs.R2&128);
    uint16_t va = ((latch_byte >> 2) & 0xf) << 6;
    va |= (r & 0x3f);
    va |= (r & 0x80) >> 1;
//    printf("[VIDADDR %04X]\n", va);
  }

//	printf("[R,%04X(%04X),PC%04X]\n", A, at(A), pc);
	if (pc == 0) { printf("[RESET LASTPC:%04X]\n", lastpc); if (lastpc != 0) exit(1); }
	lastpc = pc;
// kbd row 0x2000-0x2007 - 0-7
// kbd col 0x2000-0x2037 - 0-6
// latch 0x2038-0x203f - latch
// repeats x 32
  if (A < 0x2000) return rom[A];
	if (A <= 0x2034 && A >= 0x2001) { // maybe keyboard?
		return 1 ^ (keymatrix[A&7] >> ((A>>3)&7)&1);
	} else if (at(A) == 0xffffffff) {
//	  printf("[R,%04X(%04X),PC%04X]\n", A, at(A), pc);
	  return 0xff;
	}

	// is m1 cycle
#if 0
	uint16_t pc = Z80_GetPC();
	if (pc == A && (pc&0x8000)) {
		update(d);
		if ((!(d&0x40))) d = NOP;
	}
#endif

//	if ((Z80_GetPC()&0x8000) && (!(d&0x40))) {
//		update(d);
//		d = NOP;
//	} else if ((Z80_GetPC()&0x8000) && (d&0x40)) {
//		update(d);
//	}
//	printf("[R,%04X(%04X),PC%04X]\n", A, at(A), pc);
	return mem[at(A)-0x2000];
}

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
void Z80_WRMEM(dword A,byte V) {
  uint32_t a_ = at(A);

//  printf("[W,%04X(%04X),%02X]\n", A, a_, V);

  if (A == 0x207f) {
//    printf("[LATCH %02X PC%04X]\n", V, Z80_GetPC());
    latch_byte = V;
  } else if (a_ == 0xffffffff) {
//    printf("[W,%04X(%04X),%02X]\n", A, a_, V);
  }

  if (a_ < 0x2000) {
    printf("[Warning] Trying to write to location 0x%04X at PC 0x%04X\n", A, Z80_GetPC());
  } else if (a_ != 0xffffffff) {
//	  printf("[W,%04X(%04X),%02X]\n", A, a_, V);
    mem[a_-0x2000] = V;
  }
}
#endif

// void Z80_Debug(Z80_Regs *R) {}
void Z80_Reti (void) {}
void Z80_Retn (void) {}

int Z80_IRQ = Z80_IGNORE_INT;
//int Z80_IRQ = Z80_NMI_INT;
//int Z80_IRQ = 0xff;

int Z80_Interrupt(void) {
#if 0
#ifdef ZX81
	if (nmiPending) {
		nmiPending = 0;
		return Z80_NMI_INT;
	}
#endif

	// always interrupt
//  return vsyncPulse ? 0xff : Z80_IGNORE_INT;
	  return 0xff;
#endif
//  return Z80_NMI_INT;
  return 0xff;
}

void Z80_Patch (Z80_Regs *Regs) {
  // disk access?
}

void tests() {
	#define t(a,b) if (at(a) != b) printf("!!! Error %04X should equal %04X not %04X\n", a, b, at(a)); else printf(":) %04X == %04X\n", a, b)

	t(0x0, 0x0);
	t(0xfff, 0xfff);
	t(0x1000, 0x1000);
	t(0x1fff, 0x1fff);
	t(0x2000, 0xffff); // fix
	t(0x2fff, 0x27ff);
	t(0x3000, 0x2800);
	t(0x3fff, 0x37ff);
	t(0x4000, 0xffff);
	t(0x4400, 0xffff);
	t(0x4800, 0xffff);
	t(0x4c00, 0xffff);
	t(0x43ff, 0xffff);
	t(0x47ff, 0xffff);
	t(0x4bff, 0xffff);
	t(0x4fff, 0xffff);
}

#if 0
void key(uint8_t k) {
}
#endif

static uint8_t key_lut[] = {
	0x10, /* GKEY_A */
	0x20, /* GKEY_B */
	0x30, /* GKEY_C */
	0x40, /* GKEY_D */
	0x50, /* GKEY_E */
	0x60, /* GKEY_F */
	0x70, /* GKEY_G */
	0x01, /* GKEY_H */
	0x11, /* GKEY_I */
	0x21, /* GKEY_J */
	0x31, /* GKEY_K */
	0x41, /* GKEY_L */
	0x51, /* GKEY_M */
	0x61, /* GKEY_N */
	0x71, /* GKEY_O */
	0x02, /* GKEY_P */
	0x12, /* GKEY_Q */
	0x22, /* GKEY_R */
	0x32, /* GKEY_S */
	0x42, /* GKEY_T */
	0x52, /* GKEY_U */
	0x62, /* GKEY_V */
	0x72, /* GKEY_W */
	0x03, /* GKEY_X */
	0x13, /* GKEY_Y */
	0x23, /* GKEY_Z */
	0x73, /* GKEY_SPACE */
	0x04, /* GKEY_0 */
	0x14, /* GKEY_1 */
	0x24, /* GKEY_2 */
	0x34, /* GKEY_3 */
	0x44, /* GKEY_4 */
	0x54, /* GKEY_5 */
	0x64, /* GKEY_6 */
	0x74, /* GKEY_7 */
	0x05, /* GKEY_8 */
	0x15, /* GKEY_9 */
	0x25, /* GKEY_SEMI */
	0x35, /* GKEY_COLON */
	0x45, /* GKEY_COMMA */
	0x55, /* GKEY_EQU */
	0x65, /* GKEY_DOT */
//	0x65, /* GKEY_COMMA */
	0x06, /* GKEY_RETURN */
	0x56, /* GKEY_LSHIFT */
	0x56, /* GKEY_RSHIFT */
	0x75, /* GKEY_SLASH1 */
	0x33, /* GKEY_UP */
	0x43, /* GKEY_DOWN */
	0x53, /* GKEY_LEFT */
	0x63, /* GKEY_RIGHT */
	0x75, /* GKEY_SLASH2 */
	0x16, /* GKEY_BREAK */
	0x26, /* GKEY_REPEAT */
	0x36, /* GKEY_DELETE */
	0x46  /* GKEY_LIST */
};

void machine_ProcessKey(uint8_t key, int pressed) {
	if (key >= KEY_ENDSTOP) return;
	
	uint8_t scancode = key_lut[key];
	printf("processKey: scancode %02x pressed %d\n", scancode, pressed);
	if (pressed) {
		keymatrix[(scancode>>4)&0x7] |= (1<<(scancode&7));
	} else {
		keymatrix[(scancode>>4)&0x7] &= ~(1<<(scancode&7));
	}
}

#ifndef TESTS

void machine_Init() {
  memset(mem, 0x00, sizeof mem);
  Z80_Reset();

//  Z80_Trace = 0;
//   Z80_Trap = 0x0000;

	Z80_IPeriod = 62500;
}

void machine_Poll() {
// 	Z80_Trace = debug;
	Z80_Execute();
  tstates += Z80_IPeriod;
}

void machine_Kill() {
  Z80_RegisterDump();

  unsigned long realTstates = tstates + Z80_IPeriod - Z80_ICount;

  printf("Terminated after %lu tstates (%lu seconds)\n", realTstates, realTstates/3500000);

}
#endif

// OUT (255),X - ends the vsync - signal for new frame
//
//;; DISP-2
	//L01AD:  LD      C,(IY+$23)      ;; load C the col count from REntLT_hi.
//
//        LD      R,A             ;; R increments with each opcode until A6
//                                ;; goes low which generates the INT signal.
// int generation is from A6 line!!! - which is generated on refresh.


#if 0
void updateScreenGALXdirect() {
  int x, y;
  uint8_t b, c, inv = 0, m;

	uint16_t dfilepos = at(mem[at(0x400c)] | (mem[at(0x400d)] << 8));
	uint16_t dfea = at(mem[at(0x400e)] | (mem[at(0x400f)] << 8));
	uint16_t dfend = at(mem[at(0x4010)] | (mem[at(0x4011)] << 8));
	uint8_t dfnlower = mem[at(0x4012)];

	printf("dfilepos:%04X dfend:%04X dfea:%04X dfnlower:%02X\n", dfilepos, dfend, dfea, dfnlower);

  for (int j=0; j<192; j++) {
  	uint16_t thispos = dfilepos;
//		if ((j&7) == 0) printf("line %d: dpos %04X\n", j>>3, dfilepos);
    for (int i=-1; i<256; i++) {
      if ((i & 7) == 7) {
      	c = mem[thispos];
//      	if (thispos >= dfend && !dfnlower && c != 0x76) {
      	if (thispos >= dfend || c != HALT) {
      		m = mem[0xe00 + (c & 0x3f) * 8 + (j&7)];
      		inv = c & 0x80;
      		thispos++;
      	} else {
      		m = 0x00;
      		inv = 0;
      	}
      }
      putpixel(i, j, (inv ^ (m & 0x80)) ? 0 : 0xd7d7d7);
      m <<= 1;
    }
    if ((j&7) == 7) dfilepos = thispos + 1;
  }
}
#endif

#if 0
static uint8_t screen[2][24][32];
uint8_t currScreen = 0;
uint16_t updx = 0, updy = 0;

void reset() {
	printf("reset\n");
	updy = updx = 0;
	currScreen ^= 1;
}
#endif

#if 0
void update(uint8_t d) {
	printf("update(%02X)\n", d);
	if (d == HALT) {
		if (updy >= TOP && updy < BOTTOM) {
			int y = (updy-TOP)/8;
			for (int i=updx; i<32; i++) {
				screen[currScreen^1][y][i] = 0x00;
			}
		}
		updx = 0;
		updy ++;
	} else {
		if (updy >= TOP && updy < BOTTOM) {
			screen[currScreen^1][(updy-TOP)/8][updx] = d;
			updx ++;
		}
	}
}
#endif

#if 0
void updateScreenGALX() {
  int x, y;
  uint8_t b, c, inv = 0, m;

  for (int j=0; j<192; j++) {
    for (int i=0; i<256; i++) {
      if ((i & 7) == 0) {
      	c = screen[currScreen][j/8][i/8];
				m = mem[FONT_POS + (c & 0x3f) * 8 + (j&7)];
				inv = c & 0x80;
      }
      putpixel(i, j, (inv ^ (m & 0x80)) ? 0 : 0xd7d7d7);
      m <<= 1;
    }
  }
}
#endif

#ifndef TESTS
#define CHARHEIGHT	13
#define SCREEN_AT		0x0000

//CHROM_A <= LATCH_DATA(3 downto 0) & TMP(7) & TMP(5 downto 0);
void machine_UpdateScreen() {
  int x, y;
  uint8_t b, c, inv = 0, m;
  uint16_t pos;

  x = y = 0;

  for (int j=0; j<208; j++) {
    pos = (j / CHARHEIGHT) * 32 + SCREEN_AT;
    for (int i=0; i<256; i++) {
      if ((i & 7) == 0) {
				b = (mem[pos] & 0x3f)|((mem[pos]&0x80)>>1);
        m = charrom[b + 128*(j%CHARHEIGHT)];
        pos ++;
      }
      hw_PutPixel(i, j, (inv ^ (m & 0x80)) ? 0 : 1);
      m <<= 1;
    }
    y = (y + 1) % 8;
  }
}

#endif

#ifdef TESTS
int main(int argc, char **argv) {
  tests();
  return 0;
}
#endif


