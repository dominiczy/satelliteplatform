#ifndef RWMETHODS_H
#define RWMETHODS_H
#include "ac_tlm_protocol.H" // for port to cpu

	ac_tlm_req* req; //memory request
	ac_tlm_rsp* rsp; //memory response
  /** read register **/
  uint32_t read_register(uint32_t address)
  {
    req->type=READ;
    req->addr=address;
    *rsp=portDM ->transport(*req); 
    return (rsp->data);
  }

  /** write register **/
  void write_register(uint32_t address, uint32_t data)
  {
    req->type=WRITE;
    req->addr=address;
    req->data=data;
    *rsp=portDM ->transport(*req); 
  }


#endif // RWMETHODS_H
