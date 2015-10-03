#ifndef _MSP430_GLOBAL
#define _MSP430_GLOBAL

/**
 * @file   msp430_global.cpp
 * @author Rogerio Paludo <rogerio.pld@gmail.com>
 * @date   Thu Jan 29 03:12:12 2015
 * 
 * @brief  Global values used throughout the model
 * 
 * @todo nothing so far
 */

/// @file 

/// Define Debug mode. 
#define DEBUG_MODEL
#include "ac_debug_model.H"
#include <stdint.h>

/// Macro to get one bit.
#define getBit(variable, position) (((variable & (1 << (position))) != 0) ? true : false) 

/**
 * 
 * @defgroup DATA_TYPES Special data Types used.
 * 
 * @brief data types used 
 * @{ 
 */

/** 
 *
 * @defgroup SPECIAL_REGISTERS Special Registers
 *
 * @brief used to control flow and stack 
 * @{
 */

/// Program Counter register
#define PC 0 
/// Stack Pointer register
#define SP 1 
/// Status register
#define SR 2 

/** @} */

/** @struct cnt_gen constant generator struture
 * @brief This struture is used to indicate when a constant is loaded
 *        and its value
 * @var cnt_gen::mode 
 * Member 'mode' contains TRUE when a constant constant mode is used
 * @var cnt_gen::value
 * Member 'value' contains the constant value
 */
typedef struct cnt_gen {
  bool mode;
  int16_t value;
} cnt;

/** @struct flag_s processor flags structure
 * @brief This structure retain all the processor flags
 * 
 * @var flag_s::N 
 * Member 'N' contains 0x01 when the result is negative
 * @var flag_s::Z
 * Member 'Z' contains 0x01 when the result is Zero
 * @var flag_s::C 
 * Member 'C' contains 0x01 when the result has a carry bit
 * @var flag_s::V
 * Member 'V' contains 0x01 when the result overflowed
 */
typedef struct flag_s {
  bool N; // Negative
  bool Z; // Zero
  bool C; // Carry
  bool V; // Overflow
} flag_t;

/** Addressing modes 
 *		
 *  - RM register mode
 *   - IM indexed mode
 *   - SM Symbolic mode 
 *   - AM Absolute mode
 *  - IRM indexed register mode
 *  - IAIM indexed autoincrement mode
 *  - IMM immediate mode
 */
enum Addmodes { _16rm, _16irm, _16iaim,
		_32idst, _32isrc, _32imm, _32iaim, _32irm,
		_48idm, _48imm};
//@}

/** 
 *
 * @defgroup GLOBAL_VARIABLES Global Variables 
 *
 * @brief Global values used throughout the model
 * @{
 */
/// Constant generator type
extern cnt constant;
/// Processor flags variable
extern flag_t flags;
/// Addressing modes variable
extern Addmodes mode;

#endif //_MSP430_GLOBAL
