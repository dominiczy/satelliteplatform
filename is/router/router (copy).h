#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

// *********************************************
// Generic payload blocking transport router
// *********************************************

template<unsigned int N_TARGETS>
struct Router: sc_module
{	

  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_target_socket<Router>            target_socket;

  // *********************************************
  // Use tagged sockets to be able to distinguish incoming backward path calls
  // *********************************************

  tlm_utils::simple_initiator_socket_tagged<Router>* initiator_socket[N_TARGETS];

	

  SC_CTOR(Router)
  : target_socket("target_socket")
  {
    // Register callbacks for incoming interface method calls
    target_socket.register_b_transport(       this, &Router::b_transport);

    for (unsigned int i = 0; i < N_TARGETS; i++)
    {
      char txt[20];
      sprintf(txt, "socket_%d", i);
      initiator_socket[i] = new tlm_utils::simple_initiator_socket_tagged<Router>(txt);


    }
  }

 

  // TLM-2 blocking transport method
  virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay ){
	for (int i = 0; i < N_TARGETS; i++) {
		( *initiator_socket[i] )->b_transport( trans, delay ); // send to all targets
	}
  }


  
};


