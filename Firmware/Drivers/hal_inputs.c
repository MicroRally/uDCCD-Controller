/*
uDCCD controller
Inputs Hardware abstraction layer

Author: Andis Jargans

Revision history:
2021-10-21: Initial version	
*/

/**** Hardware configuration ****
PC0 - BAT_MON  - battery voltage, 20mV/LSB
PC1 - DCCD_UMON - DCCD Coil voltage, 20mV/LSB
PC2 - DCCD_IMON - DCCD Coil current, 10mA/LSB
PC3 - POT_IN - Analog force set, raw

PC4 - UPSW_IN - Active low, Optional raw analog input
PC5 - DNSW_IN - Active low, Optional raw analog input

PB0 - DIMM_IN - Active low
PB6 - HBRAKE_IN - Active low
PB7 - BRAKE_IN - Active low

PA1 - BOOT0 - Active low, needs integrated pull-up.
PA2 - BOOT0 - Active low, needs integrated pull-up.
PA3 - BOOT0 - Active low, needs integrated pull-up.

ADC=Vin*1024/Vref

Fadc = Fcpu/DIV
One conversion = 13.5 Fadc cycles
*/

/**** Includes ****/
#include <avr/io.h>
#include "hal_inputs.h"
#include "hw_config.h"

/**** Private variables ****/
static volatile uint8_t upsw_mode = INHAL_MODE_DIN;
static volatile uint8_t dnsw_mode = INHAL_MODE_DIN;

/**** Private function declarations ****/
static uint8_t InvertDIN(uint8_t val);

/**** Public function definitions ****/
/**
 * @brief Initializes inputs hardware
 * @param [in] extCfg Struct with configuration parameters
 */
void INHAL_Init(inHalConfigDef* extCfg)
{
	static uint8_t init_done = 0;
	if(init_done) return;
	
	//Copy neccesery configuration
	upsw_mode = *extCfg.upsw_mode;
	dnsw_mode = *extCfg.dnsw_mode;
	
	//Analog inputs configuration
	PRR &= ~0x01; //Enable ADC power
	PORTCR |= 0x04; //Pull-up disable
	DIDR0 |= 0x0F; //Disable digital inputs
	
	if(upsw_mode==INHAL_MODE_AIN) DIDR0 |= 0x10;
	if(dnsw_mode==INHAL_MODE_AIN) DIDR0 |= 0x20;
	
	ADMUX = 0x40;  //Set AVCC reference
	ADCSRA = 0x00; //ADC Disabled, Single conversion, no IT
	ADCSRA |= ADC_CLK_PRESCALER;
	ADCSRB = 0x00; //no trigger input
	
	if(*extCfg.adc_wake) ADCSRA |= 0x80;  //Enable ADC
	else PRR |= 0x01;
	
	//Digital inputs configuration
	
	//External inputs (dimm, brake, h-brake)
	DDRB &= ~0xC1; //Set as inputs
	PORTB &= ~0xC1; //Disable MCU pull-up
	
	//User interface inputs
	DDRC &= ~0xB0; //Set as input
	//Disable pullup's, althogh, ADC has allready disabled them
	PORTC &= ~0xB0; 
	
	init_done = 1;
}

/**
 * @brief Initializes bootstraps hardware
 */
void INHAL_InitBootstraps(void)
{
	static uint8_t init_done = 0;
	if(init_done) return;
	
	//Digital inputs configuration
	//Bootstraps
	DDRA &= ~0x0E; //Set as inputs
	PORTA |= 0x0E; //Enable MCU pull-up
	
	init_done = 1;
}

/**
 * @brief Turn off bootstrap pull-up's, to save power
 */
void INHAL_DeInitBootstraps(void)
{
	PORTA &= ~0x0E;
}

/**
 * @brief Wake up ADC
 */
void INHAL_WakeADC(void)
{
	//Enable ADC power
	PRR &= ~0x01;
	//Enable ADC
	ADCSRA |= 0x80;
}

/**
 * @brief Put ADC to sleep (low power mode)
 */
void INHAL_SleepADC(void)
{
	//wait to finish
	while(ADCSRA&0x40);
	//Disable ADC
	ADCSRA &= ~0x80;
	//Disable ADC power
	PRR |= 0x01;
}

/**
 * @brief Perform single ADC conversion
 * @param [in] ch Analog channel to read
 * @returns	Raw 10bit ADC value, 0xFFFF in case of failure
 */
uint16_t INHAL_ADCRead(uint8_t ch)
{
	//check if ADC is enabled
	if(!(ADCSRA&0x80)) return 0xFFFF;
	
	uint8_t mux = 0;	
	
	//Channel to MUX mapping
	switch(ch)
	{
		case INHAL_AIN_BATMON:
			mux = 0x00;
			break;
			
		case INHAL_AIN_UMON:
			mux = 0x01;
			break;
			
		case INHAL_AIN_IMON:
			mux = 0x02;
			break;
			
		case INHAL_AIN_POT:
			mux = 0x03;
			break;
			
		case INHAL_AIN_UPSW:
			mux = 0x04;
			break;
			
		case INHAL_AIN_DNSW:
			mux = 0x05;
			break;
			
		case INHAL_AIN_TEMP:
			mux = 0x08;
			break;
			
		case INHAL_AIN_INTREF:
			mux = 0x14;
			break;
			
		case INHAL_AIN_GND:
			mux = 0x15;
			break;
			
		default:
			return 0xFFFF;
	}
	
	ADMUX &= ~0x0F;
	ADMUX |= mux;
	ADCSRA |= 0x40;
	while(ADCSRA&0x40); //wait to finish
	return ADC;
}

/**
 * @brief Read digital input
 * @param [in] ch Digital channel to read
 * @returns	User side input state, 0-low, 1-high, 0xFF - fault.
 */
uint8_t INHAL_GPIORead(uint8_t ch)
{
	uint8_t raw = 0x00;
	
	//Channel mapping
	switch(ch)
	{
		case INHAL_DIN_DIMM:
			raw = InvertDIN(PINB&0x01);
			break;
			
		case INHAL_DIN_HBRAKE:
			raw = InvertDIN(PINB&0x40);
			break;
			
		case INHAL_DIN_BRAKE:
			raw = InvertDIN(PINB&0x80);
			break;
			
		case INHAL_DIN_UPSW:
			if(upsw_mode!=INHAL_MODE_AIN) raw = PINC&0x10;
			else raw = 0xFF;
			break;
			
		case INHAL_DIN_DNSW:
			if(dnsw_mode!=INHAL_MODE_AIN) raw = PINC&0x20;
			else raw = 0xFF;
			break;
			
		case INHAL_DIN_MODE:
			raw = PINC&0x80;
			break;
			
		case INHAL_DIN_BOOT0:
			raw = PINA&0x02;
			break;
			
		case INHAL_DIN_BOOT1:
			raw = PINA&0x04;
			break;
			
		case INHAL_DIN_BOOT2:
			raw = PINA&0x08;
			break;
			
		default:
			raw = 0xFF;
	}
	
	if(raw==0xFF) return 0xFF;
	else if(raw) return 1;
	else return 0;
}

/**** Private function definitions ****/
/**
 * @brief Invert DIN raw value
 */
uint8_t InvertDIN(uint8_t val)
{
	if(val) return 0;
	else return 1;
}