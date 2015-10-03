/**
 * @file   main.c
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements various tasks for obdh subsystem
 * 	   	FreeRTOS original main file by
 *		Peter A. Bigot <bigotp@acm.org>
 *  		Copyright (c) 2012
 * 
 */

#include "FreeRTOS.h"
#include "task.h"
#include "i2cdriver.h"
#include <msp430.h>

static void prvSetupHardware( void );

#define eps 12
#define ttc 13

unsigned char *ptc;     
unsigned char *ptm;               
unsigned char tm[10]= {0};
unsigned char tc[10]= {0};

void prvTaskreceiveTM (void* pvParameters) 
{
    (void) pvParameters;                    // Just to stop compiler warnings.
    for (;;) {
	*ptm++=receive(eps);
        vTaskDelay(8);	
    }
}


void prvTaskreceiveTC (void* pvParameters) 
{
    (void) pvParameters;                    // Just to stop compiler warnings.
    for (;;) {
	*ptc++=receive(ttc);
        vTaskDelay(8);	
    }
}

void prvTaskprocessData (void* pvParameters) 
{
    (void) pvParameters;                    // Just to stop compiler warnings.
    for (;;) {
        vTaskDelay(4);	
    }
}

void main( void )
{

	ptm = (unsigned char *) tm;
	ptc = (unsigned char *) tc;
	prvSetupI2C();

	xTaskCreate( prvTaskreceiveTM, ( signed char * ) "receiveTM", configMINIMAL_STACK_SIZE, NULL,
        2, ( xTaskHandle * ) NULL );
	xTaskCreate( prvTaskreceiveTC, ( signed char * ) "receiveTC", configMINIMAL_STACK_SIZE, NULL,
        3, ( xTaskHandle * ) NULL );
	xTaskCreate( prvTaskprocessData, ( signed char * ) "processData", configMINIMAL_STACK_SIZE, NULL,
        0, ( xTaskHandle * ) NULL );

	/* Start the scheduler. */
	vTaskStartScheduler();
}



void vApplicationIdleHook( void ) // this is disabled, coroutines not used
{
	/* Schedule the co-routines. */
	for( ;; )
	{
		vCoRoutineSchedule();
	}
}

void test430_bsp_init_putchar ()
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




