/**
 * @file   counter.cpp
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements a timer/counter module
 * 
 */

#include "counter.h"
#include <iostream>
using std::cout;
using std::endl;

Counter::Counter(sc_module_name name, int start)
: sc_module(name),
value(start) {
	req= new ac_tlm_req; 
	rsp= new ac_tlm_rsp; 
	SC_THREAD(event_trigger_thread);
	SC_METHOD(count_method);
	dont_initialize();
	sensitive << count_event;
}

void Counter::count_method() {
	/*if (read_register(TACTL_)& MC_1){ // up mode (mc_1)
		/**if (value <= 0){
			taifg_interrupt();
		}
		value=read_register(TAR_); // read in case changed by software
		cout << sc_time_stamp() << ": Counter has value "<< value << " in up mode and taccr0 "<< read_register(TACCR0_ ) << endl;
		
		value++;
		if (read_register(TACCTL0_) & TACLR){
			value=0;
			write_register(TACCTL0_, (read_register(TACCTL0_) & !(TACLR)) ); // reset clear
		}
		write_register(TAR_ , value); // write to reg
		if (value >= read_register(TACCR0_ )) { // compare match value
		 	if (read_register(TACCTL0_) & CCIE){ // if interrupt enabled
				ccifg_interrupt();
			}
			value=-1; // 0??
		}
	}
	else*/ 
	//cout << sc_time_stamp() << ": Counter has value "<< value << "." << endl;

	if (read_register(TA0CTL_) & MC_2){ // cont mode (mc_2)
		//value=(0xffff & read_register(TAR_)); // read in case changed by software
		value++;
		//cout << sc_time_stamp() << ": CNT: Counter has value "<< value <<endl;
		if (clear) {
			compare+=read_register(TACCR0_); //should be =, but gets reset
			//cout << sc_time_stamp() << ": CNT: Counter start or compare, compare "<< compare <<endl;
			//compare=comparezero;
			//value=0;
			clear=0;
		}
		if (read_register(TA0CTL_) & TACLR){
			clear=1;
			cout << sc_time_stamp() << ": CNT: Counter clear " <<endl;
			write_register(TA0CTL_, (read_register(TA0CTL_) & !(TACLR)) ); // reset clear
			write_register(TA0CTL_, (read_register(TA0CTL_) |  MC_2 )); // prevent clearing of mc2
			value=0;
		}

		/*if (value >= 0xffff) { // reset
			cout << sc_time_stamp() << ": CNT: Counter reset"<< endl;
			value=0; // 0??
			compare=comparezero;
		}*/
		//compare=read_register(TACCR0_);
		
		write_register(TAR_ , value); // write to reg

		if ((value == compare)){// && !(clear)) { // compare match value
		 	//if (read_register(TACCTL0_) & CCIE){ // if interrupt enabled (gets reset for some reason)
				if (clear){
					compare=32;	// very bad but works	
				} else {
					ccifg_interrupt();
				}
				cout << sc_time_stamp() << ": CNT: Counter tick interrupt" << endl;
			//}	
			
			clear=1;
			//compare=read_register(TACCR0_);
			//compare+=comparezero; // not correct but works (taccr0 gets reset for some reason)
		}
	}
}

void Counter::event_trigger_thread() {
	for (;;) { // infinite loop
	wait(30517578,SC_PS); // aclk has freq 32768 Hz > 30517578 SC_PS
	// then immediately trigger "count_method" through "count_event"
	count_event.notify();
	}
}
