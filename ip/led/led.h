/**
 * @file   led.h
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements a led controller module
 * 
 */

#ifndef LED_H
#define LED_H

#include <systemc.h>
#include "ac_tlm_protocol.H" //archc for port to cpu

#define P1OUT_                0x0021    /* Port 1 Output */
#define BIT0                (0x0001)
#define BIT6                (0x0040)
extern sc_event eventmem_write_p1out;

SC_MODULE(Led) {
	sc_port<ac_tlm_transport_if>  portDM; //port to mem TLM	
	ac_tlm_req* req; //memory request
	ac_tlm_rsp* rsp; //memory response 

	sc_out<sc_bit> redled; //output port sda

	uint32_t value;

	SC_HAS_PROCESS(Led);
	Led(sc_module_name name);
	void blink_method();



	/** read register **/
	uint32_t read_register(uint32_t address)
	{
		req->type=READ;
		req->addr=address;
		*rsp=portDM ->transport(*req); 
		return (rsp->data);
	}
};
#endif // LED_H
