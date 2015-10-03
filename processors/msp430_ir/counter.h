/**
 * @file   counter.h
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements a timer/counter module
 * 
 */

#ifndef COUNTER_H
#define COUNTER_H
#include <systemc.h>
#include "ac_tlm_protocol.H" //archc for port to cpu
//#include	<tlm_utils/simple_initiator_socket.h> 

#define TACTL_                0x0160    /* Timer A Control */
#define TA0CTL_               0x0160    /* Timer0_A3 Control */

#define MC_1                (0x0010)  /* Timer A mode control: 1 - Up to CCR0 */
#define MC_2                (0x0020)  /* Timer A mode control: 2 - Continous up */

#define TACCR0_               0x0172    /* Timer A Capture/Compare 0 */
#define TA0CCR0_              0x0172    /* Timer0_A3 Capture/Compare 0 */

#define TAIFG               (0x0001)  /* Timer A counter interrupt flag */
#define CCIFG               (0x0001)  /* Capture/compare interrupt flag */
#define CCIE                (0x0010)  /* Capture/compare interrupt enable */
//#define TIMERA1_VECTOR      (0x000A)  /* 0xFFEA Timer A CC1-2, TA */
//#define TIMERA0_VECTOR      (0x000C)  /* 0xFFEC Timer A CC0 */
//#define TIMER0_A1_VECTOR    (0x0010)  /* 0xFFF0 Timer0)A CC1, TA0 */
#define TIMER0_A0_VECTOR    (0xFFF2)//(0xFFF2) //(0x0012)  /* 0xFFF2 Timer0_A CC0 */ Interrupt Vectors (offset from 0xFFE0)

#define TACCTL0_              0x0162    /* Timer A Capture/Compare Control 0 */
#define TA0CCTL0_             0x0162    /* Timer0_A3 Capture/Compare Control 0 */

#define TACLR               (0x0004)  /* Timer A counter clear */

#define TAR_                  0x0170    /* Timer A Counter Register */
#define TA0R_                 0x0170    /* Timer0_A3 */

/** namespace **/ 
//using namespace tlm; 
//using namespace tlm_utils; 

SC_MODULE(Counter) {
  sc_port<ac_tlm_transport_if>  portDM; //port to mem TLM	
  sc_port<ac_tlm_transport_if>  portcpu; //port to cpu TLM
  ac_tlm_req* req; //memory request
  ac_tlm_rsp* rsp; //memory response 

  uint32_t value;
//  uint32_t compare;
 // uint32_t comparezero;
//  bool clear;
  sc_event count_event;
  SC_HAS_PROCESS(Counter);
  Counter(sc_module_name name, int start = 0);
  void count_method();
  void event_trigger_thread();

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

  /** ccifg interrupt **/
  void ccifg_interrupt()
  {
    write_register(TA0CCTL0_ , (read_register(TA0CCTL0_) | CCIFG));
    req->type=WRITE; 
    req->data=TIMER0_A0_VECTOR; 
    portcpu->transport(*req); 
  }

};
#endif // COUNTER_H
