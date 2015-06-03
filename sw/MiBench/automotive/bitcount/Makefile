FILE = bitcnt_1.c bitcnt_2.c bitcnt_3.c bitcnt_4.c bitcnts.c bitfiles.c bitstrng.c bstr_i.c 

bitcnts: ${FILE} Makefile
	/home/dominic/ti/ccsv6/tools/compiler/gcc_msp430_4.9.14r1_167/bin/msp430-elf-gcc -O3 -specs=archc -mmcu=msp430 -static ${FILE} -o bitcnts

clean:
	rm -rf bitcnts output*
