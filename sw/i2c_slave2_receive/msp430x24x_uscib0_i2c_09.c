/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 * 
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430x24x Demo - USCI_B0 I2C Slave RX multiple bytes from MSP430 Master
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  transmits to the slave. This is the slave code. The interrupt driven
//  data receiption is demonstrated using the USCIB0 RX interrupt.
//  ACLK = n/a, MCLK = SMCLK = default DCO = ~1.045Mhz
//
// ***to be used with "msp430x24x_uscib0_i2c_08.c" ***
//
//                                /|\  /|\
//               MSP430F24x       10k  10k     MSP430F24x
//                   slave         |    |        master
//             -----------------   |    |  -----------------
//           -|XIN  P3.1/UCB0SDA|<-|---+->|P3.1/UCB0SDA  XIN|-
//            |                 |  |      |                 |
//           -|XOUT             |  |      |             XOUT|-
//            |     P3.2/UCB0SCL|<-+----->|P3.2/UCB0SCL     |
//            |                 |         |                 |
//
//  B. Nisarga
//  Texas Instruments Inc.
//  September 2007
//  Built with CCE Version: 3.2.0 and IAR Embedded Workbench Version: 3.42A
//******************************************************************************
#include <msp430.h>
#define datasize 1
unsigned char *PRxData;                     // Pointer to RX data
unsigned char RXByteCtr;
volatile unsigned char RxBuffer[datasize];       // Allocate 128 byte of RAM, takes too long!!

int main(void) 
{
  //UCB0CTL1 &= ~UCTXSTT;	// no accidental start flag in case of multimaster  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
  UCB0I2COA = 52;                         // Own Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  UCB0I2CIE |= UCSTPIE + UCSTTIE;           // Enable STT and STP interrupt
  IE2 |= UCB0RXIE;                          // Enable RX interrupt

  while (1)
  {
    PRxData = (unsigned char *)RxBuffer;    // Start of RX buffer
    RXByteCtr = 0;                          // Clear RX byte count
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
                                            // Remain in LPM0 until master
                                            // finishes TX
    __no_operation();                       // Set breakpoint >>here<< and read
  }                                         // read out the RxData buffer
}

//------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move received data from the I2C master
// to the MSP430 memory.
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
  *PRxData++ = UCB0RXBUF;                   // Move RX data to address PRxData
  RXByteCtr++;                              // Increment RX byte count
}

//------------------------------------------------------------------------------
// The USCI_B0 state ISR is used to wake up the CPU from LPM0 in order to
// process the received data in the main program. LPM0 is only exit in case
// of a (re-)start or stop condition when actual data was received.
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
  if (RXByteCtr)                            // Check RX byte counter
    __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0 if data was
}                                           // received
