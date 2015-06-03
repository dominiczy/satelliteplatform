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
const char *archc_options="-gdb -abi";

#include  <iostream>
#include  <systemc.h>
#include  "ac_stats_base.H"
#include  "msp430.H"

int sc_main(int ac, char *av[])
{

  //!  ISA simulator
  msp430 msp430_proc1("msp430_1"); //msp1

  char *av1[] = {"msp430mem.x", "--load=a.out", ""}; //load master program here
  int ac1 = 3;

  	

#ifdef AC_DEBUG
  ac_trace("msp430_proc1.trace");
#endif 

  msp430_proc1.init(ac1, av1);
 // msp430_proc1.set_instr_batch_size(1); //one instruction before switching to other modules
  //msp430_proc1.enable_gdb(5000); //gdb

 

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
