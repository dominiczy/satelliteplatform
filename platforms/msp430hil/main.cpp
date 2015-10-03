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
const char *archc_options="-gdb";

#include  <iostream>
#include  <systemc.h>
#include  "ac_stats_base.H"
#include  "msp430.H"
#include  "ac_tlm_mem.h"
#include  "ac_tlm_mem.cpp" 
#include  "master.cpp"
#include  "slave.cpp" 

using user::ac_tlm_mem;

int sc_main(int ac, char *av[])
{

  //!  ISA simulator
  msp430 msp430_proc1("msp430_1"); //msp1
	ac_tlm_mem<3>* mem1;
	mem1    = new ac_tlm_mem<3>("mem1",128*1024);
  
  	msp430_proc1.DM_port(*(mem1->target_export[0])); //port binding memory

	initiator *in; //multimaster: master and slave
	target *tar; 
	in = new initiator("in"); 
	tar = new target("tar"); 

  char *av1[] = {"msp430hil.x", "--load=i2cmastertransmit.x", ""}; //load master program here
  int ac1 = 3;
	
	/** clock creation **/ 
	sc_clock scl("clk", 10, SC_NS); //2500?

	/** instaniation of predefined class **/ 
	sc_signal<sc_bit> sda; 
  	
	in->sda(tar->sda); 
	in->scl(tar->scl);
	tar->sda(sda); 
	tar->scl(scl); 

	/** sda line intialize to high **/ 
	sc_bit b; 
	b = 1; 
	sda.write(b); 

	// i2c port bindings
	in->portcpu(msp430_proc1.inta); 
	in->portDM(*(mem1->target_export[1])); 

	in->socket->bind( tar->socket );

#ifdef AC_DEBUG
  ac_trace("msp430_proc1.trace");
#endif 

  msp430_proc1.init(ac1, av1);
  msp430_proc1.set_instr_batch_size(1); //one instruction before switching to other modules
// msp430_proc1.enable_gdb(5000); //gdb

 

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
