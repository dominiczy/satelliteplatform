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
unsigned char RXData;
unsigned char TXData;

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
	
	int wakeup=1;
	int sleep=2;
	int slaveaddress;
	int eps=2;
	int ttc=3;
	while (1) { 
		// Wake up EPS
		slaveaddress=eps;
		TXData=wakeup;
		transmit=1;
	  	UCB0I2CSA = slaveaddress;                         // Slave Address is 048h
    	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
		while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	
		// Receive data from EPS
		transmit=0;
		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
		UCB0CTL1 &= !(UCTR);
	    UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	    while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
	    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	    	
		// Put EPS to sleep
		TXData=sleep;
		transmit=1;
    	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
		while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
   		
		// Wake up TTC
		slaveaddress=ttc;
		TXData=wakeup;
		transmit=1;
	  	UCB0I2CSA = slaveaddress;                         // Slave Address is 048h
    	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
		while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	
		// Receive data from TTC
		transmit=0;
		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
		UCB0CTL1 &= !(UCTR);
	    UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	    while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
	    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	    	
		// Put TTC to sleep
		TXData=sleep;
		transmit=1;
    	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
		while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
   		
	}
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
		UCB0TXBUF = TXData;                     // Load TX buffer
	} else { // receive interrupt handling
		RXData = UCB0RXBUF;                       // Get RX data
	}
	IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
	__bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
}
