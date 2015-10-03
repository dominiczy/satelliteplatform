/**
 * @file   msp430_syscall.cpp
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements the passing of arguments to system calls
 * 
 */

 /// Program Counter register
#define PC 0 
/// Stack Pointer register
#define SP 1 
/// Status register
#define SR 2

#include "msp430_syscall.H"

// 'using namespace' statement to allow access to all
// msp430-specific datatypes

//copied from mips, adjust according to msp430 ABI page 18
//see also Mixing C and Assembler With the MSP430 table 1

using namespace msp430_parms;
unsigned procNumber = 0;

void msp430_syscall::get_buffer(int argn, unsigned char* buf, unsigned int size)
{
// if size is small
  unsigned int addr =RB.read(12+argn);// size big DM.read(RB.read(SP)); //compiler assigns other value to r12 before calling open!!
  //cout << "addr "<< addr << " r12 " <<RB.read(12)<<endl;
  for (unsigned int i = 0; i<size; i++, addr++) {
    buf[i] = DM.read_byte(addr);
  }
}

void msp430_syscall::set_buffer(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB.read(12+argn);

  for (unsigned int i = 0; i<size; i++, addr++) {
    DM.write_byte(addr, buf[i]);
  }
}

void msp430_syscall::set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB.read(12+argn);

  for (unsigned int i = 0; i<size; i+=4, addr+=4) {
    DM.write(addr, *(unsigned int *) &buf[i]);
  }
}

int msp430_syscall::get_int(int argn)
{
  //return RB.read(15-argn);
	return RB.read(12+argn);
}

void msp430_syscall::set_int(int argn, int val)
{
  RB.write(12+argn,val);
}

void msp430_syscall::return_from_syscall() //RET instruction? see ABI
{
  RB.write(PC, DM.read(RB.read(SP)));   // PC restored, +4?? see mips, sparc
  ac_pc = DM.read(RB.read(SP));
  RB.write(SP, RB.read(SP) + 2);   // SP incremented
}

void msp430_syscall::set_prog_args(int argc, char **argv) // presumably error here, cpu 3 pc 0x0000, call in msp430.cpp cause
{
  int i, j, base;

  unsigned int ac_argv[30];
  char ac_argstr[512];

  base = AC_RAM_END - 512 - procNumber * 64 * 1024; // change here? implementation for multi cpu!
  for (i=0, j=0; i<argc; i++) {
    int len = strlen(argv[i]) + 1;
    ac_argv[i] = base + j;
    memcpy(&ac_argstr[j], argv[i], len);
    j += len;
  }

  RB.write(12, base);
  set_buffer(0, (unsigned char*) ac_argstr, 512);   //$25 = $29(sp) - 4 (set_buffer adds 4)


  RB.write(12, base - 120 );
  set_buffer_noinvert(0, (unsigned char*) ac_argv, 120);

  //RB[4] = AC_RAM_END-512-128;

  //Set %o0 to the argument count
  RB.write(12,argc);

  //Set %o1 to the string pointers
  RB.write(13,base - 120);

  procNumber ++;
}



