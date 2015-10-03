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
#define eps 48
#define ttc 13

bool transmit;
unsigned char ttcdata[240];
unsigned char epsdata[240];
unsigned char *pttc;                   // Pointer to TX data
unsigned char *peps;                     // Pointer to TX data
int slaveaddress;

int main(void)
{	
	 WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	  P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
	  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	  UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
	  UCB0BR1 = 0;
	UCB0I2CSA=2;
	UCB0I2COA = 4;                         // Own Address is 048h
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	IE2 |= UCB0RXIE;  
	IE2 |= UCB0TXIE;                          // Enable TX interrupt
	
	pttc = (unsigned char *) ttcdata;
	peps = (unsigned char *) epsdata;
	int a;
	for( a = 0; a < 240; a = a + 1 ) { 
		// Wake up EPS
		slaveaddress=eps;
		UCB0I2CSA = slaveaddress;                         // Slave Address is 048h
		// Receive data from EPS
		transmit=0;
		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
		UCB0CTL1 &= !(UCTR);
	    	UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	    	while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
	    	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	    	
		   		
		// Wake up TTC
		slaveaddress=ttc;
		UCB0I2CSA = slaveaddress;                         // Slave Address is 048h	
		// Receive data from TTC
		transmit=0;
		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
		UCB0CTL1 &= !(UCTR);
	    	UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	    	while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
	    	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	    	
   		
	}
	__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
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
	if (slaveaddress==eps){
		*peps++ = UCB0RXBUF;                       // Get RX data
	} else {
		*pttc++ = UCB0RXBUF; 
	}
	
	IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
	__bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
}
