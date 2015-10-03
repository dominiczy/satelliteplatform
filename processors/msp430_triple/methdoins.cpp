
/**
 * @file   methdoins.cpp
 * @author Rogerio Paludo <rogerio.pld@gmail.com>
 * @date   Fri Jan 30 15:44:31 2015
 * 
 * @brief  In this file all the instructions are implemented 
 * 
 * @todo tests
 */

//****************************************************
//  Auxiliary Methods
//****************************************************

/**  	
 * This function writes back the value into registe bank 
 * or memory depending on the addressing mode used
 * 
 * @param sreg source register
 * @param bw byte or word operation
 * @param dreg destination register
 * @param oset offset field (directly an address source/constant)
 * @param odst offset destination, also an address
 * @param DM data memory
 * @param RB register bank
 * @param value value to be write
 * 
 * Example of use:
 * @code 
 *     write_back(sreg, bw, dreg, oset, odst, DM, RB, sum_b);
 * @endcode
 */
inline void write_back(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		       uint16_t oset, uint16_t odst, 
		       ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		       ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		       int16_t value) {
  /// @note The addressing modes are grouped by functionality 
  switch (mode) {
    // The result is saved into a register
  case _16rm:
  case _16irm:
  case _32isrc:
  case _32imm:    
    RB.write(dreg, value);
    break;
    // The result is saved into a register and the source is incremented
  case _16iaim:
    RB.write(dreg, value);
    if (constant.mode) {
      constant.mode = false;
      break;
    }
    else {
      if (bw)
	RB.write(sreg, 0x00ff & (RB.read(sreg) + 1));
      else
	RB.write(sreg, (RB.read(sreg) + 2));
    }
    break;
    // The result is saved into memory and the source is incremented 
  case _32iaim:    
    if (bw) {
      DM.write_byte(oset, value);
      RB.write(sreg, 0x00ff & (RB.read(sreg) + 1));
    }
    else {
      DM.write(oset, value);
      RB.write(sreg, (RB.read(sreg) + 2));
    }
    break;
    // Into a memory address
  case _32idst:
  case _32irm:
    if (bw) 
      DM.write_byte(oset, value);
    else 
      DM.write(oset, value);
    break;
    // From a memory address or constant to a memory address   
  case _48imm:
  case _48idm:
    if (bw) 
      DM.write_byte(odst, value);
    else 
      DM.write(odst, value);
    break;
  }  
}

/** 
 * This function implements a binary-coded decimal addition with carry
 * 
 * @param X BCD1
 * @param Y BCD2
 * @param C_ carry in
 * 
 * @return BCD1 + BCD2 + carry
 * Example of use:
 * @code 
 *     bcd_add((0xf & val1), (0xf & val2), (0x1 & flags.C));
 * @endcode
 */
uint8_t bcd_add(uint8_t X, uint8_t Y, uint8_t C_) {	
  // temp
  uint8_t Z_ = 0;
  // carry in
  C_ = (C_ > 0) ? 1 : 0;
  // addition
  Z_ = (0x1f & X) + (0x1f & Y) + (0x1 & C_);
  // return values
  if (Z_ < 10)
    return (0x1f & Z_);	
  else 
    return (0x1f & Z_) + (0x1f & 6);
}


//****************************************************
// Behavior Methods
//****************************************************

//**********************MOV****************************
/** 
 * This function implements the MOV instruction considering 
 * all addressing modes possible, and also pseudo-instructions
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param as addressing bits responsible for the addressing mode
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param ac_pc program counter (used for pseudo intructions)
 */
inline void MOV(unsigned int sreg, unsigned int bw, unsigned int as, unsigned int dreg, 
		uint16_t oset, uint16_t osrc, uint16_t odst,
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB,
		ac_reg<unsigned>& ac_pc) {
  
  switch (mode) {
    //********
  case _16rm:    
    if (constant.mode) {
      if (bw) {
	RB.write(dreg, 0xff & constant.value);
	constant.mode = false;
      }
      else {
	RB.write(dreg, constant.value); 
	constant.mode = false;
      }
    }
    else {
      if (bw)     
	RB.write(dreg, 0xff & RB.read(sreg));
      else 
	RB.write(dreg, RB.read(sreg));
    }
    break;
    //********
  case _16irm:  
    if (constant.mode) {
      if (bw) {
	RB.write(dreg, 0xff & constant.value);
	constant.mode = false;
      }
      else {
	RB.write(dreg, constant.value); 
	constant.mode = false;
      }
    } 
    else { 
      if (bw)
	RB.write(dreg, 0xff & DM.read(RB.read(sreg)));
      else 
	RB.write(dreg, DM.read(RB.read(sreg)));
    }
    break;
    //********
  case _16iaim:     
    if (constant.mode) {
      if (bw) {
	// constant mode && byte operation 
	RB.write(dreg, 0xff & constant.value);
	constant.mode = false; 
      }
      else {
	// constant mode && word operation
	RB.write(dreg, constant.value);
	constant.mode = false; 
      }
    } 
    else {
      if (bw) {
	// normal mode && byte operation
	RB.write(dreg, 0xff & DM.read(RB.read(sreg)));
	RB.write(sreg, 0x00ff & (RB.read(sreg) + 1));
      }
      else {
	// normal mode && word operation
       	RB.write(dreg, DM.read(RB.read(sreg)));
	RB.write(sreg, (RB.read(sreg) + 2));
      }
    }
    break;   
    //********
  case _32idst: 
    if (constant.mode) {
      if (bw) {
	DM.write_byte(oset, constant.value);
	constant.mode = false;
      }
      else {
	DM.write(oset, constant.value); 
	constant.mode = false;
      }
    } 
    else { 
      if (bw) {
	DM.write_byte(oset, RB.read(sreg));
	dbg_printf("value: 0x0%X\n", uint8_t(constant.value));
      }
      else 
	DM.write(oset, RB.read(sreg));
    }
    break;
   
    //********
  case _32isrc:
    if (constant.mode) {
      if (bw) {
	// constant mode && byte operation 
	RB.write(dreg, 0xff & constant.value);
	constant.mode = false; 
      }
      else {
	// constant mode && word operation
	RB.write(dreg, constant.value);
	constant.mode = false; 
      }
    } 
    else {
      if (bw) {
	// normal mode && byte operation
	RB.write(dreg, 0xff & DM.read(oset));
      }
      else {
	// normal mode && word operation
	RB.write(dreg, DM.read(oset));
      }
    }
    break;
    //***********
  case _32imm:
    if (bw)
      RB.write(dreg, 0xff & oset);
    else 
      RB.write(dreg, oset);
    break;
    //********
  case _32iaim:
    dbg_printf("oset: 0x0%X\n", oset);
    dbg_printf("constant.value: 0x0%X\n", 0x00ff & constant.value);	
    if (constant.mode) {
      if (bw) {
	// constant mode && byte operation 
	DM.write_byte(oset, constant.value);	
	constant.mode = false; 
      }
      else {
	// constant mode && word operation
	DM.write(oset, constant.value);
	constant.mode = false; 
      }
    } 
    else {
      if (bw) {
	// normal mode && byte operation
	DM.write_byte(oset, DM.read(RB.read(sreg)));
	RB.write(sreg, 0x00ff & (RB.read(sreg) + 1));	
      }
      else {
	// normal mode && word operation
	DM.write(oset, DM.read(RB.read(sreg)));
	RB.write(sreg, (RB.read(sreg) + 2));
      }
    }
    break;
    //********
  case _32irm:  
    if (constant.mode) {
      if (bw) {
	// constant mode && byte operation 
	DM.write_byte(oset, constant.value);
	constant.mode = false; 
      }
      else {
	// constant mode && word operation
	DM.write(oset, constant.value);
	constant.mode = false; 
      }
    } 
    else {
      if (bw) {
	// normal mode && byte operation
	DM.write_byte(oset, DM.read(RB.read(sreg)));
      }
      else {
	// normal mode && word operation
	DM.write(oset, DM.read(RB.read(sreg)));	
      }
    }
    break;
    //********
  case _48idm:      
    if (constant.mode) {
      if (bw) {
	DM.write_byte(odst, constant.value);
	constant.mode = false; 
      }
      else {
	DM.write(odst, constant.value);
	constant.mode = false; 
      }
    } 
    else {
      if (bw) {
	// normal mode && byte operation
	DM.write_byte(odst, DM.read_byte(osrc));
      }
      else {
	// normal mode && word operation
	DM.write(odst, DM.read(osrc)); 
      }
    }
    break;
    //*********
  case _48imm:
    //behavior
    if (bw)
      DM.write_byte(odst, osrc);
    else
      DM.write(odst, osrc);
  }
  // Branch?	
  if (dreg == PC & (as != 1))
    ac_pc = RB.read(PC);
}  

//**********************ADD****************************
/** 
 * This function implements the ADD instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void ADD(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		uint16_t oset, uint16_t osrc, uint16_t odst,
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		uint16_t val1, uint16_t val2) {
  
  //  Variables
  int32_t sum = 0;
  int16_t res = 0;
	
  // Constant MOde
  if (constant.mode) { 
    val1 = constant.value;
  }

  dbg_printf("val1: 0x%X \n", val1);
  dbg_printf("val2: 0x%X \n", val2);
  
  // Behavior
  if (bw) {
    sum = (0xff & val1) + (0xff & val2);
    res = 0xff & sum;
    dbg_printf("sum: 0x%X \n", sum);
    dbg_printf("res: 0x%X \n", res);
  }
  else {
    sum = val1 + val2;
    res = 0xffff & sum;
    dbg_printf("sum: 0x%X \n", sum);
    dbg_printf("res: 0x%X \n", res);
  }

  // Flags Generation
  flags.N = bw ? getBit(sum, 7) : getBit(sum, 15);
  flags.Z = ((res == 0) ? true : false);
  flags.C = bw ? getBit(sum, 8) : getBit(sum, 16);      

  flags.V = bw ? ((getBit(val1, 7) & getBit(val2, 7) & ~getBit(sum, 7))    |
                 (~getBit(val1, 7) & ~getBit(val2, 7) & getBit(sum, 7)))   : 
                 ((getBit(val1, 15) & getBit(val2, 15) & ~getBit(sum, 15)) |
                  ~getBit(val1, 15) & ~getBit(val2, 15) & getBit(sum, 15));

  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));

  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(sreg, bw, dreg, oset, odst, DM, RB, res);	
  constant.mode = false;
}

//**********************ADDC****************************
/** 
 * This function implements the ADDC (ADD with carry) instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void ADDC(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		 uint16_t oset, uint16_t osrc, uint16_t odst,
		 ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		 ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		 int val1, int val2) {

  // Variables
  int32_t sum = 0;
  int16_t res = 0;

  // Constant Mode
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  }
	
  // Behavior
  if (bw) {
    sum =  (uint8_t)(val1) + (uint8_t)(val2) + (uint8_t)(0x1 & flags.C);
    res = 0xff & sum;
    //debug
    dbg_printf("sum: 0x%X \n", sum);
    dbg_printf("res: 0x%X \n", res);
  }
  else {
    sum =  (uint16_t)(val1) + (uint16_t)(val2) + (uint16_t)(0x1 & flags.C);
    res = 0xffff & sum;
    // debug
    dbg_printf("sum: 0x%X \n", sum);
    dbg_printf("res: 0x%X \n", res);
  }

  // Flags Generation  
  flags.N = bw ? getBit(sum, 7) : getBit(sum, 15);
  flags.Z = ((res == 0) ? true : false);
  flags.C = bw ? getBit(sum, 8) : getBit(sum, 16);
  flags.V = bw ?   ((getBit(val1, 7) & getBit(val2, 7) & ~getBit(sum, 7))    |
		    (~getBit(val1, 7) & ~getBit(val2, 7) & getBit(sum, 7)))   : 
    ((getBit(val1, 15) & getBit(val2, 15) & ~getBit(sum, 15)) |
     ~getBit(val1, 15) & ~getBit(val2, 15) & getBit(sum, 15));
  
  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));
  
  //debug
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(sreg, bw, dreg, oset, odst, DM, RB, res);	
}

//**********************AND****************************
/** 
 * This function implements the AND instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void AND(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		uint16_t oset, uint16_t osrc, uint16_t odst,
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		int val1, int val2) {

  // Variables 
  int16_t res = 0;

  // Constant mode
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  }
	
  // Behavior	
  if (bw) {
    res = 0xff & (val1 & val2);
  }
  else {
    res = val1 & val2;
  }

  // Flags Generation  
  flags.N = bw ? getBit(res, 7) : getBit(res, 15);
  flags.Z = ((res == 0) ? true : false);
  flags.C = !flags.Z;
  flags.V = 0;

  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));  

  // Debug
  dbg_printf("and 0x%X \n", res);
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(sreg, bw, dreg, oset, odst, DM, RB, res);	
}

//**********************BIC****************************
/** 
 * This function implements the BIC instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void BIC(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		uint16_t oset, uint16_t osrc, uint16_t odst,
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		int val1, int val2) {

  // Variables 
  int16_t res = 0;

  // Constant mode
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  }	
  
  dbg_printf("val1: 0x%hx\n", val1);
  dbg_printf("val2: 0x%hx\n", val2);
  
  // Behavior	(not(val1) and val2)
  if (bw) {
    res = 0xff & (~val1 & val2);
  }
  else {
    res = ~val1 & val2;
  }
  // Flags Generation  not affected	
  // Debug
  dbg_printf("bic 0x%X \n", res);
  // Writing back values
  write_back(sreg, bw, dreg, oset, odst, DM, RB, res);	
}

//**********************BIS****************************
/** 
 * This function implements the BIS instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void BIS(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		uint16_t oset, uint16_t osrc, uint16_t odst,
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		int val1, int val2) {

  // Variables 
  int16_t res = 0;

  // Constant mode
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  }	
  // Behavior	(val1 OR val2)
  if (bw) {
    res = 0xff & (val1 | val2);
  }
  else {
    res = val1 | val2;
  }
  // Flags Generation  not affected	
  // Debug
  dbg_printf("bis 0x%X \n", res);
  // Writing back values
  write_back(sreg, bw, dreg, oset, odst, DM, RB, res);	
}

//**********************BIT****************************
/** 
 * This function implements the BIT instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void BIT(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		uint16_t oset, uint16_t osrc, uint16_t odst,
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		int val1, int val2) {

  // Variables 
  int16_t res = 0;

  // Constant mode
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  }	
  // Behavior	(val1 OR val2)
  if (bw) {
    res = 0xff & (val1 & val2);
  }
  else {
    res = val1 & val2;
  }
	
  // Flags Generation  
  flags.N = bw ? getBit(res, 7) : getBit(res, 15);
  flags.Z = ((res == 0) ? true : false);
  flags.C = !flags.Z;
  flags.V = 0;
  
  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));
  
  // Debug
  dbg_printf("bit 0x%X \n", res);
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);
  
  // There is no write back
  // Only autoindexed increment mode 
  // autoincrement source register
  switch (mode) {
  case _16iaim:
    if (bw)
      RB.write(sreg, 0x00ff & (RB.read(sreg) + 1));
    else
      RB.write(sreg, (RB.read(sreg) + 2));
    break;
  case _32iaim:
    if (bw)
      RB.write(sreg, 0x00ff & (RB.read(sreg) + 1));
    else
      RB.write(sreg, (RB.read(sreg) + 2));
    break;
  }	
}

//**********************CMP****************************
/** 
 * This function implements the CMP instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void CMP(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		uint16_t oset, uint16_t osrc, uint16_t odst,
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		uint16_t val1, uint16_t val2) {

  int sum = 0;
  int16_t res = 0;  
	
  // Constant MOde
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  }
 // dbg_printf("val1: 0x%hx\n", val1);
  //dbg_printf("val1_dec: %d\n", val1);
  //dbg_printf("val2: 0x%hx\n", val2);
  // Behavior
  if (bw) {    
    sum = (uint8_t)(~val1) + 1 + (uint8_t)(val2); 
    val1 = (uint8_t)(~val1) + 1;
    res = 0xff & sum;
  }
  else {     
    sum = (uint16_t)(~val1) + 1 + (uint16_t)(val2); 
    val1 = (~val1) + 1;
    res = 0xffff & sum;
  }

  // Flags Generation  
  flags.N = bw ? getBit(sum, 7) : getBit(sum, 15);
  flags.Z = ((res == 0) ? true : false);
  flags.C = bw ? getBit(sum, 8) : getBit(sum, 16);
 // dbg_printf("val1: 0x%hx\n", val1);
 //	 dbg_printf("val1_dec: %d\n", val1);
  if (bw) {
    if (val1 != 0x80) {
      flags.V = (getBit(val1, 7) & getBit(val2, 7) & ~getBit(sum, 7)) |
	       (~getBit(val1, 7) & ~getBit(val2, 7) & getBit(sum, 7));  
    //  dbg_printf("Vbwif: %d\n", flags.V); 
    }
    else {
      flags.V = (1 & ~getBit(val2, 7) & getBit(sum, 7));
     // dbg_printf("Vbwelse: %d\n", flags.V); 
    }
  } else {
    if (val1 != 0x8000) {
      flags.V = (getBit(val1, 15) & getBit(val2, 15) & ~getBit(sum, 15)) |
	       (~getBit(val1, 15) & ~getBit(val2, 15) & getBit(sum, 15));    
  //    dbg_printf("Vif: %d\n", flags.V); 
    }
    else {
      flags.V = (1 & ~getBit(val2, 15) & getBit(sum, 15));
    //  dbg_printf("Velse: %d\n", flags.V); 
    }
  }

  RB.write(SR, 0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));

  // Debug
 // dbg_printf("cmp 0x%hX \n", res);
 // dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V); 

  // There is no write back
  // Only autoindexed increment mode 
  // autoincrement source register
  switch (mode) {
  case _16iaim:
    if (bw)
      RB.write(sreg, 0x00ff & (RB.read(sreg) + 1));
    else
      RB.write(sreg, (RB.read(sreg) + 2));
    break;
  case _32iaim:
    if (bw)
      RB.write(sreg, 0x00ff & (RB.read(sreg) + 1));
    else
      RB.write(sreg, (RB.read(sreg) + 2));
    break;
  }	
}

//**********************DADD****************************
/** 
 * This function implements the DADD instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void DADD(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		 uint16_t oset, uint16_t osrc, uint16_t odst,
		 ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		 ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		 int val1, int val2) {
	
  //variables
  uint16_t dadd0, dadd1, dadd2, dadd3;
  uint16_t res;

  // Constant Mode
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  }

  // Behavior
  if (bw) {
    dadd0 = bcd_add((0xf & val1), (0xf & val2), (0x1 & flags.C));
    dadd1 = bcd_add((0xf & (val1 >> 4)), (0xf & (val2 >> 4)), (0x10 & dadd0));
    res = (0xf & dadd1);
    res = (res << 4) | (0xf & dadd0);
  }
  else {
    dadd0 = bcd_add((0xf & val1), (0xf & val2), (0x1 & flags.C));
    dadd1 = bcd_add((0xf & (val1 >> 4)), (0xf & (val2 >> 4)), (0x10 & dadd0));
    dadd2 = bcd_add((0xf & (val1 >> 8)), (0xf & (val2 >> 8)), (0x10 & dadd1));
    dadd3 = bcd_add((0xf & (val1 >> 12)), (0xf & (val2 >> 12)), (0x10 & dadd2));
    res = (0xf & dadd3);
    res = (res << 4 ) | (0xf & dadd2);
    res = (res << 4 ) | (0xf & dadd1);
    res = (res << 4 ) | (0xf & dadd0);
  }	

  // Flags Generation  
  flags.N = bw ? getBit(res, 7) : getBit(res, 15);
  flags.Z = ((res == 0) ? true : false);
  if ((bw && (res > 99)) || (!bw && (res > 9999)) ||
      (bw && val2 == 0x99 && flags.Z) ||
      (!bw && val2 == 0x9999 && flags.Z))
    flags.C = true;
  else 
    flags.C = false;
  // flags.V = undefined;

  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));

  //debug
  dbg_printf("dadd 0x%X \n", res);
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(sreg, bw, dreg, oset, odst, DM, RB, res);	
}

//**********************SUB****************************
/** 
 * This function implements the SUB instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void SUB(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		uint16_t oset, uint16_t osrc, uint16_t odst,
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		uint16_t val1, uint16_t val2) {

  int sum = 0;
  int16_t res = 0;  
	
  // Constant MOde
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  }
  dbg_printf("val1: 0x%hx\n", val1);
  dbg_printf("val2: 0x%hx\n", val2);
  // Behavior
  if (bw) {    
    sum = (uint8_t)(~val1) + 1 + (uint8_t)(val2); 
    val1 = (~val1) + 1;
    res = 0xff & sum;
  }
  else {     
    sum = (uint16_t)(~val1) + 1 + (uint16_t)(val2); 
    val1 = (~val1) + 1;
    res = 0xffff & sum;
  }

  // Flags Generation  
  flags.N = bw ? getBit(sum, 7) : getBit(sum, 15);
  flags.Z = ((res == 0) ? true : false);
  flags.C = bw ? getBit(sum, 8) : getBit(sum, 16);
  
  if (bw) {
    if (val1 != 0x80) {
      flags.V = (getBit(val1, 7) & getBit(val2, 7) & ~getBit(sum, 7)) |
	       (~getBit(val1, 7) & ~getBit(val2, 7) & getBit(sum, 7));                 
    }
    else {
      flags.V = (1 & ~getBit(val2, 7) & getBit(sum, 7));
    }
  } else {
    if (val1 != 0x8000) {
      flags.V = (getBit(val1, 15) & getBit(val2, 15) & ~getBit(sum, 15)) |
	       (~getBit(val1, 15) & ~getBit(val2, 15) & getBit(sum, 15));                 
    }
    else {
      flags.V = (1 & ~getBit(val2, 15) & getBit(sum, 15));
    }
  }

  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));

  //debug
  dbg_printf("sub 0x%X \n", res);
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);
  
  // Writing back values
  write_back(sreg, bw, dreg, oset, odst, DM, RB, res);	
  
}

//**********************SUBC****************************
/** 
 * This function implements the SUBC instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void SUBC(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		 uint16_t oset, uint16_t osrc, uint16_t odst,
		 ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		 ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		 int16_t val1, int16_t val2) {  

   // Variables
  int32_t sum = 0;
  int16_t res = 0;

  // Constant Mode
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  } 

   // Behavior
  if (bw) {    
    sum = (uint8_t)(~val1) + (uint8_t)(val2) + (uint8_t)(0x1 & flags.C); 
    val1 = (uint8_t)(~val1);
    val2 = (uint8_t)(val2);  
    res = 0xff & sum;
  }
  else {     
    sum = (uint16_t)(~val1) + (uint16_t)(val2) + (uint16_t)(0x1 & flags.C); 
    val1 = (uint16_t)(~val1);
    val2 = (uint16_t)(val2); 
    res = 0xffff & sum;
  }

  // Flags Generation  
  flags.N = bw ? getBit(sum, 7) : getBit(sum, 15);
  flags.Z = ((res == 0) ? true : false);
  flags.C = bw ? getBit(sum, 8) : getBit(sum, 16);
  
  flags.V = bw ? ((getBit(val1, 7) & getBit(val2, 7) & ~getBit(sum, 7))    |
                 (~getBit(val1, 7) & ~getBit(val2, 7) & getBit(sum, 7)))   : 
                 ((getBit(val1, 15) & getBit(val2, 15) & ~getBit(sum, 15)) |
                  ~getBit(val1, 15) & ~getBit(val2, 15) & getBit(sum, 15));
  
  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));
  
  //debug
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(sreg, bw, dreg, oset, odst, DM, RB, res);	
}


//**********************XOR***************************
/** 
 * This function implements the SUB instruction
 * 
 * @param sreg source register
 * @param bw byte/word operation
 * @param dreg destination register
 * @param oset offset field used for 32 bits instructions
 * @param osrc offset source field 
 * @param odst offset destination field
 * @note offset is just a name for the address following the instruction word
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value 
 * @param val2 second value 
 */
inline void XOR(unsigned int sreg, unsigned int bw, unsigned int dreg, 
		uint16_t oset, uint16_t osrc, uint16_t odst,
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		int val1, int val2) {
  // variables
  int16_t res = 0;

  // Constant Mode
  if (constant.mode) { 
    val1 = constant.value;
    constant.mode = false;
  }
	
  // Behavior
  if (bw) {
    res = (0xff & (val1 ^ val2));
  }
  else {
    res = val1 ^ val2;
  }

  // Flags Generation  
  flags.N = bw ? getBit(res, 7) : getBit(res, 15);
  flags.Z = (res == 0) ? true : false;
  flags.C = !flags.Z;
  flags.V = bw ? (getBit(val1, 7) && getBit(val2, 7))   :
    (getBit(val1, 15) && getBit(val2, 15));		
  
  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));

  // debug
  dbg_printf("xor 0x%X \n", res);
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(sreg, bw, dreg, oset, odst, DM, RB, res);
}

//*****************************************************
// Single Operand Instructions - Behavior methods
//*****************************************************
/** 
 * This function implements the RRC instruction
 * 
 * @param bw byte/word operation
 * @param sreg destination register
 * @param oset  offset field used for 32 bits instructions
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value
 */
//**********************RRC****************************
inline void RRC(unsigned int bw, unsigned int sreg, 
		uint16_t oset, 
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		int16_t val1) {
  // variables
  int16_t res = 0;
  bool lsb;

  lsb = (1 & val1);
  
  dbg_printf("carry 0x%X \n", flags.C);  
  
  // Behavior
  if (bw) {
    val1 >>= 1;
    val1 = (((flags.C << 7) & 0x0080) | (0x7f & val1));
    res = 0xff & val1;
  }
  else {
    val1 >>= 1;
    val1 = (((flags.C << 15) & 0x8000) | (0x7fff & val1));
    res = 0xffff & val1;
  }
  
  // Flags Generation  
  flags.N = bw ? getBit(res, 7) : getBit(res, 15);
  flags.Z = (res == 0) ? true : false;
  flags.C = (lsb == 1) ? true : false;
  flags.V = false;

  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));

  // debug
  dbg_printf("rrc 0x%X \n", res);  
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(0, bw, sreg, oset, 0, DM, RB, res);
}
/** 
 * This function implements the SWPB instruction
 * 
 * @param bw byte/word operation
 * @param sreg destination register
 * @param oset  offset field used for 32 bits instructions
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value
 */
//**********************SWPB****************************
inline void SWPB(unsigned int bw, unsigned int sreg, 
		 uint16_t oset, 
		 ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		 ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		 int val1) {
  // variables
  int16_t res = 0;
  int8_t lobyte, hibyte;

  hibyte  = (0xff00 & val1) >> 8;
  lobyte = (0xff & val1);

  // Behavior
  res = (lobyte << 8) | hibyte;

  // Flags Generation -> Status bits are not affected 

  // debug
  dbg_printf("swpb 0x%X \n", res);
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(0, bw, sreg, oset, 0, DM, RB, res);
}
/** 
 * This function implements the RRA instruction
 * 
 * @param bw byte/word operation
 * @param sreg destination register
 * @param oset  offset field used for 32 bits instructions
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value
 */
//**********************RRA****************************
inline void RRA(unsigned int bw, unsigned int sreg, 
		uint16_t oset, 
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		int val1) {
  // variables
  int16_t res = 0;
  bool lsb, msb;

  lsb = (1 & val1); 

  // Behavior
  if (bw) {
    msb = (0x0080 & val1);
    val1 >>= 1;
    val1 = (((msb << 7) & 0x80) | val1);
    res = 0xff & val1;
  }
  else {
    msb = (0x8000 & val1);
    val1 >>= 1;
    val1 = (((msb << 15) & 0x8000) | val1);
    res = 0xffff & val1;
  }

  // Flags Generation  
  flags.N = bw ? getBit(res, 7) : getBit(res, 15);
  flags.Z = (res == 0) ? true : false;
  flags.C = (lsb == 1) ? true : false;
  flags.V = false;

  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));

  // debug
  dbg_printf("rra 0x%X \n", res);
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(0, bw, sreg, oset, 0, DM, RB, res);
}
/** 
 * This function implements the SXT instruction
 * 
 * @param bw byte/word operation
 * @param sreg destination register
 * @param oset  offset field used for 32 bits instructions
 * @param DM data memory
 * @param RB register bank
 * @param val1 first value
 */
//**********************SXT****************************
inline void SXT(unsigned int bw, unsigned int sreg, 
		uint16_t oset, 
		ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
		ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB, 
		int val1) {

  // variables
  int16_t res = 0;

  // Behavior
  res = (getBit(val1, 7)) ? (0xffff & (0xff00 | val1)) : (0xffff & (0x00ff & val1));

  // Flags Generation -> Status bits are not affected 
  
  // Flags Generation  
  flags.N = getBit(res, 7);
  flags.Z = (res == 0) ? true : false;
  flags.C = !flags.Z;
  flags.V = false;

  RB.write(SR,  0x0117 & (flags.C | (flags.Z << 1) | (flags.N << 2) | (getBit(RB.read(SR), 4) << 4) | (flags.V << 8)));

  // debug
  dbg_printf("sxt 0x%X \n", res);
  dbg_printf("N: %d, Z: %d, C: %d V: %d\n", flags.N, flags.Z, flags.C, flags.V);

  // Writing back values
  write_back(0, bw, sreg, oset, 0, DM, RB, res);
}
/** 
 * This function implements the PUSH instruction
 * 
 * @param bw byte/word operation
 * @param sreg destination register, actually is source register is this case
 * @param oset  offset field used for 32 bits instructions
 * @param DM data memory
 * @param RB register bank
 */
//**********************PUSH****************************
inline void PUSH(unsigned int bw, unsigned int sreg, 
                 uint16_t oset, 
                 ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
                 ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB) {
  // Behavior
  RB.write(SP, RB.read(SP)-2);

  switch (mode) {
  case _16rm:
    if (constant.mode) {
      if (bw) DM.write(RB.read(SP), (uint8_t(0xff & constant.value)));
      else  DM.write(RB.read(SP), (0xffff & constant.value));
			constant.mode = false;
    }
    else {
      if (bw) DM.write(RB.read(SP), (0x00ff & RB.read(sreg)));
      else  DM.write(RB.read(SP), (0xffff & RB.read(sreg)));
    }
    break;
  case _16irm:
    if (constant.mode) {
      if (bw) DM.write(RB.read(SP), (uint8_t(0xff & constant.value)));
      else  DM.write(RB.read(SP), (0xffff & constant.value));
			constant.mode = false;
    } 
    else {
      if (bw) DM.write(RB.read(SP), (0x00ff & DM.read(RB.read(sreg))));
      else  DM.write(RB.read(SP), (0xffff & DM.read(RB.read(sreg))));
    }
    break;
  case _16iaim: 
    if (constant.mode) {
      if (bw) DM.write(RB.read(SP), (uint8_t(0xff & constant.value)));
      else  DM.write(RB.read(SP), (0xffff & constant.value));
			constant.mode = false;
    }
    else {
      if (bw) DM.write(RB.read(SP), (0x00ff & RB.read(sreg)));
      else  DM.write(RB.read(SP), (0xffff & RB.read(sreg)));
    }      
    if (bw)
      RB.write(sreg, 0x00ff & (RB.read(sreg) + 1));
    else
      RB.write(sreg, (RB.read(sreg) + 2));
    break;
  case _32irm:
    dbg_printf("oset 0x%X \n", oset);
    if (bw) DM.write(RB.read(SP), (0x00ff & DM.read(oset)));
    else DM.write(RB.read(SP), (0xffff & DM.read(oset)));
    break;
  case _32imm:
    if (bw) DM.write(RB.read(SP), (0x00ff & oset));
    else DM.write(RB.read(SP), (0xffff & oset));
    break;
  }
  // Flags Generation -> Status bits are not affected 

  // debug
  dbg_printf("push SP - 2 -> SP = 0x%X \n", RB.read(SP));
}
/** 
 * This function implements the RRC instruction
 * 
 * @param bw byte/word operation
 * @param sreg destination register
 * @param oset  offset field used for 32 bits instructions
 * @param DM data memory
 * @param RB register bank
 * @param ac_pc program counter
 */
//**********************CALL****************************
inline void CALL(unsigned int bw, unsigned int sreg, 
                 uint16_t oset, 
                 ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
                 ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB,
                 ac_reg<unsigned>& ac_pc) {
  // Behavior
  // Increment Stack
  RB.write(SP, (RB.read(SP)-2));
  // Save current PC on top of stack
  DM.write(RB.read(SP), RB.read(PC));

  /// @todo implement all the addressing modes
  switch (mode) {
  case _16rm:
    RB.write(PC, RB.read(sreg));
    ac_pc = RB.read(sreg);
    break;
  case _16irm:
    RB.write(PC, DM.read(RB.read(sreg)));
    ac_pc = DM.read(RB.read(sreg));
    break;
  case _16iaim:
    RB.write(PC, DM.read(RB.read(sreg)));
    ac_pc = DM.read(RB.read(sreg));
    RB.write(sreg, (RB.read(sreg) + 2));
    break;
  case _32imm:
    RB.write(PC, oset);
    ac_pc = oset;
    break;		
  }
  // Flags Generation -> Status bits are not affected 
  // debug
}
/** 
 * This function implements the RRC instruction
 * 
 * @param bw byte/word operation
 * @param sreg destination register
 * @param oset  offset field used for 32 bits instructions
 * @param DM data memory
 * @param RB register bank
 * @param ac_pc program counter
 */
//**********************RETI****************************
inline void RETI(unsigned int bw, unsigned int sreg, 
                 uint16_t oset, 
                 ac_memport<msp430_parms::ac_word, msp430_parms::ac_Hword>& DM,
                 ac_regbank<16, msp430_parms::ac_word, msp430_parms::ac_Dword>& RB,
                 ac_reg<unsigned>& ac_pc) {
  // Behavior
  // SR restored 
  RB.write(SR, DM.read(RB.read(SP)));
  // SP incremented
  RB.write(SP, RB.read(SP) + 2);
  // PC restored 
  RB.write(PC, DM.read(RB.read(SP)));
  ac_pc = DM.read(RB.read(SP));
  // SP incremented
  RB.write(SP, RB.read(SP) + 2);
  
  // Flags Generation -> Status bits are not affected 
  flags.N = (RB.read(SR) & 0x4) ? true : false;
  flags.Z = (RB.read(SR) & 0x2) ? true : false;
  flags.C = (RB.read(SR) & 0x1) ? true : false;
  flags.V = (RB.read(SR) & 0x100) ? true : false;           
}
