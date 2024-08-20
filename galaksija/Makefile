#### Makefile for Z80Em
# Following #defines can be used:
# DEBUG			Compile with builtin debugger
# LSB_FIRST		Compile for a low-endian machine
# X86_ASM		Use inline 80x86 assembly (GCC only)

CC	= gcc	# C compiler used
LD	= gcc -g	# Linker used

#CFLAGS	= -Wall -O2 -fomit-frame-pointer -DLSB_FIRST -DDEBUG
CFLAGS	= -g -Wall -DLSB_FIRST -DDEBUG -D__64BIT__

all:    Z80.o Z80Debug.o z80dasm galaksija

%.o : %.c
	gcc ${CFLAGS} -c -o $@ $<

Z80Dasm.o:  Z80Dasm.c
hwpc.o: hwpc.c hardware.h machine.h
z80dasm:    Z80Dasm.o
	$(LD) -s -o z80dasm Z80Dasm.o

Z80Debug.o: Z80Debug.c Z80.h Z80IO.h Z80Dasm.h
Z80.o:      Z80.c Z80.h Z80Codes.h Z80IO.h Z80DAA.h

galaksija: galaksija.o Z80.o hwpc.o Z80Debug.o
	${LD} -g -s -o $@ $^ -lSDL

zx80: zx80.o Z80.o screen.o Z80Debug.o
	${LD} -g -s -o $@ $^ -lSDL

zx81: zx81.o Z80.o screen.o Z80Debug.o
	${LD} -g -s -o $@ $^ -lSDL

jupiterace.o: jupiterace.c
galaksija.o: galaksija.c machine.h hardware.h
zx80.o: zx80.c
zx81.o: zx81.c zx80.c screen.h

runz80.o: runz80.c

tap2raw.o: tap2raw.c

tap2raw: tap2raw.o
	${CC} -o$@ $<

tapex.o: tapex.c

tapex: tapex.o
	${CC} -o$@ $<
test:
	./runz80 run ../roms/ZXS48.ROM ../bios/bootloader.bin 32768 ../data/sdcard.xxx
	od -t x1 specram.bin

gal:
	./galaksija emu ./grom/roma_b.bin


galtests:
	gcc -DTESTS -ogaltests1 galaksija.c
	./galtests1

listen:
	play -b8 -esigned-integer -r48000 -c1 h.raw

clean:
	rm *.o z80dasm runz80 mkfs galaksija

mkfs: mkfs.c
	gcc -g -o $@ $<


