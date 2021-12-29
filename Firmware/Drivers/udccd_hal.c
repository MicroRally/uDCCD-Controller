/*
uDCCD controller
Hardware abstraction layer

Author: Andis Jargans

Revision history:
v0.0 - YYYY-MM-DD: Initial version
*/

/**** Hardware configuration ****
PA0 - 
PA1 - BOOT0 - Active low, needs integrated pull-up.
PA2 - BOOT1 - Active low, needs integrated pull-up.
PA3 - BOOT2 - Active low, needs integrated pull-up.

PB0 - DIMM_IN - Active low
PB1 - DCCD_CTRL - OC1A, Active high
PB2 - LED_PWM - OC1B, Active high
PB3 - MOSI
PB4 - MISO
PB5 - SCK
PB6 - HBRKAE_IN - Active low
PB7 - BRKAE_IN - Active low

PC0 - BAT_MON - Battery voltage, 20mV/LSB
PC1 - DCCD_UMON - DCCD Coil voltage, 20mV/LSB
PC2 - DCCD_IMON - DCCD Coil current, 10mA/LSB
PC3 - POT_IN - Analog force set
PC4 - UPSW_IN - Active low, Optional raw analog input
PC5 - DNSW_IN - Active low, Optional raw analog input
PC6 - RESET
PC7 - MODE_IN - Active low

PD0 - DSP0 - Active high GPO, OPEN
PD1 - DSP1 - Active high GPO, 20%
PD2 - DSP2 - Active high GPO, 40%
PD3 - DSP3 - Active high GPO, 60%
PD4 - DSP4 - Active high GPO, 80%
PD5 - DSP5 - Active high GPO, LOCK
PD6 - 
PD7 - 

Fadc = Fcpu/DIV
Ftim = Fcpu/(2*TOP*DIV)

ADC=Vin*1024/Vref

One conversion = 13.5 Fadc cycles
*/

/**** Includes ****/
#include <avr/io.h>
#include <avr/wdt.h>
#include "hw_config.h"

/**** Private variables ****/
static volatile uint8_t bootstraps = 0x00;
static volatile uint8_t upsw_mode = INHAL_MODE_DIN;
static volatile uint8_t dnsw_mode = INHAL_MODE_DIN;
static volatile uint8_t dccd_en = 0;

/**** Private function declarations ****/
static uint8_t InvertDIN(uint8_t val);
static void SetOutputCompare(uint8_t ch, uint16_t oc_val);

/**** Public function definitions ****/
void HAL_InitSystick(void)
{
	//Generic timing timer configuration
	TCCR0A = 0x00; //Normal mode
	TCNT0 = 0x00;
	OCR0A = 0x00;
	OCR0B = 0x00;
	TIMSK0 = 0x01; //OVF interrupt
		
	TCCR0A |= 0x03;  //Enable timer, with div64, T=2ms @8MHz
}

void HAL_InitWatchdog(void)
{
	//watchdog timer setup
	WDTCSR |= 0x10; //Change enable
	WDTCSR |= 0x0D; //System reset mode, 0.5s period.
	//use special instruction to reset watchdog timer
}

/**
 * @brief Initializes inputs hardware
 * @param [in] extCfg Struct with configuration parameters
 */
void HAL_InitInputs(halInpConfigDef* extCfg)
{
	static uint8_t init_done = 0;
	if(init_done) return;
	
	//Copy necessary configuration
	upsw_mode = extCfg->upsw_mode;
	dnsw_mode = extCfg->dnsw_mode;
	
	//Analog inputs configuration
	PRR &= ~0x01; //Enable ADC power
	PORTCR |= 0x04; //Pull-up disable
	DIDR0 |= 0x0F; //Disable digital inputs
	
	if(upsw_mode==HAL_INP_MODE_AIN) DIDR0 |= 0x10;
	if(dnsw_mode==HAL_INP_MODE_AIN) DIDR0 |= 0x20;
	
	ADMUX = 0x40;  //Set AVCC reference
	ADCSRA = 0x00; //ADC Disabled, Single conversion, no IT
	ADCSRA |= CFG_ADC_CLK_PRESCALER;
	ADCSRB = 0x00; //no trigger input
	
	if(extCfg->adc_wake) ADCSRA |= 0x80;  //Enable ADC
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
 * @brief Initializes hardware
 */
void HAL_InitOutputs(void)
{
	static uint8_t init_done = 0;
	
	if(init_done) return;
	
	//GPIO configuration
	PORTB &= ~0x06; //Set low
	DDRB |= 0x06;   //Set as outputs
	
	PORTD &= ~0x3F; //Set low
	DDRD |= 0x3F;   //Set as outputs
	
	//PWM Timer configuration
	TCCR1A = 0xA0; //Connect OC1A and OC1B
	TCCR1B = 0x10; //PWM, Phase & Frequency Correct ICR1 top, no clock
	TCCR1C = 0x00;
	TCNT1 = 0x0000;
	OCR1A = 0x0000;
	OCR1B = 0x0000;
	ICR1 = CFG_PWM_TIMER_TOP; //TOP
	
	TCCR1B |= CFG_PWM_TIMER_PRESCALER;  //Enable timer
	
	dccd_en = 0;
	init_done = 1;
}

/**
 * @brief Wake up ADC
 */
void HAL_WakeADC(void)
{
	//Enable ADC power
	PRR &= ~0x01;
	//Enable ADC
	ADCSRA |= 0x80;
}

/**
 * @brief Put ADC to sleep (low power mode)
 */
void HAL_SleepADC(void)
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
uint16_t HAL_ADCRead(uint8_t ch)
{
	//check if ADC is enabled
	if(!(ADCSRA&0x80)) return 0xFFFF;
	
	uint8_t mux = 0;	
	
	//Channel to MUX mapping
	switch(ch)
	{
		case HAL_AIN_BATMON:
			mux = 0x00;
			break;
			
		case HAL_AIN_UMON:
			mux = 0x01;
			break;
			
		case HAL_AIN_IMON:
			mux = 0x02;
			break;
			
		case HAL_AIN_POT:
			mux = 0x03;
			break;
			
		case HAL_AIN_UPSW:
			mux = 0x04;
			break;
			
		case HAL_AIN_DNSW:
			mux = 0x05;
			break;
			
		case HAL_AIN_TEMP:
			mux = 0x08;
			break;
			
		case HAL_AIN_INTREF:
			mux = 0x14;
			break;
			
		case HAL_AIN_GND:
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
uint8_t HAL_GPIORead(uint8_t ch)
{
	uint8_t raw = 0x00;
	
	//Channel mapping
	switch(ch)
	{
		case HAL_DIN_DIMM:
			raw = InvertDIN(PINB&0x01);
			break;
			
		case HAL_DIN_HBRAKE:
			raw = InvertDIN(PINB&0x40);
			break;
			
		case HAL_DIN_BRAKE:
			raw = InvertDIN(PINB&0x80);
			break;
			
		case HAL_DIN_UPSW:
			if(upsw_mode!=HAL_INP_MODE_AIN) raw = PINC&0x10;
			else raw = 0xFF;
			break;
			
		case HAL_DIN_DNSW:
			if(dnsw_mode!=HAL_INP_MODE_AIN) raw = PINC&0x20;
			else raw = 0xFF;
			break;
			
		case HAL_DIN_MODE:
			raw = PINC&0x80;
			break;
			
		case HAL_DIN_BOOT0:
			raw = PINA&0x02;
			break;
			
		case HAL_DIN_BOOT1:
			raw = PINA&0x04;
			break;
			
		case HAL_DIN_BOOT2:
			raw = PINA&0x08;
			break;
			
		default:
			raw = 0xFF;
	}
	
	if(raw==0xFF) return 0xFF;
	else if(raw) return 1;
	else return 0;
}

void HAL_ReducePower(void)
{
	//Disable unnecessary peripherals 
	PRR = 0x84;  //TWI and SPI
}

void HAL_ResetWatchdog(void)
{
	//Do WDT reset instruction
}

/**
 * @brief Set PWM duty cycle
 * @param [in] ch Timer output compare channel
 * @param [in] dc PWM Duty cycle in percent [0..100]
 */
void HAL_SetPWM(uint8_t ch, uint8_t dc)
{
	//Map PWM Duty cycle to TIM TOP range
	uint16_t temp = 0;
	if(dc>=100) temp = CFG_PWM_TIMER_TOP;
	else if(dc==0) temp = 0;
	else temp = (uint16_t)(((uint32_t)dc*CFG_PWM_TIMER_TOP)/100);
	
	SetOutputCompare(ch,temp);
}

/**
 * @brief Set PWM duty cycle, but with more resolution
 * @param [in] ch Timer output compare channel
 * @param [in] dc PWM Duty cycle as 16bit value
 */
void HAL_SetPWM16b(uint8_t ch, uint16_t dc)
{
	//Map PWM Duty cycle to TIM TOP range
	uint16_t temp = 0;
	if(dc==0xFFFF) temp = CFG_PWM_TIMER_TOP;
	else if(dc==0) temp = 0;
	else temp = (uint16_t)(((uint32_t)dc*CFG_PWM_TIMER_TOP)/0xFFFF);
	
	SetOutputCompare(ch,temp);
}

/**
 * @brief Set LED image
 * @param [in] image bitwise output states to set
 */
void HAL_SetLEDs(uint8_t image)
{
	//Capture current display image
	uint8_t temp = PORTD;
	temp &= 0x3F;
	
	//To reduce unnecessary flashing, change outputs only if there is change
	if(image!=temp)
	{
		//clear display
		PORTD &= ~0x3F;
		
		//set display
		PORTD |= (image&0x3F);
	};
}

/**
 * @brief Enable DCCDs output channel (OC1A)
 */
void HAL_EnableDCCDch(void)
{
	dccd_en = 1;
}

/**
 * @brief Disable DCCDs output channel (OC1A)
 */
void HAL_DisableDCCDch(void)
{
	OCR1A = 0x0000;
	dccd_en = 0;
}

/**
 * @brief Initializes bootstraps hardware
 */
void HAL_InitBootstraps(void)
{	
	//Digital inputs configuration
	//Bootstraps
	DDRA &= ~0x0E; //Set as inputs
	PORTA |= 0x0E; //Enable MCU pull-up
	
	bootstraps = 0x00;
}

/**
 * @brief Turn off bootstrap pull-up's, to save power
 */
void HAL_DeInitBootstraps(void)
{
	PORTA &= ~0x0E; //Disable pullup`s
}

/**
 * @brief Read and latch bootstraps
 */
uint8_t HAL_LatchBootstraps(void)
{
	//Init HW
	bootstraps = 0x00;
	INHAL_InitBootstraps();
	
	//Read value
	uint8_t i = 0;
	i = HAL_GPIORead(HAL_DIN_BOOT0);
	bootstraps |= i;
	i = HAL_GPIORead(HAL_DIN_BOOT1);
	bootstraps |= (i<<1);
	i = HAL_GPIORead(HAL_DIN_BOOT2);
	bootstraps |= (i<<2);
	
	HAL_DeInitBootstraps();
	
	return bootstraps;
}
/**
 * @brief Read chosen bootstrap
 * @param [in] ch Channel to read
 * @return Properly converted analog value as mV or mA
 */
uint8_t HAL_GetBootstrap(uint8_t ch)
{
	uint8_t i=0;
	
	switch(ch)
	{
		case INPUT_BOOT0:
			i = bootstraps&0x01;
			break;
			
		case INPUT_BOOT1:
			i = bootstraps&0x02;
			break;
			
		case INPUT_BOOT2:
			i = bootstraps&0x04;
			break;
			
		default:
			return bootstraps;
	}
	
	if(i) return 1;
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

/**
 * @brief Set output compare register, also makes sure DCCD duty cycle is cliped.
 * @param [in] ch channel to set
 * @param [in] oc_val register value to set
 */
void SetOutputCompare(uint8_t ch, uint16_t oc_val)
{
	switch(ch)
	{
		case HAL_PWM_CH_DCCD:
			if(oc_val>CFG_PWM_TIMER_DCCD_MAX) oc_val = CFG_PWM_TIMER_DCCD_MAX;
			if(dccd_en) OCR1A = oc_val;
			else OCR1A = 0x0000;
			break;
			
		case HAL_PWM_CH_LED:
			OCR1B = oc_val;
			break;
			
		default:
			break;
	}
}