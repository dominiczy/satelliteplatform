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
#include	<string.h>
#include "ac_tlm_protocol.H" //archc for port to cpu


/** macro **/ 
#define SC_INCLUDE_DYNAMIC_PROCESSES 

#define UCB0CTL1_             0x0069    /* USCI B0 Control Register 1 */
#define UCB0I2COA_            0x0118    /* USCI B0 I2C Own Address */
#define UCB0STAT_             0x006D    /* USCI B0 Status Register */
#define UCSTTIFG            (0x02)    /* START Condition interrupt Flag */
#define UCSTPIFG            (0x04)    /* STOP Condition interrupt Flag */
#define UCTR                (0x10)    /* Transmit/Receive Select/Flag */
#define IFG2_                 0x0003    /* Interrupt Flag 2 */
#define UCB0TXIFG           (0x08) // transmit interrupt flag
#define UCB0RXIFG           (0x04) //receive interrupt flag
#define UCB0TXBUF_            0x006F    /* USCI B0 Transmit Buffer */
#define UCB0RXBUF_            0x006E    /* USCI B0 Receive Buffer */
#define USCIAB0TX_VECTOR        (0xFFEC) // USCI A0/B0 Transmit receive int vector
#define USCIAB0RX_VECTOR        (0xFFEE) // USCI A0/B0 stt stp int vector 

extern sc_event eventmem_write_txbuf; //mem write events
extern bool boolmem_write_txbuf;
extern sc_event eventmem_write_ctl1; 
extern bool boolmem_write_ctl1;
extern sc_event eventmem_read_rxbuf; //extern mem read event
extern bool boolmem_read_rxbuf;

/** namespace **/ 
using namespace tlm; 
using namespace tlm_utils; 

/** Flags **/
extern char STT_flag; 
extern char STP_flag; 
extern bool bus_stall;

/** class for target module **/ 
class target : public sc_module { 
	public: 
	/** instaniation of predefined class **/ 
	simple_target_socket<target> socket; 

	sc_port<ac_tlm_transport_if>  portDM; // port to mem TLM	
	sc_port<ac_tlm_transport_if>  portcpu; // port to cpu TLM

	sc_out<sc_bit> sda; 
	sc_in_clk scl; 

	sc_bv<8> transmit_shift_reg; // transmit shift register
	sc_bv<8> receive_shift_reg; // receive shift register
	sc_bv<8> UCB0TXBUF; // transmit buffer
	sc_bit UCTR_flag; // transmit flag
	sc_bv<7> UCB0I2COA; // own address
	bool addressmatch;

	ac_tlm_req* req; //memory request
	ac_tlm_rsp* rsp; //memory response 

	uint16_t UCB0STAT; // USCI B0 Status Register value
	uint16_t UCB0CTL1; // ctl1 value
	uint16_t IFG2; // flag reg 2 value 
	int bitnr; 

	/** transmit interrupt **/
	void transmit_interrupt()
	{
		IFG2=read_register(IFG2_); 
		write_register(IFG2_ , ((IFG2 | UCB0TXIFG)& !UCB0RXIFG));

		req->type=WRITE; 
		req->data=USCIAB0TX_VECTOR; 
		portcpu->transport(*req); 
	}

	/** receive interrupt **/
	void receive_interrupt()
	{
		IFG2=read_register(IFG2_); 
		write_register(IFG2_ , ((IFG2 | UCB0RXIFG)& !UCB0TXIFG));
		
		req->type=WRITE; 
		req->data=USCIAB0TX_VECTOR; 
		portcpu->transport(*req); 
	}

	/** start cond interrupt **/
	void start_cond_interrupt()
	{
		
		UCB0STAT=read_register(UCB0STAT_); 
		write_register(UCB0STAT_ ,((~(UCSTPIFG))&(UCB0STAT | UCSTTIFG)));

		req->type=WRITE; 
		req->data=USCIAB0RX_VECTOR ; 
		portcpu->transport(*req); 
	}

	/** stop cond interrupt **/
	void stop_cond_interrupt()
	{
		UCB0STAT=read_register(UCB0STAT_); 
		write_register(UCB0STAT_ ,((~(UCSTTIFG))&(UCB0STAT | UCSTPIFG)));

		req->type=WRITE; 
		req->data=USCIAB0RX_VECTOR ; 
		portcpu->transport(*req); 
	}

	/** read register **/
	uint32_t read_register(uint32_t address)
	{
		req->type=READ;
		req->addr=address;
		*rsp=portDM ->transport(*req); 
		return (rsp->data);
	}

	/** write register **/
	void write_register(uint32_t address, uint32_t data)
	{
		req->type=WRITE;
		req->addr=address;
		req->data=data;
		*rsp=portDM ->transport(*req); 
	}
	 
	/** wait txbuf **/
	void wait_txbuf()
	{
		//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- wait txbuf "  << endl; 
		if (!(boolmem_write_txbuf)) { 
			//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- wait longer txbuf "  << endl; 
			bus_stall=1;
			wait(eventmem_write_txbuf);
		}
		bus_stall=0;
		boolmem_write_txbuf=0;
		UCB0TXBUF=(sc_bv<8>)(read_register(UCB0TXBUF_ )); 
	}

	virtual void b_transport(tlm_generic_payload &trans, sc_time& delay) { //implementation of b_transport

		sc_bit b; 
		
		tlm_command cmd = trans.get_command(); 
		sc_dt :: uint64 addr = trans.get_address(); 
		sc_out<sc_bit> *ptr = (sc_out<sc_bit>*)trans.get_data_ptr(); 
		unsigned int length = trans.get_data_length(); 
		unsigned char *byte = trans.get_byte_enable_ptr(); 
		unsigned int width = trans.get_streaming_width(); 

		if (cmd == TLM_WRITE_COMMAND) { 
			b = *ptr; 
			if (STT_flag) {
				if (bitnr < 7) { // addressing
					//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- read address " << b << endl; 
					receive_shift_reg[bitnr++] = b;
					//cout  << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- shift reg " << receive_shift_reg << endl;
				} else { // receiving R/W
					//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- R/!W : " << b << endl; 

					/** set/clear transmit flag **/
					UCTR_flag=b; 
					UCB0CTL1=read_register(UCB0CTL1_);

					/** read own address **/
					//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- Rec : " << receive_shift_reg << endl; 
					UCB0I2COA=read_register(UCB0I2COA_);
					//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- Own address " << read_register(UCB0I2COA_) << endl; 

					/** check address match **/
					addressmatch=0;
					if ((sc_bv<7>)receive_shift_reg == UCB0I2COA) { 
						addressmatch=1;
					}
					if (addressmatch) {
						/** start cond interrupt **/
						//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- start cond interrupt: " << endl;
						start_cond_interrupt();
				
						if (UCTR_flag) { //transmitting
							write_register(UCB0CTL1_ , (UCB0CTL1 & (UCTR)) ); 
							//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- transmitting, sending transmit_interrupt" << endl; 

							/** transmit interrupt **/
							transmit_interrupt();
						}
						else { //receiving
							write_register(UCB0CTL1_ , (UCB0CTL1 & (~(UCTR))) );
						} 
					}
				}

			} else { //if (bitnr >= 0) { 
				if (addressmatch) {
					if (UCTR_flag) { //transmitting data
						/** check master acknowledge **/
						//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- Ack : " << b << " wait for txbuf write "<< endl; 
						/** wait for txbuf **/
						wait_txbuf();
			
						//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- data to be transmitted : " << UCB0TXBUF<< endl;
						bitnr=7;
					}
					else {// receiving data
						//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- receiving data " << b << endl; 
						receive_shift_reg[bitnr--] = b;
					}	
				}
			} 

				
		} else if (cmd == TLM_READ_COMMAND) { 
			if (addressmatch) {
				b = 0; 
				if (STT_flag) { 
					if (UCTR_flag) { //transmitting
						//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- wait for first txbuf write: " << UCB0TXBUF<< endl;
						/** wait for txbuf **/
						wait_txbuf();

						//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- data to be transmitted : " << UCB0TXBUF<< endl;
						cout << sc_time_stamp()<< ": I2C: Slave "<< read_register(UCB0I2COA_)<<"----- Transmitting data : " << UCB0TXBUF << endl; 
						/** clear start cond int flag **/
						UCB0STAT=read_register(UCB0STAT_); 	
						write_register(UCB0STAT_ ,((~(UCSTTIFG))&(UCB0STAT)));
					}
					/** acknowledge address match **/
					sda.write(b); 
					memcpy(ptr, &sda, length);  
				} else { 
					if (UCTR_flag) { // transmitting
						if (bitnr == 7){
							/** buffer to shift register **/
							transmit_shift_reg=UCB0TXBUF;

							//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- transmit interrupt"<< endl;

							/** transmit interrupt **/
							transmit_interrupt();
						}

						/** transmit data bitwise **/
						//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"---- transmitting data "<< b << " bitnr " << bitnr << endl;

						b = transmit_shift_reg[bitnr--]; 
						sda.write(b); 
						memcpy(ptr, &sda, length);   
					}
					else { // receiving
						/* read data byte */
						bitnr = 7; 
						cout << sc_time_stamp()<< ": I2C: Slave "<< read_register(UCB0I2COA_)<<"----- Received data : " << receive_shift_reg << endl; 

						/* transfer shift register to buffer */
						write_register(UCB0RXBUF_, (sc_uint<8>) receive_shift_reg);

						/* receive interrupt */
						receive_interrupt();	

						/* wait until buffer read */
						//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- wait for rxbuf read " << endl;
						if (!(boolmem_read_rxbuf)) wait(eventmem_read_rxbuf);
						boolmem_read_rxbuf=0;				
						//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- rxbuf read happened " << endl;

							
						sda.write(b); 
						memcpy(ptr, &sda, length); 
					}
				}
			} 
	
		} else if (cmd == TLM_IGNORE_COMMAND) { 
			if (STP_flag == 1) { 
				bitnr = 0; 
				if (addressmatch) { // stop condition
					//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- Stop : " << *ptr  << endl; 
					/** stop cond interrupt **/
					stop_cond_interrupt();
				}
			} else  { // start condition
				//cout << "I2C: Slave "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<< read_register(UCB0I2COA_)<<"- Start : " << *ptr << endl; 
			}
		} 
		trans.set_response_status(TLM_OK_RESPONSE);		 
	} 

	/** constructor for target class **/ 
	target(sc_module_name name) : sc_module(name), socket("SOCKET") { 
		bitnr = 0; 
		socket.register_b_transport(this, &target::b_transport);
		req= new ac_tlm_req; 
		rsp= new ac_tlm_rsp;  
	} 

	SC_HAS_PROCESS(target); 
};
