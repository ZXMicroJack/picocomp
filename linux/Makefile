#### Makefile for Z80Em
# Following #defines can be used:
# DEBUG			Compile with builtin debugger
# LSB_FIRST		Compile for a low-endian machine
# X86_ASM		Use inline 80x86 assembly (GCC only)

z80=../cpu-z80
galaksija=../galaksija
inc=../inc
menu=../menu
fs=../fs

CC	= gcc	# C compiler used
LD	= gcc -g -m32	# Linker used

#CFLAGS	= -Wall -O2 -fomit-frame-pointer -DLSB_FIRST -DDEBUG
CFLAGS	= -g -m32 -Wall -DLSB_FIRST -DDEBUG -D__64BIT__ -I../inc -I${z80} -I${galaksija} -I${menu} -I${fs}

all:    Z80.o Z80Debug.o z80dasm galaksija

%.o : ${galaksija}/%.c
	gcc ${CFLAGS} -c -o $@ $<

%.o : ${fs}/%.c
	gcc ${CFLAGS} -c -o $@ $<

%.o : ${menu}/%.c
	gcc ${CFLAGS} -c -o $@ $<

%.o : ${z80}/%.c
	gcc ${CFLAGS} -c -o $@ $<

%.o : %.c
	gcc ${CFLAGS} -c -o $@ $<

Z80Dasm.o:  ${z80}/Z80Dasm.c
hwpc.o: hwpc.c ${inc}/hardware.h ${inc}/machine.h
menu.o: ${menu}/menu.c
fat32.o: ${fs}/fat32.c ${fs}/fat32.h
z80dasm:    Z80Dasm.o
	$(LD) -s -o z80dasm Z80Dasm.o

Z80Debug.o: ${z80}/Z80Debug.c ${z80}/Z80.h ${z80}/Z80IO.h ${z80}/Z80Dasm.h
Z80.o:      ${z80}/Z80.c ${z80}/Z80.h ${z80}/Z80Codes.h ${z80}/Z80IO.h ${z80}/Z80DAA.h

#fat32.o
galaksija: galaksija.o Z80.o hwpc.o Z80Debug.o menu.o 
	${LD} -g -s -o $@ $^ -lSDL

zx80: zx80.o Z80.o hwpc.o Z80Debug.o
	${LD} -g -s -o $@ $^ -lSDL

zx81: zx81.o Z80.o hwpc.o Z80Debug.o
	${LD} -g -s -o $@ $^ -lSDL

jupiterace: jupiterace.o Z80.o hwpc.o Z80Debug.o
	${LD} -g -s -o $@ $^ -lSDL

jupiterace.o: jupiterace.c ${inc}/machine.h ${inc}/hardware.h
galaksija.o: ${galaksija}/galaksija.c ${inc}/machine.h ${inc}/hardware.h ${menu}/menu.c
zx80.o: zx80.c ${inc}/machine.h ${inc}/hardware.h
zx81.o: zx81.c ${inc}/machine.h ${inc}/hardware.h

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


