#!/bin/bash
od -v -t x1 -w1 $1.bin | awk '{ printf "0x%s, ", $2; if ((NR%16)==0) printf("\n"); }' > $1.h

