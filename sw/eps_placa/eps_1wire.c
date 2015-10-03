/************************************************************************/
/*  - name:
 *  - Description:
 *
 ***********************************************************************/

#include <msp430.h>
#include "intrinsics.h"



//configuration functions
void config_MSP430(void);
void config_DS2784(void);

//Measurement functions
void measurement_data_DS2784(void);
void measurement__adc(void);


//1 wire protocol functions
void outp(int bit);
int inp(void);
int OneWireReset(void);
void OneWireWrite(int bit);
int OneWireReadBit(void);
void OWWriteByte(int data);
int OWReadByte(void);

//define

#define P_1WireOUT P1OUT
#define P_1WireIN P1IN
#define DIR_P_1Wire P1DIR


volatile unsigned int RG_adc[6];             // ADC results register
volatile unsigned int RG_Temp_MSP430;             // ADC results register

volatile unsigned int RG_Current[2];
volatile unsigned int RG_Average_Current[2];
volatile unsigned int RG_Accumulated_Current[2];
volatile unsigned int RG_Voltage[2];
volatile unsigned int RG_Temp[2];


/*************************************************************************/
//                        main
/************************************************************************/
int main(void) {


	config_MSP430();
	config_DS2784();


	while(1){
	 //	measurement__adc();
        measurement_data_DS2784();
	}



  }

/************************************************************************/
/*  - name: config_MSP430
 *  - Description:
 *  - input data: --
 *  - output data: --
 ***********************************************************************/


void config_MSP430(void){

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	//ADC Msp430
	ADC12CTL0 = ADC12ON+MSC+SHT0_2 + REFON;  // Turn on ADC12, set sampling time, 16 ADC12CLK cycles, Vref=1.5V
	ADC12CTL1 = SHP + CONSEQ_1;
	ADC12MCTL0 = INCH_0+SREF_1;                   // ref+=VREF+, channel = A0
    ADC12MCTL1 = INCH_1+SREF_1;                   // ref+=VREF+, channel = A1
	ADC12MCTL2 = INCH_2+SREF_1;                   // ref+=VREF+, channel = A2
	ADC12MCTL3 = INCH_3+SREF_1;                   // ref+=VREF+, channel = A3
	ADC12MCTL4 = INCH_4+SREF_1;                   // ref+=VREF+, channel = A4
	ADC12MCTL5 = INCH_5+SREF_1;              // ref+=AVcc, channel = A6, end seq.
	ADC12MCTL6 = INCH_10+SREF_1+EOS;                // Vr+=Vref+
	ADC12CTL0 |= ENC;                       // Enable conversions

	 //1-wire Clock
 	BCSCTL1 = CALBC1_8MHZ;          		// 8Mhz cal data for DCO
    DCOCTL = CALDCO_8MHZ;           		// 8Mhz cal data for DCO
	//P1REN=0x01;		                       //pullup resistor enable   ????/ ojo mirar cual es el resistor pull-up
	//P1OUT |= 0x01;                        // resistor pullup activado no necesito poner a 1 el bus

}

/************************************************************************/
/*  - name: config_MSP430
 *  - Description:
 *  - input data: --
 *  - output data: --
 ***********************************************************************/

void config_DS2784(void){
	// CONFIGURATION DS2784

	volatile unsigned int reset=0x1;
	reset= OneWireReset();				// PROTECTION REGISTER
	OWWriteByte(0xCC);
	OWWriteByte(0x6C);
	OWWriteByte(0x00);
	OWWriteByte(0x03);

	reset= OneWireReset();              // PROTECTOR THRESHOLD REGISTER
	OWWriteByte(0xCC);
	OWWriteByte(0x6C);
	OWWriteByte(0x7F);
	OWWriteByte(0xF0);

}

/************************************************************************/
/*  - name: measurement__adc
 *  - Description:this fuccion read the sensors values by the ADC12.
 *  It uses the internal 1.5V reference and performs a single conversion
 *  on channels A0-A5. The conversion results are stored in results_adc register.
 *  - input data: --
 *  - ouput data: ADC results register
 ***********************************************************************/

void measurement__adc(void){                             // ADC  function

	ADC12CTL0 |= ADC12SC;                     // Start convn - software trigger

	RG_adc[0]= ADC12MEM0;
	RG_adc[1]= ADC12MEM1;
	RG_adc[2]= ADC12MEM2;
	RG_adc[3]= ADC12MEM3;
	RG_adc[4]= ADC12MEM4;
	RG_adc[5]= ADC12MEM5;
	RG_Temp_MSP430= ADC12MEM6;

}

/************************************************************************/
/*  - name: measurement_data_DS2784
 *  - Description:
 *  - input data: --
 *  - output data: --
 ***********************************************************************/
void measurement_data_DS2784(void){

    volatile unsigned int aux;
    volatile unsigned int reset=0x1;


   // TEMPERATURE MEASUREMENT

    reset= OneWireReset();              // TEMPERATURE MEASUREMENT - LSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x0B);
    aux=OWReadByte();
    RG_Temp[0]=aux>>5;

    reset= OneWireReset();              // TEMPERATURE MEASUREMENT - MSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x0A);
    aux=OWReadByte();
    RG_Temp[1]=aux>>5;
    aux=aux<<3;
    RG_Temp[0]|=aux & 0xF8;



    //AVERAGE CURRENT MEASUREMENT

    reset= OneWireReset();              // AVERAGE CURRENT MEASUREMENT - LSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x09);
    RG_Average_Current[0]=OWReadByte();

    reset= OneWireReset();              // AVERAGE CURRENT MEASUREMENT - MSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x08);
    RG_Average_Current[1]=OWReadByte();

    //ACCUMULATED CURRENT MEASUREMENT

    reset= OneWireReset();              // ACCUMULATED CURRENT MEASUREMENT - LSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x11);
    RG_Accumulated_Current[0]=OWReadByte();

    reset= OneWireReset();              // ACCUMULATED CURRENT MEASUREMENT - MSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x10);
    RG_Accumulated_Current[1]=OWReadByte();



   //CURRENT MEASUREMENT

    reset= OneWireReset();              //  CURRENT MEASUREMENT - LSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x0F);
    RG_Current[0]=OWReadByte();


    reset= OneWireReset();              // CURRENT MEASUREMENT - MSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x0E);
    RG_Current[1]=OWReadByte();

    //VOLTAGE MEASUREMENT

    reset= OneWireReset();           // VOLTAGE MEASUREMENT - LSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x0D);
    aux=OWReadByte();
    RG_Voltage[0]=aux>>5;

    reset= OneWireReset();              // VOLTAGE MEASUREMENT - MSB REGISTER
    OWWriteByte(0xCC);
    OWWriteByte(0x69);
    OWWriteByte(0x0C);
    aux=OWReadByte();
    RG_Voltage[1]=aux>>5;
    aux=aux<<3;
    RG_Voltage[0]|=aux & 0xF8;

	reset= OneWireReset();				// PROTECTION REGISTER
	OWWriteByte(0xCC);
	OWWriteByte(0x69);
	OWWriteByte(0x00);
	 aux=OWReadByte();
}



/************************************************************************/
/*  - name: outp
 *  - Description: This function sends bit to 1-wire slave.
 *  - input data: int bit
 *  - output data: --
 ***********************************************************************/

void outp(int bit){


	DIR_P_1Wire |= 0x01;			//sets P1.0 as output

	if(bit == 0){
		P_1WireOUT= 0x0;           //drives P1.0 low
	}
	else{
		P_1WireOUT=0x01;           //releases the bus
	}
}
/************************************************************************/
/*  - name: inp
 *  - Description: This function reads a bit from the 1-wire slave.
 *  - input data: --
 *  - output data: int result
 ***********************************************************************/

int inp(void){
	unsigned int result=0;

	 DIR_P_1Wire=0x00;          //sets P1.0 as input
	 result= P_1WireIN;         //prepares the bit on P1.0 to be returned

	 return result;		        //returns the bit on P1.0

	}

/************************************************************************/
/*  - name: OneWireReset
 *  - Description: This function genererates 1-wire reset. Returns 0
 *    if no presence detect was found, return 1 otherwise.
 *  - input data: --
 *  - output data: int result
 ***********************************************************************/

int OneWireReset(void){

	int result;

	 // outp(1);				        //drives P1.0 high
	__delay_cycles(0);			    // delay of 0
	  outp(0);				        //drives P1.0 low
	__delay_cycles(3812);			//delay of 480us, 8,12MHz* 480us=3897

	DIR_P_1Wire = 0x00;			//sets P1.0 as output
	// outp(1);						//releases the bus
	  result = inp();			        //prepares the result of present detection to be returned
	  __delay_cycles(568);			//delay of 70us, 8,12MHz*70us=568

	  //outp(1);
	__delay_cycles(3329);			//delay of 410us, 8,12MHz*410us=3329

	return result;			        //returns the presence detection result

}

/************************************************************************/
/*  - name: OneWireWrite
 *  - Description: this function sends a 1-wire write bit to 1-wire slave.
 *  - input data: int bit
 *  - output data:--
 ***********************************************************************/

void OneWireWrite(int bit){



		if(bit == 1){
			outp(0);				//drives P1.0 low
		__delay_cycles(40);		    //delay of 6us, 8,12 MHz*6us= 44
		DIR_P_1Wire = 0x00;
		//	outp(1);				//releases the bus
		__delay_cycles(500);		//delay of 64us, 8,12MHz*64us=517

		}else{
	       // Write '0' bit
			outp(0);				//drives P1.0 low
		__delay_cycles(470);		//delay of 60us, 8,12MHz*60us=487
		DIR_P_1Wire = 0x00;
			//outp(1);				//releases the bus
		__delay_cycles(60);			// delay of 10us, 8MHz*10us=80

		}

}

/************************************************************************/
/*  - name: OneWireReadBit
 *  - Description: This function reads a 1-wire data bit  to 1-wire slave and returns int.
 *  - input data: --
 *  - output data: int result
 ***********************************************************************/

int OneWireReadBit(void){

	int result;
	outp(0);						//drives P1.0 low
	__delay_cycles(44);		        //delay of 6us, 8,12 MHz*6us= 44
	DIR_P_1Wire = 0x00;
	//outp(1);						//releases the bus
	__delay_cycles(70);				//delay of 9us, 8,12MHz*9us=73

	result = inp();					//sample the bit from slave
	__delay_cycles(443);			//delay of 55us, 8,12MHz*55us=446

	return result;

}

/************************************************************************/
/*  - name: OWWriteByte
 *  - Description: This function writes a 1-wire data byte.
 *  - input data: int data
 *  - output data: --
 ***********************************************************************/

void OWWriteByte(int data)
{

	int loop;

	for (loop = 0; loop < 8; loop++)	// Loop to write each bit in the byte, LS-bit first
	{
		OneWireWrite(data & 0x01);

		data >>= 1;						// shift the data byte for the next bit
	}
}

/************************************************************************/
/*  - name: OWReadByte
 *  - Description: This function read a 1-wire data byte and return it
 *  - input data: int data
 *  - output data: --
 ***********************************************************************/

int OWReadByte(void)
{
	int loop, result = 0;
	for (loop = 0; loop < 8; loop++)
	{

		result >>= 1;						// shift the result to get it ready for the next bit

		if (OneWireReadBit())			    // if result is one, then set MS bit
			result |= 0x80;
	}
		return result;
}








