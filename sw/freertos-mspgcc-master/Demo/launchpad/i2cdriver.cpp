#include "i2cdriver.h"

void prvSetupI2C( void )
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	  P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
	  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	  UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
	  UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	IE2 |= UCB0RXIE;  
	IE2 |= UCB0TXIE;                          // Enable TX interrupt
}

void transmit(int data, int address){
	UCB0I2CSA = address;
	TXData=data;
	transmit=1;
    	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
	while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
}

int receive(int address){
		UCB0I2CSA = address;
		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
		UCB0CTL1 &= !(UCTR);
	    	UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	    	while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
	    	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
		return RXData;
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
		UCB0TXBUF = TXData;                 // Load TX buffer
	} else { // receive interrupt handling
		RXData = UCB0RXBUF;                  // Get RX data
	}
	IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
	__bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
}
