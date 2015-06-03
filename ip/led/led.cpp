/**
 * @file   led.cpp
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements a led controller module
 * 
 */

#include "led.h"
#include <iostream>
using std::cout;
using std::endl;

Led::Led(sc_module_name name)
: sc_module(name) {
	req= new ac_tlm_req; 
	rsp= new ac_tlm_rsp; 
	SC_METHOD(blink_method);
	//dont_initialize();
	sensitive << eventmem_write_p1out;
}

void Led::blink_method() {
	value = read_register(P1OUT_);
	//cout << sc_time_stamp() << ": P1out written "<< value << endl;
	if ((value & BIT0) & (value & BIT6)){
		cout << sc_time_stamp() << ": LED: ** Red 1 Green 1" << endl;
		redled.write((sc_bit) 1); 
	} else if ((value & BIT0) & !(value & BIT6)) {
		cout << sc_time_stamp() << ": LED: *- Red 1 Green 0" << endl;
		redled.write((sc_bit) 1); 
	} else if (!(value & BIT0) & (value & BIT6)) {
		cout << sc_time_stamp() << ": LED: -* Red 0 Green 1" << endl;
		redled.write((sc_bit) 0); 
	}else {
		cout << sc_time_stamp() << ": LED: -- Red 0 Green 0" << endl;
		redled.write((sc_bit) 0); 
	}
}

