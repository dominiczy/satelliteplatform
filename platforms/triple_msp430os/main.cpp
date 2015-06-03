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
#include  "counter.h"
#include  "counter.cpp"  
#include  "led.h"
#include  "led.cpp"  

using user::ac_tlm_mem;

int sc_main(int ac, char *av[])
{

  //!  ISA simulator
  msp430 msp430_proc1("msp430_1"); //msp1
	ac_tlm_mem<4>* mem1;
	mem1    = new ac_tlm_mem<4>("mem1",128*1024);
  
  	msp430_proc1.DM_port(*(mem1->target_export[0])); //port binding memory

// instantiate module(s)
	Counter c1("Counter_1", 0);
	c1.portcpu(msp430_proc1.intb);
	c1.portDM(*(mem1->target_export[3])); 

	Led l1("Led_1");
	l1.portDM(*(mem1->target_export[2])); 


  msp430 msp430_proc2("msp430_2"); //msp2
	ac_tlm_mem<2>* mem2;
	mem2    = new ac_tlm_mem<2>("mem2",128*1024);
 
  	msp430_proc2.DM_port(*(mem2->target_export[0])); //port binding memory

  msp430 msp430_proc3("msp430_3"); //msp3
  	ac_tlm_mem<2>* mem3;
	mem3    = new ac_tlm_mem<2>("mem3",128*1024);
 
  	msp430_proc3.DM_port(*(mem3->target_export[0])); //port binding memory

  char *av1[] = {"three_msp430os.x", "--load=demo.x", ""}; //load master program here
  int ac1 = 3;
  char *av2[] = {"three_msp430os.x", "--load=eps.x", ""}; //load slave program here
  int ac2 = 3;
  char *av3[] = {"three_msp430os.x", "--load=ttc.x", ""}; //load slave 2 program here
  int ac3 = 3;


	//sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING); 
/** trace file creation **/ 
	//sc_trace_file *fp = sc_create_vcd_trace_file("wav"); 

  	/** clock creation **/ 
	sc_clock scl("clk", 2500, SC_NS); 

	/** instaniation of predefined class **/ 
	sc_signal<sc_bit> sda; 

/** add signal into trace file **/ 
	//sc_trace(fp, scl, "scl"); 
	//sc_trace(fp, sda, "sda"); 
	//sc_trace(fp, transmit_reg, "tran_reg"); 

		/** instaniation of i2c **/ 
	initiator *in; //multimaster: master and slave
	//target *tar; 

	//initiator *in2; //multimaster
	target *tar2; 

	target *tar3; //slave

	in = new initiator("in"); 
	//in2 = new initiator("in2"); 
	//tar = new target("tar"); 
	tar2 = new target("tar2"); 
	tar3 = new target("tar3"); 
	
	/** binding of sda and scl line **/ 
	in->sda(tar2->sda); 
	in->scl(tar2->scl);

	//tar->sda(tar2->sda); 
	//tar->scl(tar2->scl); 

	//in2->sda(tar2->sda); 
	//in2->scl(tar2->scl); 
		
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
		
	//tar->portcpu(msp430_proc1.intb); 
	//tar->portDM(*(mem1->target_export[1])); 

	//in2->portcpu(msp430_proc2.inta); 
	//in2->portDM(*(mem2->target_export[1])); 
	tar2->portcpu(msp430_proc2.inta); 
	tar2->portDM(*(mem2->target_export[1]));

	tar3->portcpu(msp430_proc3.inta); 
	tar3->portDM(*(mem3->target_export[1]));

	/** initialize router **/
	Router<2,1>* router;
	router    = new Router<2,1>("router");

	// router bindings
	router->target_socket[0]->bind( in->socket );
	//router->target_socket[1]->bind( in2->socket );
	//router->initiator_socket[0]->bind( tar->socket ); //port binding router to i2c slave
	router->initiator_socket[1]->bind( tar2->socket ); //port binding router to i2c slave
	router->initiator_socket[0]->bind( tar3->socket ); //port binding router to i2c slave
#ifdef AC_DEBUG
  ac_trace("msp430_proc1.trace");
#endif 
int batch=1;
  msp430_proc1.init(ac1, av1);
  msp430_proc1.set_instr_batch_size(batch); //one instruction before switching to other modules
// msp430_proc1.enable_gdb(5000); //gdb

  msp430_proc2.init(ac2, av2);
  msp430_proc2.set_instr_batch_size(batch); //one instruction before switching to other modules
//msp430_proc2.enable_gdb(5001); //gdb

  msp430_proc3.init(ac3, av3);
  msp430_proc3.set_instr_batch_size(batch); //one instruction before switching to other modules
//msp430_proc3.enable_gdb(5002); //gdb

  cerr << endl;

  sc_start();

  msp430_proc1.PrintStat();
  msp430_proc2.PrintStat();
  msp430_proc3.PrintStat();
  cerr << endl;

/** close the trace file **/ 
	//sc_close_vcd_trace_file(fp); 

#ifdef AC_STATS
  ac_stats_base::print_all_stats(std::cerr);
#endif 

#ifdef AC_DEBUG
  ac_close_trace();
#endif 

  return msp430_proc1.ac_exit_status+msp430_proc2.ac_exit_status;
}
