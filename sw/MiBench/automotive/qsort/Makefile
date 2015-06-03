FILE1 = qsort_small.c
FILE2 = qsort_large.c

all: qsort_small qsort_large

qsort_small: qsort_small.c Makefile
	/home/dominic/ti/ccsv6/tools/compiler/gcc_msp430_4.9.14r1_167/bin/msp430-elf-gcc -specs=archc -static qsort_small.c -O3 -o qsort_small -lm

clean:
	rm -rf qsort_small qsort_large output*
