/*
 * File: ert_main.c
 *
 * Code generated for Simulink model 'fuel_rate_control'.
 *
 * Model version                  : 1.534
 * Simulink Coder version         : 8.6 (R2014a) 27-Dec-2013
 * C/C++ source code generated on : Thu Aug 20 17:04:08 2015
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Texas Instruments->MSP430
 * Code generation objectives: Unspecified
 * Validation result: Not run
 *			  
 *			  
 *			  
 *	 _____ _       	       	_      	       	   __ _
 *	|_   _(_)_ __  ___ _ _ /_\    __ ___ _ _  / _(_)__ _
 *	  | | |	| '  \/	-_) '_/	_ \  / _/ _ \ '	\|  _| / _` |
 *	  |_| |_|_|_|_\___|_|/_/ \_\ \__\___/_||_|_| |_\__, |
 	       	       	       	       	       	       |___/

 * TACTL register
 *	+--------+--------+--------+--------+
 *	|x x x x |x x x x |x x x x |x x x x |
 *	+--------+--------+--------+--------+
 *     	-----+------- -+-  -+- -+-  | | | |
 *	     |	       |    |	|   | | | +-------- TAIFG -> Interrupt flag (0 no interrupt pending/ 1 interrupt pending)a
 *	     |	       |    |	|   | | |
 *	     |	       |    |	|   | |	+---------- TAIE  -> Interrupt Enable, This bit enables the TAIFG interrupt request
 *	     | 	       |    |   |   | |
 *	     | 	       |    |   |   | +------------ TACLR -> Timer_A clear. Setting this bit resets TAR, the clock divider,
             |         |    |   |   |                        and the count direction. The TACLR bit is automatically reset
	     | 	       |    |	|   | 			     and is always read as zero.
	     |	       |    |	|   |
 *	     | 	       |    |   |   |
 *	     | 	       |    |   |  -+-------------- unused
 *	     | 	       |    | 	|
 *	     | 	       |    | 	+------------------ MCx Mode control. Setting MCx = 00h when Timer_A is not in use conserves power.
             |         |    |                                | 00 | Stop mode: the timer is halted.                                 |
             |         |    |                                | 01 | Up mode: the timer counts up to TACCR0.                         |
             |         |    |                                | 10 | Continuous mode: the timer counts up to 0FFFFh.                 |
             |         |    |                                | 11 | Up/down mode: the timer counts up to TACCR0 then down to 0000h. |
	     |	       |    |
	     |	       |    |
 *	     | 	       |    |
 *	     | 	       |    +---------------------- IDx INput divider, for the input clock.
             |         |                            00/1 01/02 ... 11/8
 *	     |         |
 *	     |         +---------------------------- TASSELx Timer_A clock source select
             |                                                | 00 | TACLK                            |
             |                                                | 01 | ACLK                             |
             |                                                | 10 | SMCLK                            |
             |                                                | 11 | INCLK (INCLK is device-specific) |
	     |
	     |
 *	     |
 *	     |
 *	     |
 *	     +--------------------------------------- unused
 *
 *
 */

//#define ESBMC 1

#include <msp430.h>
//#include <legacymsp430.h>
#include <stddef.h>
#include "fuel_rate_control.h"         /* Model's header file */
#include "rtwtypes.h"

#ifdef ESBMC
real32_T nondet_real32_T() {};
uint32_T nondet_uint32_T() {};
#endif

short interruptflag;

/*
 * Associating rt_OneStep with a real-time clock or interrupt service routine
 * is what makes the generated code "real-time".  The function rt_OneStep is
 * always associated with the base rate of the model.  Subrates are managed
 * by the base rate from inside the generated code.  Enabling/disabling
 * interrupts and floating point context switches are target specific.  This
 * example code indicates where these should take place relative to executing
 * the generated code step function.  Overrun behavior should be tailored to
 * your application needs.  This example simply sets an error status in the
 * real-time model and returns from rt_OneStep.
 */
void rt_OneStep(void)
{
  static boolean_T OverrunFlag = 0;

  /* Disable interrupts here */

  //Dominic 25/8: setup timer (codigo de freertos)
  /* Disable the timeslice interrupt */
  TACCTL0 = 0;

  /* Check for overrun */
  if (OverrunFlag) {
    rtmSetErrorStatus(fuel_rate_control_M, "overrun");
    return;
  }

  OverrunFlag = true;

  /* Save FPU context here (if necessary) */
  /* Re-enable timer or interrupt here */
  /* Set model inputs here */


  //Dominic 25/8: setup timer (codigo de freertos)
  /* Enable the interrupts. */
  TACCTL0 = CCIE; // up mode, counts to TACCR0


#ifdef ESBMC
  fuel_rate_control_U.sensors.speed = nondet_real32_T();
  fuel_rate_control_U.sensors.ego = nondet_real32_T();
  fuel_rate_control_U.sensors.map = nondet_real32_T();
  fuel_rate_control_U.sensors.throttle = nondet_real32_T();
#endif // ESBMC


  /* Step the model */
  fuel_rate_control_step();

  /* Get model outputs here */

  /* Indicate task complete */
  OverrunFlag = false;

  /* Disable interrupts here */
  //Dominic 25/8: setup timer (codigo de freertos)
  /* Disable the timeslice interrupt */
  TACCTL0 = 0;

  /* Restore FPU context here (if necessary) */
  /* Enable interrupts here */

  //Dominic 25/8: setup timer (codigo de freertos)
  /* Enable the interrupts. */
  TACCTL0 = CCIE;
}
/*
 * The example "main" function illustrates what is required by your
 * application code to initialize, execute, and terminate the generated code.
 * Attaching rt_OneStep to a real-time clock is target specific.  This example
 * illustates how you do this relative to initializing the model.
 */
int_T main()
{
  /* /\* Unused arguments *\/ */
  /* (void)(argc); */
  /* (void)(argv); */
	
  //Dominic 25/8: setup timer (codigo de freertos)
  /* Ensure the timer is stopped before configuring. */
  TACTL = 0; // clear all values
  //__delay_cycles(50);

  /* Initialize model */
  fuel_rate_control_initialize();

  /* Count up using ACLK clearing the initial counter. */
  TACTL = TASSEL_1 | MC_1 | TACLR;

  /* Set the compare match value according to the tick rate we want. */
  // aclk tem frequencia 32kHz
  TACCR0 = 320;//TAR + portACLK_FREQUENCY_HZ / configTICK_RATE_HZ;
  /* Enable the interrupts. */
  TACCTL0 = CCIE;	

  /* Attach rt_OneStep to a timer or interrupt service routine with
   * period 0.01 seconds (the model's base sample time) here.  The
   * call syntax for rt_OneStep is
   *
   rt_OneStep();   
  */
  /* printf("Warning: The simulation will run forever. " */
  /*        "Generated ERT main won't simulate model step behavior. " */
  /*        "To change this behavior select the 'MAT-file logging' option.\n"); */
  /* fflush((NULL)); */

  while (rtmGetErrorStatus(fuel_rate_control_M) == (NULL)) {
    /*  Perform other application tasks here */
	if (interruptflag) {
		interruptflag=0;
		rt_OneStep();  
	} else {
		__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
	}
  }

  /* Disable rt_OneStep() here */

  /* Terminate model */
  fuel_rate_control_terminate();
  return 0;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) TIMER0_A0_ISR (void)
{ 
	interruptflag=1;
	__bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
} 


