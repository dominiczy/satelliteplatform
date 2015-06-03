/**
 * @file   i2cdriver.h
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  This file implements a simple i2c driver
 * 
 */
#include <msp430.h>

unsigned char RXData; // Received data	
unsigned char TXData; // Data to be transmitted
unsigned char address; // Slave address
unsigned char booltransmit; // Transmitting or receiving?

void prvSetupI2C( void ); // Setup I2C modules
void transmit(unsigned char data,unsigned char address); // Transmit data
unsigned char receive(unsigned char address); // Receive data
