/**
 * @file   eps.c
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements transmission and receiving of data for eps subsystem
 * 	   	I2C routines based on CCS example code by
 *		B. Nisarga
 *  		Texas Instruments Inc.
 * 		September 2007
 * 
 */

#include <msp430.h>

unsigned char TXData;
unsigned char RXData;
unsigned char state;

int main(void)
{
  state=2; 									// State receive
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
  UCB0I2COA = 12;                           // Own Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  UCB0I2CIE |= UCSTPIE + UCSTTIE;           // Enable STT and STP interrupt
  IE2 |= UCB0TXIE;                          // Enable TX interrupt
  IE2 |= UCB0RXIE;                          // Enable RX interrupt
  TXData = 0xff; 
  while (1)
  {
	if (state==1){ 							// Do operations
		TXData = (TXData-1)%127;            // Calculate TX data
	} 
	__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
    //__no_operation();                     
  }                                         
}

//------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move data from MSP430 memory to the
// I2C master. PTxData points to the next byte to be transmitted, and TXByteCtr
// keeps track of the number of bytes transmitted.
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
	UCB0TXBUF = TXData;                       // TX data
	RXData = UCB0RXBUF; 
	state=1;
	__bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
}

//------------------------------------------------------------------------------
// The USCI_B0 state ISR is used to wake up the CPU from LPM0 in order to do
// processing in the main program after data has been transmitted. LPM0 is
// only exit in case of a (re-)start or stop condition when actual data
// was transmitted.
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCIAB0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
	UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);       // Clear interrupt flags
    __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0 if data was
}                                           // transmitted
