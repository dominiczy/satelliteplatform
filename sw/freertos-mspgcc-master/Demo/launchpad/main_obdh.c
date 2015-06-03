/*
  
Copyright (c) 2012, Peter A. Bigot <bigotp@acm.org>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the software nor the names of its contributors may be
  used to endorse or promote products derived from this software without
  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include "FreeRTOS.h"
#include "task.h"
//#include "croutine.h"
//#include "clocks/bc2.h"
#include <msp430.h>
//#include <stdbool.h>
//#include <stdio.h>
//#include "integer.c"
//#include "i2c.h"
//#include "comp.h"
#define P1OUT_                0x0021    /* Port 1 Output */

/* The number of flash co-routines to create. */
#define mainNUM_FLASH_CO_ROUTINES       ( 2 )

static void prvSetupHardware( void );
static void prvSetupI2C( void );

int rv;
#define wakeup 1;
#define sleep 2;
#define eps 12;
#define ttc 13;
//bool transmit;
unsigned char RXData;
unsigned char TXData;

void prvTaskOBDHtoEPS (void* pvParameters) // OBDH to EPS routine
{		
    (void) pvParameters;                    // Just to stop compiler warnings.
	
    for (;;) {

	// Wake up EPS
		P1OUT ^= BIT0;
	  	UCB0I2CSA = eps;                         // Slave Address is 048h
		TXData=wakeup;
		//transmit=1;
    		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    		UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
		while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	
		// Receive data from EPS
		//transmit=0;
		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
		UCB0CTL1 &= !(UCTR);
	    	UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	    	while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
	    	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	    	
		// Put EPS to sleep
		TXData=sleep;
		//transmit=1;
    		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    		UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
		while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
        vTaskDelay(4);
    }
}

void prvTaskOBDHtoTTC (void* pvParameters) // OBDH to TTC routine
{
    (void) pvParameters;                    // Just to stop compiler warnings.

    for (;;) {
     // Wake up TTC
		P1OUT ^= BIT6;
		UCB0I2CSA = ttc;                         // Slave Address is 048h
		TXData=wakeup;
		//transmit=1;
    		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    		UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
		while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	
		// Receive data from TTC
		//transmit=0;
		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
		UCB0CTL1 &= !(UCTR);
	    	UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	    	while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
	    	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	    	
		// Put TTC to sleep
		TXData=sleep;
		//transmit=1;
    		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    		UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
		while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
        vTaskDelay(5);
    }
}

void main( void )
{
	prvSetupI2C();
	//prvSetupHardware();
	//vParTestInitialise();

	//printf("\nStarting up CPU %lu: SR %04x IFG1 %02x\n", configCPU_CLOCK_HZ, __read_status_register(), IFG1);
	
	//vStartFlashCoRoutines( mainNUM_FLASH_CO_ROUTINES );
	//vStartIntegerMathTasks( 1);	
	//xCoRoutineCreate( vI2CCoRoutine, 1, 0 );
	//xTaskCreate( vTaskComp, "comp", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	//xTaskCreate( vTaskI2C, "i2c", configMINIMAL_STACK_SIZE, NULL, 2, NULL );

	 xTaskCreate( prvTaskOBDHtoEPS, ( signed char * ) "TaskOBDHtoEPS", configMINIMAL_STACK_SIZE, NULL,
        2, ( xTaskHandle * ) NULL );
    	xTaskCreate( prvTaskOBDHtoTTC, ( signed char * ) "TaskOBDHtoTTC", configMINIMAL_STACK_SIZE, NULL, 
        2, ( xTaskHandle * ) NULL );


	/* Start the scheduler. */
	vTaskStartScheduler();
}

static void prvSetupI2C( void )
{
		 WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	  P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
	  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	  UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
	  UCB0BR1 = 0;
	//UCB0I2COA = 1;                         // Own Address is 048h
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	IE2 |= UCB0RXIE;  
	IE2 |= UCB0TXIE;                          // Enable TX interrupt
	//UCB0I2CSA=eps;
	TXData=wakeup;

}

void vApplicationIdleHook( void ) // this is disabled, coroutines not used
{
	/* Schedule the co-routines. */
	for( ;; )
	{
		vCoRoutineSchedule();
	}
}

void
test430_bsp_init_putchar ()
{
	unsigned short usBR;
	unsigned char ucBRF = 0;
	unsigned char ucBRS = 0;
	
	UCA0CTL1 |= UCSWRST;
	UCA0CTL1 |= UCSSEL_2;
#if 1000000 == configCPU_CLOCK_HZ
	/* 1 MHz SMCLK */
	usBR = 104;
	ucBRS = 1;
#elif 12000000 == configCPU_CLOCK_HZ
	/* 12 MHz SMCLK */
	usBR = 1250;
#else
	/* 4 MHz SMCLK */
	usBR = 416;
	ucBRS = 6;
#endif
	UCA0BR0 = usBR & 0xFF;
	UCA0BR1 = usBR >> 8;
	UCA0MCTL = (ucBRF * UCBRF_1) + (ucBRS * UCBRS_1);
#if __MSP430G2553__
	/* P1.1 = UCA0RXD, P1.2 = UCA0TXD */
	P1SEL |= BIT1 | BIT2;
	P1SEL2 |= BIT1 | BIT2;
#else /* MCU */
#endif /* MCU */
	UCA0CTL1 &= ~UCSWRST;
}

int putchar (int c)
{
  while (!(UC0IFG & UCA0TXIFG)) {
    ;
  }
  UCA0TXBUF = c;
  return c;
}

static void prvSetupHardware( void )
{
	unsigned char ucDCOCTL;
	unsigned char ucBCSCTL1;
	unsigned char ucBCSCTL2;
	unsigned char ucBCSCTL3;

	WDTCTL = WDTPW + WDTHOLD;
	IFG1 = 0;

#if __MSP430G2553__
	/* P2.6 = XIN, P2.7 = XOUT */
	P2DIR &= ~BIT6;
	P2DIR |= BIT7;
	P2SEL |= ( BIT6 | BIT7 );
	P2SEL2 &= ~ ( BIT6 | BIT7 );
#else /* MCU */
#warning Unable to configure XIN/XOUT on unrecognized MCU
#endif /* MCU */
	ucBCSCTL3 = XCAP_1;
#if 16000000 == configCPU_CLOCK_HZ
	/* 16 MHz MCLK, 4 MHz SMCLK */
	ucBCSCTL1 = CALBC1_16MHZ;
	ucDCOCTL = CALDCO_16MHZ;
	ucBCSCTL2 = DIVS_2;
#elif 12000000 == configCPU_CLOCK_HZ
	/* 12 MHz MCLK, 12 MHz SMCLK */
	ucBCSCTL1 = CALBC1_12MHZ;
	ucDCOCTL = CALDCO_12MHZ;
	ucBCSCTL2 = DIVS_0;
#elif 8000000 == configCPU_CLOCK_HZ
	/* 8 MHz MCLK, 4 MHz SMCLK */
	ucBCSCTL1 = CALBC1_8MHZ;
	ucDCOCTL = CALDCO_8MHZ;
	ucBCSCTL2 = DIVS_1;
#elif 1000000 == configCPU_CLOCK_HZ
	/* 1 MHz MCLK, 1 MHz SMCLK */
	ucBCSCTL1 = CALBC1_1MHZ;
	ucDCOCTL = CALDCO_1MHZ;
	ucBCSCTL2 = DIVS_0;
#endif /* configCPU_CLOCK_HZ */

	if ( pdFALSE == ucBSP430bc2Configure( ucDCOCTL, ucBCSCTL1, ucBCSCTL2, ucBCSCTL3 ) ) {
		/* No crystal: output, port function */
#if __MSP430G2553__
		P2DIR |= BIT6 | BIT7;
		P2SEL &= ~( BIT6 | BIT7 );
#else /* MCU */
#endif /* MCU */
	}

	test430_bsp_init_putchar ();
}

#include "utility/led.h"

const xLEDDefn pxLEDDefn[] = {
	{ .pucPxOUT = &P1OUT, .ucBIT = BIT0 }, /* Red */
	{ .pucPxOUT = &P1OUT, .ucBIT = BIT6 }, /* Green */
};
const unsigned char ucLEDDefnCount = sizeof(pxLEDDefn) / sizeof(*pxLEDDefn);



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
	P1OUT ^= BIT6;
	P1OUT ^= BIT0;
	if (IFG2 & UCB0TXIFG) { // transmit interrupt handling
		UCB0TXBUF = TXData;   
		P1OUT ^= BIT6;                  // Load TX buffer
	} else { // receive interrupt handling
		RXData = UCB0RXBUF;   
		P1OUT ^= BIT0;                    // Get RX data
	}
	IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
	__bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
}
