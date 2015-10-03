/**
 * @file      ac_tlm_mem.cpp
 * @author    Bruno de Carvalho Albertini
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   0.1
 * @date      Sun, 02 Apr 2006 08:07:46 -0200
 *
 * @brief     Implements a ac_tlm memory.
 *
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 * 
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  Added sc_events and boolean
 * 
 */
 
//////////////////////////////////////////////////////////////////////////////
// Standard includes
// SystemC includes
// ArchC includes

#include "ac_tlm_mem.h"

#define UCB0TXBUF_            0x006F    /* USCI B0 Transmit Buffer */
#define UCB0CTL1_             0x0069    /* USCI B0 Control Register 1 */
#define UCB0RXBUF_            0x006E    /* USCI B0 Receive Buffer */
#define P1OUT_                0x0021    /* Port 1 Output */

//////////////////////////////////////////////////////////////////////////////
 sc_event eventmem_write_txbuf; // events
 bool boolmem_write_txbuf;
 sc_event eventmem_write_ctl1; 
 bool boolmem_write_ctl1;
 sc_event eventmem_read_rxbuf;
 bool boolmem_read_rxbuf;
 sc_event eventmem_write_p1out;

/// Namespace to isolate memory from ArchC
using user::ac_tlm_mem;

/// Constructor
template<unsigned int N_TARGETS>
ac_tlm_mem<N_TARGETS>::ac_tlm_mem( sc_module_name module_name , int k ) :
  sc_module( module_name )
{
	for (unsigned int i = 0; i < N_TARGETS; i++)
    	{
		char txt[20];
     		sprintf(txt, "iport_%d", i);
      		target_export[i] = new sc_export< ac_tlm_transport_if >(txt);
	    	/// Binds target_export to the memory
	    	(*target_export[i])( *this );

	}
    /// Initialize memory vector
    memory = new uint8_t[ k ];
    for(k=k-1;k>0;k--) memory[k]=0;
}

/// Destructor
template<unsigned int N_TARGETS>
ac_tlm_mem<N_TARGETS>::~ac_tlm_mem() {

  delete [] memory;
}

/** Internal Write
  * Note: Always write 32 bits
  * @param a is the address to write
  * @param d id the data being write
  * @returns A TLM response packet with SUCCESS
*/
template<unsigned int N_TARGETS>
ac_tlm_rsp_status ac_tlm_mem<N_TARGETS>::writem( const uint32_t &a , const uint32_t &d )
{
  *((uint32_t *) &memory[a]) = *((uint32_t *) &d);

	if (a==UCB0TXBUF_) { // event mem write txbuf
		// cout << "MEM: "<<name()<<" txbuf write notify "<< a <<  endl;
		 eventmem_write_txbuf.notify();
		 boolmem_write_txbuf=1;
	} else if (a==UCB0CTL1_) { // event mem write ctl1
		// cout << "MEM: "<<name()<<" ctl1 write notify "<< a <<  endl;
		 eventmem_write_ctl1.notify();
		 boolmem_write_ctl1=1;
	} else if (a==P1OUT_) { // event mem write ctl1
		// cout << "MEM: "<<name()<<" p1out write notify "<< a <<  endl;
		 eventmem_write_p1out.notify();
	}
  return SUCCESS;
}

/** Internal Read
  * Note: Always read 32 bits
  * @param a is the address to read
  * @param d id the data that will be read
  * @returns A TLM response packet with SUCCESS and a modified d
*/
template<unsigned int N_TARGETS>
ac_tlm_rsp_status ac_tlm_mem<N_TARGETS>::readm( const uint32_t &a , uint32_t &d )
{
  *((uint32_t *) &d) = *((uint32_t *) &memory[a]);

	if (a==UCB0RXBUF_){ // even mem read rxbuf
		//cout << "MEM: "<<name()<<"rxbuf read notify "<< a <<  endl;		
		eventmem_read_rxbuf.notify();
		boolmem_read_rxbuf=1;
	}
  return SUCCESS;
}



