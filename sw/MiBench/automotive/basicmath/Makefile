FILE1 = basicmath_small.c   rad2deg.c  cubic.c   isqrt.c
FILE2 = basicmath_large.c   rad2deg.c  cubic.c   isqrt.c
FILE3 = basicmath_tiny.c   rad2deg.c  cubic.c   isqrt.c

all: basicmath_small

basicmath_small: ${FILE1} Makefile
	/home/dominic/ti/ccsv6/tools/compiler/gcc_msp430_4.9.14r1_167/bin/msp430-elf-gcc -specs=archc -mmcu=msp430 -static -O0 ${FILE1} -o basicmath_small.x -lm
basicmath_large: ${FILE2} Makefile
	/home/dominic/ti/ccsv6/tools/compiler/gcc_msp430_4.9.14r1_167/bin/msp430-elf-gcc -specs=archc -static -O3 ${FILE2} -o basicmath_large -lm
basicmath_tiny: ${FILE3} Makefile
	/home/dominic/ti/ccsv6/tools/compiler/gcc_msp430_4.9.14r1_167/bin/msp430-elf-gcc -specs=archc -mmcu=msp430 -static -O0 ${FILE3} -o basicmath_tiny.x -lm

clean:
	rm -rf basicmath_small basicmath_large basicmath_tiny output*
