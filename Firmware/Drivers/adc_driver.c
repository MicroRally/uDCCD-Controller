/*
uDCCD controller
Analog driver

Author: Andis Jargans

Revision history:
2021-09-15: Initial version	
*/

/**** Hardware configuration ****
PC0 - BAT_MON  - battery voltage, 20mV/LSB
PC1 - DCCD_UMON - DCCD Coil voltage, 20mV/LSB
PC2 - DCCD_IMON - DCCD Coil current, 10mA/LSB
PC3 - POT_IN - Analog force set
PC4 - UPSW_IN - Optional analog input
PC5 - DNSW_IN - Optional analog input

ADC=Vin*1024/Vref

Fadc = Fcpu/DIV
One conversion = 13.5 Fadc cycles
*/

/**** Includes ****/
#include <avr/io.h>
#include "adc_driver.h"

/**** Private variables ****/
static volatile uint16_t bat_mon = 0;
static volatile uint16_t dccdu_mon = 0;
static volatile uint16_t dccdi_mon = 0;
static volatile uint16_t pot_mon = 0;
//static volatile uint16_t upsw_mon = 0;
//static volatile uint16_t dnsw_mon = 0;

/**** Public function definitions ****/
/**
 * @brief Initializes ADC hardware
 * @param [in] wake initialize ADC in waked state
 */
void ADCDRV_Init(uint8_t wake)
{
	/*
	uint8_t adcdiv = 0x07;
	
	if(presc<4) adcdiv = 0x01;  //set2
	else if(presc<8) adcdiv = 0x02; //set4
	else if(presc<16) adcdiv = 0x03; //set8
	else if(presc<32) adcdiv = 0x04; //set16
	else if(presc<64) adcdiv = 0x05; //set32
	else if(presc<128) adcdiv = 0x06; //set64
	else adcdiv = 0x07;  //set128
	*/
	
	PRR &= ~0x01; //Enable ADC power
	PORTCR |= 0x04; //Pull-up disable
	DIDR0 |= 0x0F; //Disable digital inputs
	ADMUX = 0x40; //Set AVCC reference
	ADCSRA = 0x07; //ADC Enabled, Single conversion, no IT, 125kHz @8MHz
	ADCSRB = 0x00; //no trigger input
	if(wake) ADCSRA |= 0x80;  //Enable ADC
	else PRR |= 0x01;
}

/**
 * @brief Wake up ADC
 */
void ADCDRV_Wake(void)
{
	//Enable ADC power
	PRR &= ~0x01;
	//Enable ADC
	ADCSRA |= 0x80;
}

/**
 * @brief Put ADC to sleep (low power mode)
 */
void ADCDRV_Sleep(void)
{
	//wait to finish
	while(ADCSRA&0x40);
	//Disable ADC
	ADCSRA &= ~0x80;
	//Disable ADC power
	PRR |= 0x01;
}

void ADCDRV_MeasureAll(void)
{
	//check if ADC is enabled
	if(!(ADCSRA&0x80)) return;
	
	ADMUX &= ~0x0F;
	ADMUX |= 0x00;
	ADCSRA |= 0x40;
	while(ADCSRA&0x40); //wait to finish
	bat_mon = ADC;
	
	ADMUX &= ~0x0F;
	ADMUX |= 0x01;
	ADCSRA |= 0x40;
	while(ADCSRA&0x40); //wait to finish
	dccdu_mon = ADC;
	
	ADMUX &= ~0x0F;
	ADMUX |= 0x02;
	ADCSRA |= 0x40;
	while(ADCSRA&0x40); //wait to finish
	dccdi_mon = ADC;
	
	ADMUX &= ~0x0F;
	ADMUX |= 0x03;
	ADCSRA |= 0x40;
	while(ADCSRA&0x40); //wait to finish
	pot_mon = ADC;
}

uint16_t ADCDRV_GetValue(uint8_t ch)
{
	//convert values to mV or mA and return
	switch(ch)
	{
		case 0:
		return bat_mon*20;
		
		case 1:
		return dccdu_mon*20;
		
		case 2:
		return dccdi_mon*10;//((dccdi_mon*59)/6);
		
		case 3:
		return ((pot_mon*63)/13);
		
		default:
		return 0;
	}
}
