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

 unsigned char TxData[47] = {133,131,125,134,140,138,141,144,146,150,151,154,156,157,159,162,164,165,168,169,170,172,173,176,176,178,179,180,181,182,183,184,185,185,187,188,188,189,190,190,191,192,193,193,194,194};
  unsigned char TxData1[47] = {195,195,196,196,197,197,197,198,196,190,187,184,182,179,176,171,170,168,167,165,163,161,159,158,157,155,153,152,151,151,150,150,150,150,150,150,150,151,151,152,152,153,156,164,161,171};
  unsigned char TxData2[47] = {168,166,178,175,172,180,190,187,197,194,203,200,202,198,196,193,187,186,186,186,186,186,186,186,186,186,186,186,186,186,187,189,193,198,196,200,202,202,199,197,194,190,187,183,180,177};
  unsigned char TxData3[47] = {174,170,168,165,162,159,157,156,153,152,152,151,151,150,150,150,150,150,150,151,152,152,153,155,157,158,161,164,162,165,169,172,170,173,174,178,183,189,186,198,195,190,198,197,196,196};
  unsigned char TxData4[47] = {196,195,194,194,194,193,193,192,191,191,190,189,189,188,187,187,186,185,185,183,182,182,181,179,178,177,176,175,174,173,170,169,168,165,164,161,160,158,156,153,151,148,146,143,141,139};  
  unsigned char TxData5[5] ={136,134,130,127,126};
WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
  UCB0I2COA = 13;                         // Own Address is 048h
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
