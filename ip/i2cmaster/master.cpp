/**
 * @file   master.cpp
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements an I2C master module
 * 
 */

/** Includes **/ 
#include	<systemc.h> 
//#include         <time.h>
#include	<tlm_utils/simple_initiator_socket.h> 
#include	<string.h>
#include "ac_tlm_protocol.H" //archc for port to cpu

/** Macro **/ 
#define SC_INCLUDE_DYNAMIC_PROCESSES 

#define UCB0CTL1_             0x0069    /* USCI B0 Control Register 1 */
#define UCTXSTP             (0x04)    /* Transmit STOP */
#define UCTXSTT             (0x02)    /* Transmit START */
#define UCB0I2CSA_            0x011A    /* USCI B0 I2C Slave Address */
#define UCTR                (0x10)    /* Transmit/Receive Select/Flag */
#define IFG2_                 0x0003    /* Interrupt Flag 2 */
#define UCB0TXIFG           (0x08) // transmit interrupt flag
#define UCB0RXIFG           (0x04) //receive interrupt flag
#define UCALIFG             (0x01)    /* Arbitration Lost interrupt Flag */
#define UCB0STAT_             0x006D    /* USCI B0 Status Register */
#define UCB0TXBUF_            0x006F    /* USCI B0 Transmit Buffer */
#define UCB0RXBUF_            0x006E    /* USCI B0 Receive Buffer */
#define USCIAB0TX_VECTOR        (0xFFEC) // USCI A0/B0 Transmit receive int vector
#define USCIAB0RX_VECTOR        (0xFFEE) // USCI A0/B0 stt stp int vector 
#define UCB0CTL0_             0x0068    /* USCI B0 Control Register 0 */
#define UCMST               (0x08)    /* Sync. Mode: Master Select */

char STT_flag; //start flag used for communication with slave
char STP_flag; //stop flag used for communication with slave

bool bus_stall;
extern sc_event eventmem_write_txbuf; //mem write events
extern bool boolmem_write_txbuf;
extern sc_event eventmem_write_ctl1; 
extern bool boolmem_write_ctl1;
extern sc_event eventmem_read_rxbuf; //extern mem read event
extern bool boolmem_read_rxbuf;

/** namespace **/ 
using namespace tlm; 
using namespace tlm_utils; 

/** class for initiator **/ 
class initiator : 
		public sc_module
		//, public ac_tlm_transport_if  //ArchC TLM
	{ 
	public: 
	/** Instaniation of predefined class **/ 
	simple_initiator_socket<initiator> socket; 

	sc_port<ac_tlm_transport_if>  portDM; //port to mem TLM	
	sc_port<ac_tlm_transport_if>  portcpu; //port to cpu TLM

	sc_out<sc_bit> sda; //output port sda
	sc_in_clk scl; //input clock
	int bitnr; 

	uint16_t UCB0CTL1; // ctl1 value
	uint16_t IFG2; // flag reg 2 value
	sc_bv<7> UCB0I2CSA; // slave address
	sc_bv<8> UCB0TXBUF; // transmit buffer
	sc_bit UCTR_flag; // transmit flag
	sc_bv<8> shift_reg; // shift register

	ac_tlm_req* req; //memory request
	ac_tlm_rsp* rsp; //memory response

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

	/** wait for other master **/
	void wait_other_master()
	{
		write_register(UCB0STAT_,((read_register(UCB0STAT_))&UCALIFG));// set arbitration lost flag
		while (STP_flag==0) {
			//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Other master transmitting, waiting for stop flag " << endl;
			wait();		
		}
		//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- stop flag set, wait until cleared " << endl;
		while (STP_flag==1) {
			//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Other master transmitting, wait until stop flag cleared" << endl;
			wait();				
		}
		//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- other master finished, ready to start" << endl;
	}

	/** function declaration **/ 
	void process()
	{	
		wait(2500, SC_NS); //100? wait to prevent use of garbage reg values
		sc_time delay = sc_time(2500, SC_NS); 
		sc_bit b;

		/** wait for start flag **/	
		transmissionstart: //goto label in case of arbitration loss
		//clock_t start = clock(); // clock start
		do {	
			//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- waiting for stt flag write " << UCTXSTT << endl;
			if (!(boolmem_write_ctl1)) wait(eventmem_write_ctl1);
			boolmem_write_ctl1=0; // this is a global variable, can be reset by other master!!!
			UCB0CTL1=read_register(UCB0CTL1_);
			//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- stt flag write happened " << UCTXSTT << endl;
			/** HERE: check for start condition of other master on bus, if yes wait (slave remains) **/
			if (STT_flag==1 ) {
				//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Other master transmitting " << endl;
				wait_other_master();
			}
		} while ((!(UCB0CTL1 & UCTXSTT))| ( !((read_register(UCB0CTL0_)) & UCMST) )); // no start flag or no master
		
		//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- start flag set, start transmission " << UCTXSTT << endl;
		STT_flag=1; 

		/** see if transmitting or receiving **/
		UCB0CTL1=read_register(UCB0CTL1_);
		if (UCB0CTL1 & UCTR){ // transmitting
			UCTR_flag=1;
		} else UCTR_flag=0; // receiving

		/** generate transmit interrupt if transmitting **/
		if(UCTR_flag) transmit_interrupt(); 
	
		/** TLM-2 generic payload transaction **/
    		tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
      		trans->set_address( 0);
   		trans->set_data_ptr(reinterpret_cast<unsigned char *>(&sda)); 
      		trans->set_data_length( 4 );
      		trans->set_streaming_width( 4 ); // = data_length to indicate no streaming
      		trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
      		trans->set_dmi_allowed( false ); // Mandatory initial value
      		trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
		trans->set_command(TLM_IGNORE_COMMAND); 

		/** write start condition to bus **/
		b = 0; 
		sda.write(b); 
		wait(6*250, SC_NS);//wait(6, SC_NS); //why not wait() for clock >> start condition sld change on high scl
		socket->b_transport(*trans, delay); 

		trans->set_command(TLM_WRITE_COMMAND); 

		/** writing slave address to bus **/ 
		UCB0I2CSA=(sc_bv<7>)(read_register(UCB0I2CSA_));
		//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- slave address " << UCB0I2CSA  << endl;
		while (bitnr < 7) { 			
			b = UCB0I2CSA[bitnr]; 
			sda.write(b); 
			wait(); 
			socket->b_transport(*trans, delay); 

			/** if b is high, check if other master is writing b low on bus, if yes wait (slave remains) **/
			if ((b==1)&(sda.read() == 0)) {
				//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Arbitration lost, waiting " << endl;
				write_register(UCB0CTL1_, ( (read_register(UCB0CTL1_))& (!(UCTXSTT)) ) ); // clear start flag
				wait_other_master();
				//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- bus free, starting over " << endl;
				goto transmissionstart;
			}
			bitnr++; 
			/** writing read/!write bit to bus **/ 
			if (bitnr == 7) { 
				b = !(UCTR_flag); 
				sda.write(b); 
				wait(); 
				socket->b_transport(*trans, delay);
			} 
		} 

		/** if transmitting **/
		if (UCTR_flag) 
		{ 
			/** read transmit buffer **/		
			if (!(boolmem_write_txbuf)) { // no data written yet
				//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Waiting for transmit buffer write "  << endl;
				wait(eventmem_write_txbuf);	
			}	
			boolmem_write_txbuf=0;		
			UCB0TXBUF=(sc_bv<8>)(read_register(UCB0TXBUF_ ));
		}

		/** wait for address acknowledgement **/
		trans->set_command(TLM_READ_COMMAND); 
		socket->b_transport(*trans, delay); 
		wait(); 
		//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Ack : " << sda.read() << endl; 
		if (sda.read() == 1) //no acknowledgement, this does not happen, write 1 by default?
		{	
			int test;
			cin>>test; 
		}

		/** clear start flag **/
		UCB0CTL1=read_register(UCB0CTL1_ ); 
		write_register(UCB0CTL1_ , (UCB0CTL1 & (~(UCTXSTT))) );
		STT_flag=0; 
	
		while (!(UCB0CTL1 & UCTXSTP)) // no stop flag
		{
			/** if transmitting **/
			if (UCTR_flag) 
			{ 
				/** buffer to shift register **/
				shift_reg=UCB0TXBUF;

				/** transmit interrupt **/
				transmit_interrupt(); 
	
				/** data transfer **/
				cout << sc_time_stamp()<< ": I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Transmitting data : " << UCB0TXBUF << endl;
				trans->set_command(TLM_WRITE_COMMAND);  
				bitnr = 7; 
				while (bitnr >= 0) {
					b = shift_reg[bitnr--]; 
					sda.write(b); 
					wait(); 
					socket->b_transport(*trans, delay); 
				} 
				/** read transmit buffer **/		
				while ((!(read_register(UCB0CTL1_) & UCTXSTP))&(!(boolmem_write_txbuf))){ // no stop flag and no data written
					//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Waiting for transmit buffer write "  << endl;
					wait(eventmem_write_txbuf | eventmem_write_ctl1);
				}	
				boolmem_write_txbuf=0;
				UCB0TXBUF=(sc_bv<8>)(read_register(UCB0TXBUF_ ));	
 	 
				/** wait for acknowledgement **/
				trans->set_command(TLM_READ_COMMAND); 
				socket->b_transport(*trans, delay); 

				do {wait();} while (bus_stall & (!(read_register(UCB0CTL1_) & UCTXSTP))) ; 
				//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Ack : " << sda.read() << endl; 
				if (sda.read() == 1) //no acknowledgement, this does not happen, write 1 by default?
				{	
					int test;
					cin>>test; 
				}
			}
			/** if receiving **/
			else {
				trans->set_command(TLM_READ_COMMAND);
				bitnr=7;
				/* receive data bitwise */
				while (bitnr >= 0) { 
					socket->b_transport(*trans, delay); 
					do {wait();} while (bus_stall & (!(read_register(UCB0CTL1_) & UCTXSTP)));					
					//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- receiving data : " << sda.read() << endl; 
					shift_reg[bitnr--] = sda.read();
				}
				cout << sc_time_stamp()<< ": I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Received data : " << shift_reg << endl;

				/* transfer shift register to buffer */
				write_register(UCB0RXBUF_, (sc_uint<8>) shift_reg);

				/* receive interrupt */
				receive_interrupt();	

				/* wait until receive buffer read */	
				while ((!(read_register(UCB0CTL1_) & UCTXSTP))&(!(boolmem_read_rxbuf))){ // no stop flag and buffer not read
					//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- Waiting for rxbuffer read "  << endl;
					wait(eventmem_read_rxbuf | eventmem_write_ctl1);
				}	
				boolmem_read_rxbuf=0;
	
				/** acknowledgement **/
				trans->set_command(TLM_WRITE_COMMAND);
				b=0;
				sda.write(b); 
				wait(); 
				socket->b_transport(*trans, delay); 
			}; 
			UCB0CTL1=read_register(UCB0CTL1_ );
		}
		/** write stop condition on bus **/
		//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- stop flag " << endl;
		STP_flag = 1; //stop flag
		wait(7*250, SC_NS); 
		trans->set_command(TLM_IGNORE_COMMAND); 
		b = 1; 
		sda.write(b); 
		wait(); 
		socket->b_transport(*trans, delay); 

		/** clear stop flag **/
		//cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- clear stop flag " << endl;
		write_register(UCB0CTL1_ , (UCB0CTL1 & (~(UCTXSTP)))); //clear stop flag
		STP_flag=0; 
		//if (UCTR_flag) sc_stop(); //testing transmission 
		wait();	
		bitnr = 0; 		
		
		//clock_t end = clock(); // clock end
		//float seconds = (float)(end - start) / CLOCKS_PER_SEC;
		////cout << "I2C: Master "<<sc_core::sc_get_current_process_b()->get_parent()->basename()<<"--- time "<< seconds << endl;

		goto transmissionstart;
	}


	/** constructor for initiator class **/ 
	initiator(sc_module_name name) : sc_module(name), socket("socket") { 
		bitnr = 0; 
		
		req= new ac_tlm_req; 
		rsp= new ac_tlm_rsp; 
		SC_THREAD(process); 
		sensitive << scl.neg(); //clock sensitive
	} 

	SC_HAS_PROCESS(initiator); 
};
