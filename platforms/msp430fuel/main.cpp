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
#include  "ac_tlm_mem.cpp"  
#include  "counter.h"
#include  "counter.cpp"  

using user::ac_tlm_mem;

int sc_main(int ac, char *av[])
{

  //!  ISA simulator
  msp430 msp430_proc1("msp430_1"); //msp1
	ac_tlm_mem<2>* mem1;
	mem1    = new ac_tlm_mem<2>("mem1",128*1024);
  
  	msp430_proc1.DM_port(*(mem1->target_export[0])); //port binding memory

// instantiate module(s)
	Counter c1("Counter_1", 0);
	c1.portcpu(msp430_proc1.inta);
	c1.portDM(*(mem1->target_export[1])); 

	

  char *av1[] = {"three_msp430os.x", "--load=fluel.x", ""}; //load master program here
  int ac1 = 3;



#ifdef AC_DEBUG
  ac_trace("msp430_proc1.trace");
#endif 
int batch=1;
  msp430_proc1.init(ac1, av1);
  msp430_proc1.set_instr_batch_size(batch); //one instruction before switching to other modules
 // msp430_proc1.enable_gdb(5000); //gdb


  cerr << endl;

  sc_start();

  msp430_proc1.PrintStat();
  cerr << endl;

/** close the trace file **/ 
	//sc_close_vcd_trace_file(fp); 

#ifdef AC_STATS
  ac_stats_base::print_all_stats(std::cerr);
#endif 

#ifdef AC_DEBUG
  ac_close_trace();
#endif 

  return msp430_proc1.ac_exit_status;
}
