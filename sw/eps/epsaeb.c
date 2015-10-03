/**
 * @file   ttc.c
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements transmission and receiving of data for ttc subsystem
 * 	   	I2C routines based on CCS example code by
 *		B. Nisarga
 *  		Texas Instruments Inc.
 * 		September 2007
 * 
 */

#include <msp430.h>


unsigned char *PTxData;                     // Pointer to TX data
int i;

int main(void)
{
unsigned char TxData[47] = {124,122,120,117,115,113,111,109,107,104,103,100,99,97,96,93,92,91,88,87,86,84,83,81,80,78,77,77,75,74,74,72,72,71,70,69,69,68,67,66,69,66,72,71,70,75};
unsigned char TxData1[47] = {78,77,79,81,84,85,87,88,89,92,93,93,94,94,95,95,95,95,94,94,93,91,90,88,87,85,83,82,81,79,78,77,74,73,72,75,74,73,77,80,81,83,86,87,88,89};
unsigned char TxData2[47] = {89,89,89,89,89,88,87,86,85,84,83,80,79,77,76,75,73,71,70,68,67,65,63,60,58,57,60,58,57,61,65,67,68,71,73,74,75,78,79,81,82,83,84,86,87,88};
unsigned char TxData3[47] = {89,89,89,89,89,89,88,87,85,83,82,80,77,76,74,73,72,74,73,78,79,81,83,84,85,87,90,91,92,93,94,95,95,95,95,94,94,93,92,91,89,86,85,84,81,79};
unsigned char TxData4[47] = {78,77,74,73,71,70,69,66,67,67,68,68,69,70,70,71,72,73,73,75,75,76,77,78,79,81,82,83,84,85,87,88,90,92,94,97,98,99,101,103,105,106,108,111,112,115};
unsigned char TxData5[5] = {117,119,121,120,124};
 WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
  UCB0I2COA = 12;                         // Own Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  UCB0I2CIE |= UCSTPIE + UCSTTIE;           // Enable STT and STP interrupt
  IE2 |= UCB0TXIE;                          // Enable TX interrupt
  IE2 |= UCB0RXIE;                          // Enable RX interrupt
  PTxData = (unsigned char *)TxData;
  i=1;
  while (1)
  {
	__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
	

    //__no_operation();                       // Set breakpoint >>here<< and
  }                                         // read out the TXByteCtr counter
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
	UCB0TXBUF = *PTxData;                      // TX data
	if (i==1) {
		PTxData++;
		i = 0;
	} else {
		i = 1;
	}

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
