#include <msp430g2553.h>
/* Task to be created. */
void vTaskComp( void * pvParameters )
{
	volatile int i;
	WDTCTL = WDTPW | WDTHOLD;
  // set up bit 0 of P1 as output
  P1DIR = 0x01;
  // intialize bit 0 of P1 to 0
  P1OUT = 0x00;
  for( ;; )
  {
	    // toggle bit 0 of P1
    P1OUT ^= 0x01;
      /* Task code goes here. */
	for (i = 0; i < 0x6000; i++);
  }
}


