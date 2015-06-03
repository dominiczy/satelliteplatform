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
#include "partest.h"
#include "flash.h"
#include "platform.h"
#include <bsp430/clocks/ucs.h>
#include <bsp430/timers/timerA0.h>
#include <bsp430/utility/console.h>
#include "queue.h"
#include "serial.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define mainLED_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define mainSHOWDCO_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define mainSERIAL_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

static void prvSetupHardware( void );

static void showDCO ()
{
	unsigned portBASE_TYPE ctl0a;
	unsigned portBASE_TYPE ctl0b;
	unsigned long freq_Hz;
		
	portDISABLE_INTERRUPTS();
	freq_Hz = ulBSP430ucsTrimFLL( configCPU_CLOCK_HZ, configCPU_CLOCK_HZ / 128 );
	ctl0a = UCSCTL0;
	do {
		ctl0b = UCSCTL0;
		if (ctl0a == ctl0b) {
			break;
		}
		ctl0a = UCSCTL0;
	} while (ctl0a != ctl0b);
	portENABLE_INTERRUPTS();

	cprintf("UCS: SR 0x%02x RSEL %u DCO %u MOD %u ; freq %lu\n", __read_status_register(), 0x07 & (UCSCTL1 >> 4), 0x1f & (ctl0a >> 8), 0x1f & (ctl0a >> 3), freq_Hz);
}

static portTASK_FUNCTION( vShowDCO, pvParameters )
{
	portTickType xLastWakeTime;
	unsigned long last_ticks;

	( void ) pvParameters;
	xLastWakeTime = xTaskGetTickCount();
	last_ticks = ulBSP430timerA0Ticks();

	for(;;)
	{
		unsigned long ticks;
		
		vTaskDelayUntil( &xLastWakeTime, 3000 );
		ticks = ulBSP430timerA0Ticks();
		showDCO();
		cprintf("%lu ticks since last wake, uptime %lu seconds\n", (ticks - last_ticks), ticks / 32768);
		last_ticks = ticks;
	}
}

static xComPortHandle hsuart;

static portTASK_FUNCTION( vSerialStuff, pvParameters )
{
	unsigned int nrx = 0;
	( void ) pvParameters;
	hsuart = xSerialPortInit(serCOM1, ser115200, serNO_PARITY, serBITS_8, serSTOP_1, 16);
	portENABLE_INTERRUPTS();
	for(;;)
	{
		portBASE_TYPE rv;
		signed char c;

		if (xSerialGetChar(hsuart, &c, 5000)) {
			++nrx;
			rv = xSerialPutChar(hsuart, c, 0);
			if (pdTRUE != rv) {
				cprintf("\nSERIAL PUT failed\n");
			}
		} else {
			cprintf("Serial woke without rx, nrx %u\n", nrx);
			cprintf("UCA0: STAT %02x IFG %02x IE %02x\n", UCA0STAT, UCA0IFG, UCA0IE);
		}
	}
}

int main( void )
{
	prvSetupHardware();
	vParTestInitialise();

	cprintf("Up and running"
		   "\n20-bit aware: "
#if __MSP430X__ & ( __MSP430_CPUX_TARGET_SR20__ | __MSP430_CPUX_TARGET_ISR20__ )
		   "YES"
#else
		   "no"
#endif
		   "; calls 20-bit: "
#if __MSP430X__ & __MSP430_CPUX_TARGET_C20__
		   "YES"
#else
		   "no"
#endif
		   "; data 20-bit: "
#if __MSP430X__ & __MSP430_CPUX_TARGET_D20__
		   "YES"
#else
		   "no"
#endif
		   "\nPreemption "
#if configUSE_PREEMPTION
		   "ON"
#else
		   "OFF"
#endif
		   "\n");
	showDCO();
	
	vStartLEDFlashTasks( mainLED_TASK_PRIORITY );

	xTaskCreate( vShowDCO, ( signed char * ) "SDCO", 300, NULL, mainSHOWDCO_TASK_PRIORITY, ( xTaskHandle * ) NULL );
	xTaskCreate( vSerialStuff, ( signed char * ) "Serial", 300, NULL, mainSERIAL_TASK_PRIORITY, ( xTaskHandle * ) NULL );

	/* Start the scheduler. */
	vTaskStartScheduler();
}

void vApplicationIdleHook( void ) { }

static void prvSetupHardware( void )
{
	vBSP430platformSetup();

	/* Enable console */
	xConsoleConfigure(xSerialPortInit(serCOM2, ser9600, serNO_PARITY, serBITS_8, serSTOP_1, 0),
					  500);

	/* P11.0: ACLK ; P11.1: MCLK; P11.2: SMCLK ; all available on test
	 * points */
	P11SEL |= BIT0 | BIT1 | BIT2;
	P11DIR |= BIT0 | BIT1 | BIT2;
}
