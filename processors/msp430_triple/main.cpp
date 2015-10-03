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
#include "master.cpp" 
#include "slave.cpp" 
#include "router.h"

using user::ac_tlm_mem;

int sc_main(int ac, char *av[])
{

	//!  ISA simulator
	msp430 msp430_proc1("msp430_1"); //msp1
	ac_tlm_mem<3>* mem1;
	mem1    = new ac_tlm_mem<3>("mem1",128*1024);
  	msp430_proc1.DM_port(*(mem1->target_export[0])); //port binding memory

	msp430 msp430_proc2("msp430_2"); //msp2
	ac_tlm_mem<2>* mem2;
	mem2    = new ac_tlm_mem<2>("mem2",128*1024);
  	msp430_proc2.DM_port(*(mem2->target_export[0])); //port binding memory

	msp430 msp430_proc3("msp430_3"); //msp3
  	ac_tlm_mem<2>* mem3;
	mem3    = new ac_tlm_mem<2>("mem3",128*1024);
  	msp430_proc3.DM_port(*(mem3->target_export[0])); //port binding memory

	char *av1[] = {"three_msp430.x", "--load=obdh.x", ""}; //load master program here
	int ac1 = 3;
	char *av2[] = {"three_msp430.x", "--load=eps.x", ""}; //load slave program here
	int ac2 = 3;
	char *av3[] = {"three_msp430.x", "--load=ttc.x", ""}; //load slave 2 program here
	int ac3 = 3;

  	/** clock, data line creation **/ 
	sc_clock scl("clk", 10, SC_NS); //2500?
	sc_signal<sc_bit> sda; 

	/** instaniation of i2c **/ 
	initiator *in; //multimaster: master and slave
	target *tar; 
	target *tar2; 
	target *tar3; //slave

	in = new initiator("in"); 
	tar = new target("tar"); 
	tar2 = new target("tar2"); 
	tar3 = new target("tar3"); 
	
	/** binding of sda and scl line **/ 
	in->sda(tar->sda); 
	in->scl(tar->scl);

	tar->sda(tar2->sda); 
	tar->scl(tar2->scl); 
		
	tar2->sda(tar3->sda); 
	tar2->scl(tar3->scl); 

	tar3->sda(sda); 
	tar3->scl(scl); 
	

	/** sda line intialize to high **/ 
	sc_bit b; 
	b = 1; 
	sda.write(b); 
 
	// i2c port bindings
	in->portcpu(msp430_proc1.inta); 
	in->portDM(*(mem1->target_export[1])); 
		
	tar->portcpu(msp430_proc1.intb); 
	tar->portDM(*(mem1->target_export[2])); 

	tar2->portcpu(msp430_proc2.inta); 
	tar2->portDM(*(mem2->target_export[1]));

	tar3->portcpu(msp430_proc3.inta); 
	tar3->portDM(*(mem3->target_export[1]));

	/** initialize router **/
	Router<3,1>* router;
	router    = new Router<3,1>("router");

	// router bindings
	router->target_socket[0]->bind( in->socket );
	router->initiator_socket[0]->bind( tar->socket ); //port binding router to i2c slave
	router->initiator_socket[1]->bind( tar2->socket ); //port binding router to i2c slave
	router->initiator_socket[2]->bind( tar3->socket ); //port binding router to i2c slave

#ifdef AC_DEBUG
  ac_trace("msp430_proc1.trace");
#endif 
	int batch=1;
	msp430_proc1.init(ac1, av1);
	msp430_proc1.set_instr_batch_size(batch); //one instruction before switching to other modules
	msp430_proc1.enable_gdb(5000); //gdb

	msp430_proc2.init(ac2, av2);
	msp430_proc2.set_instr_batch_size(batch); //one instruction before switching to other modules
	msp430_proc2.enable_gdb(5001); //gdb

	msp430_proc3.init(ac3, av3);
	msp430_proc3.set_instr_batch_size(batch); //one instruction before switching to other modules
	msp430_proc3.enable_gdb(5002); //gdb

	cerr << endl;

	sc_start();

	msp430_proc1.PrintStat();
	msp430_proc2.PrintStat();
	msp430_proc3.PrintStat();
	cerr << endl;

#ifdef AC_STATS
	ac_stats_base::print_all_stats(std::cerr);
#endif 

#ifdef AC_DEBUG
	ac_close_trace();
#endif 

	return msp430_proc1.ac_exit_status+msp430_proc2.ac_exit_status;
}
