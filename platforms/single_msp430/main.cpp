/**
 * @file   main.cpp
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file instantiates and connects the modules and starts the simulation
 * 
 */
 

const char *project_name="msp430";
const char *project_file="msp430.ac";
const char *archc_version="2.2";
const char *archc_options="";

#include  <iostream>
#include  <systemc.h>
#include  "ac_stats_base.H"
#include  "msp430.H"
#include  "ac_tlm_mem.h"
#include  "top.h" 

using user::ac_tlm_mem;

int sc_main(int ac, char *av[])
{

  //!  ISA simulator
  msp430 msp430_proc1("msp430");
  ac_tlm_mem mem("mem"); //memory
 

  msp430_proc1.DM_port(mem.target_export); //port binding memory

  /** clock creation **/ 
	sc_clock scl("clk", 10, SC_NS); 

	/** instaniation of predefined class **/ 
	sc_signal<sc_bit> sda; 

	/** instaniation of top class **/ 
	top top_obj("obj"); 

	/** binding of sda and scl line **/ 
	top_obj.tar->sda(sda); 
	top_obj.tar->scl(scl); 

	/** sda line intialize to high **/ 
	sc_bit b; 
	b = 1; 
	sda.write(b); 

	top_obj.in->portcpu(msp430_proc1.inta); //port binding i2c to cpu
	top_obj.in->portDM(mem.target_export2); //port binding i2c to memory

#ifdef AC_DEBUG
  ac_trace("msp430_proc1.trace");
#endif 

  msp430_proc1.init(ac, av);
  msp430_proc1.set_instr_batch_size(1); //one instruction before switching to other modules
 // msp430_proc1.enable_gdb(); //turn gdb on/off
  cerr << endl;

  sc_start();

  msp430_proc1.PrintStat();
  cerr << endl;

#ifdef AC_STATS
  ac_stats_base::print_all_stats(std::cerr);
#endif 

#ifdef AC_DEBUG
  ac_close_trace();
#endif 

  return msp430_proc1.ac_exit_status;
}
