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
	sensitive << eventmem_write_p1out;
}

void Led::blink_method() {
	value = read_register(P1OUT_); // read p1 register
	if ((value & BIT0) & (value & BIT6)){
		cout << sc_time_stamp() << ": LED: ** Red 1 Green 1" << endl;
	} else if ((value & BIT0) & !(value & BIT6)) {
		cout << sc_time_stamp() << ": LED: *- Red 1 Green 0" << endl; 
	} else if (!(value & BIT0) & (value & BIT6)) {
		cout << sc_time_stamp() << ": LED: -* Red 0 Green 1" << endl;
	}else {
		cout << sc_time_stamp() << ": LED: -- Red 0 Green 0" << endl;
	}
}

