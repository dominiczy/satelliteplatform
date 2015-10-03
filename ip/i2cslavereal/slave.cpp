/**
 * @file   slave.cpp
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements an I2C slave module
 * 
 */

/** includes **/ 
#include	<systemc.h> 
#include	<tlm_utils/simple_target_socket.h> 
//#include	<string.h>
//#include "ac_tlm_protocol.H" //archc for port to cpu


/** macro **/ 
#define SC_INCLUDE_DYNAMIC_PROCESSES 

/** namespace **/ 
using namespace tlm; 
using namespace tlm_utils; 

/** class for target module **/ 
class target : public sc_module { 
	public: 
	/** instaniation of predefined class **/ 
	simple_target_socket<target> socket; 

	sc_out<sc_bit> sda; 
	sc_in_clk scl; 

	


	virtual void b_transport(tlm_generic_payload &trans, sc_time& delay) { //implementation of b_transport

		sc_bit b; 
		
		tlm_command cmd = trans.get_command(); 
		sc_dt :: uint64 addr = trans.get_address(); 
		sc_out<sc_bit> *ptr = (sc_out<sc_bit>*)trans.get_data_ptr(); 
		unsigned int length = trans.get_data_length(); 
		unsigned char *byte = trans.get_byte_enable_ptr(); 
		unsigned int width = trans.get_streaming_width(); 

		char sdapin=7;
		char sclpin=6; //for msp430g2553
		
		if (cmd == TLM_WRITE_COMMAND) { 
			b = *ptr;
			// write b to sda (pin ?) 
			
			//DO NOT PUBLISH THIS CODE!!!! 
			system("echo $PASS | sudo -S mspdebug -q rf2500 \"simio add gpio aap\" \"simio config aap set %c %c\" \"simio info aap\"", sdapin, b);

				
		} else if (cmd == TLM_READ_COMMAND) { 
			system("echo $PASS | sudo -S mspdebug -q rf2500 \"simio add gpio aap\" \"simio info aap\"");
					// check sda (pin ?), write to sda
			cin>>b;
			sda.write(b); 
			memcpy(ptr, &sda, length);  
		} else if (cmd == TLM_IGNORE_COMMAND) { 
			b = *ptr;
			// write b to sda (pin ?) 
			system("echo $PASS | sudo -S mspdebug -q rf2500 \"simio add gpio aap\" \"simio config aap set %c %c\" \"simio info aap\"", sdapin, b);
		} 
		trans.set_response_status(TLM_OK_RESPONSE);		 
	} 

	/** constructor for target class **/ 
	target(sc_module_name name) : sc_module(name), socket("SOCKET") { 
		 
		socket.register_b_transport(this, &target::b_transport);
	} 

	SC_HAS_PROCESS(target); 
};
