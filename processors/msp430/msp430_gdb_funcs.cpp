/**
 * @file   msp430_gdb_funcs.cpp
 * @author Rogerio Paludo <rogerio.pld@gmail.com>
 * @date   Thu Feb  5 10:23:17 2015
 * 
 * @brief  In this file the GDB methods are implemented. 
 * They are inherited from ...
 * 
 * 
 */

#include "msp430.H"

#include "ac_debug_model.H"

using namespace msp430_parms;

/** 
 * This method return the number of registers in msp430::RB
 * 
 * 
 * @return a int, number of registers 
 */
int msp430::nRegs(void) {
  dbg_printf("GDB-----------------\n");
  dbg_printf("Returning nRegs");
  return 16;
}

/** 
 * This method return the value of a register
 * 
 * @param reg the register to be read
 * 
 * @return the value of a register
 */
ac_word msp430::reg_read(int reg) {
  dbg_printf("GDB-----------------\n");
  dbg_printf("Returning a REG value: 0x%X", RB.read(reg));
  return RB.read( reg );
}

/** 
 * This method writes a value into a specific register
 * 
 * @param reg the register to be write
 * @param value the value
 */
void msp430::reg_write(int reg, ac_word value) {
  dbg_printf("GDB-----------------\n");
  dbg_printf("Writing a REG value: 0x%X reg: 0x%X", value, reg);
  RB.write(reg, value);
}

/** 
 * To read a value from a memory location
 * 
 * @param address the address to be read
 * 
 * @return the read value
 */
unsigned char msp430::mem_read(unsigned int address) {
  dbg_printf("GDB-----------------\n");
  dbg_printf("Returning a MEM value: 0x%X address: 0x%X", IM->read_byte(address), address);
  return IM->read_byte(address);
}

/** 
 * This method writes a value into a memory location
 * 
 * @param address the address to be write in
 * @param byte the value to be write
 */
void msp430::mem_write(unsigned int address, unsigned char byte) {
  IM->write_byte(address, byte);
  dbg_printf("GDB-----------------\n");
  dbg_printf("Writing a MEM value: 0x%X address: 0x%X", byte, address);
}
