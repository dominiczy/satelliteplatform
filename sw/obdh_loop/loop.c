/**
 * @file   obdh.c
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements transmission and receiving of data for obdh subsystem
 * 	   	I2C routines based on CCS example code by
 *		B. Nisarga
 *  		Texas Instruments Inc.
 * 		September 2007
 * 
 */

#include <msp430.h>
#include <stdbool.h>
#define datasize 255 //256 bytes per packet
bool transmit;
bool transexit;
unsigned char *PTxData;                     // Pointer to TX data
unsigned char TXByteCtr;
unsigned char TxData[datasize];             // Table of data to transmit

unsigned char *PRxData;                     // Pointer to RX data
unsigned char RXByteCtr;
volatile unsigned char RxBuffer[datasize];       // Allocate 128 byte of RAM  //takes too long



void mastertransmit(unsigned char * PTxData, int slaveaddress)
{
	 transmit=1;
	  UCB0I2CSA = slaveaddress;                         // Slave Address is 048h
	  IE2 |= UCB0TXIE;                          // Enable TX interrupt

	  while (1) 
	  //int i;
	  //for ( i = 0; i < 5; i++ ) 
	  {
	    PTxData = (unsigned char *)TxData;      // TX array start address
	    TXByteCtr = sizeof TxData;              // Load TX byte counter
	    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent

	    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
	   // if (transexit) break;
	    //if (TXByteCtr == 5) break;
	    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
		                                    // Remain in LPM0 until all data
		if (transexit) break;                                    // is TX'd
	  }
	  //__bic_SR_register(CPUOFF);
}

unsigned char * masterreceive(int slaveaddress)
{
  	transmit=0;
  UCB0I2CSA = slaveaddress;                         // Slave Address is 048h
  IE2 |= UCB0RXIE;                          // Enable RX interrupt

  while (1)
  {
    PRxData = (unsigned char *)RxBuffer;    // Start of RX buffer
    RXByteCtr = datasize;                          // Load RX byte counter
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
	UCB0CTL1 &= !(UCTR);
    UCB0CTL1 |= UCTXSTT;                    // I2C start condition
    if (PRxData[datasize-1] != 0) break;
	
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
                                            // Remain in LPM0 until all data
                                       // is RX'd
    __no_operation();                       // Set breakpoint >>here<< and
     
  }          
  //__bic_SR_register(CPUOFF);  // change loop to prevent lpm                              // read out the RxBuffer buffer
  return PRxData;
}


int main(void)
{	
	 WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	  P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
	  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	  UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
	  UCB0BR1 = 0;
	UCB0I2COA = 50;                         // Own Address is 048h
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
 	//unsigned int i = 0;
	int slaveaddress;
	//for (i = 0; i < 1; i++) { // send 255 bytes
		slaveaddress=51;
		PTxData= masterreceive(slaveaddress);
		slaveaddress=52;
		mastertransmit(PTxData, slaveaddress);
		
	//}
}


// interrupt routines
//------------------------------------------------------------------------------
// The USCIAB0TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCIAB0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
	if (transmit) { // transmit interrupt handling
		  if (TXByteCtr)                            // Check TX byte counter
		  {
		    UCB0TXBUF = *PTxData++;                 // Load TX buffer
		    TXByteCtr--;                            // Decrement TX byte counter
		  }
		  else
		  {
			transexit=1;
		    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
		    IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
		    __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
		  }
	} else { // receive interrupt handling
		  RXByteCtr--;                              // Decrement RX byte counter
		  if (RXByteCtr)
		  {
		    *PRxData++ = UCB0RXBUF;                 // Move RX data to address PRxData
		    if (RXByteCtr == 1)                     // Only one byte left?
		      UCB0CTL1 |= UCTXSTP;                  // Generate I2C stop condition
		  }
		  else
		  {
		    *PRxData = UCB0RXBUF;                   // Move final RX data to PRxData
		    __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
		  }
	}
}
