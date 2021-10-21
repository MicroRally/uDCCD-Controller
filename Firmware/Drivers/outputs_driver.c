/*
uDCCD controller
Outputs Hardware abstraction layer

Author: Andis Jargans

Revision history:
2021-09-17: Initial version
*/

/**** Hardware configuration ****
PB1 - DCCD_CTRL - OC1A, Active high
PB2 - LED_PWM - OC1B, Active high

PD0 - DSP0 - Active high GPO, OPEN
PD1 - DSP1 - Active high GPO, 20%
PD2 - DSP2 - Active high GPO, 40%
PD3 - DSP3 - Active high GPO, 60%
PD4 - DSP4 - Active high GPO, 80%
PD5 - DSP5 - Active high GPO, LOCK

Ftim = Fcpu/(2*TOP*DIV)
*/

/**** Includes ****/
#include <avr/io.h>
#include "outputs_driver.h"

/**** Private definitions ****/

/**** Private variables ****/
static volatile uint8_t init_done = 0;
static volatile uint8_t dccd_en = 0;

/**** Private function declarations ****/

/**** Public function definitions ****/

/**** Private function definitions ****/
/**
 * @brief Initializes outputs hardware
 * @param [in] presc Prescaler for timer [1/8/64/256/1024]
 * @param [in] top Overflow value for timer [1..65535]
 */
void OUTHAL_Init(void)
{
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
	ICR1 = OUTHAL_TIM_TOP; //TOP
	
	TCCR1B |= OUTHAL_CLK_PRESCALER;  //Enable timer
	
	dccd_en = 0;
	init_done = 1;
}

/**
 * @brief Set PWM duty cycle
 * @param [in] ch Timer output compare channel
 * @param [in] dc PWM Duty cycle in percent [0..100]
 */
void OUTHAL_SetPWM(uint8_t ch, uint8_t dc)
{
	//Map PWM Duty cycle to TIM TOP range
	uint16_t temp = 0;
	if(dc>100) temp = OUTHAL_TIM_TOP;
	else if(dc==0) temp = 0;
	else temp = (uint16_t)(((uint32_t)dc*OUTHAL_TIM_TOP)/100);
	
	switch(ch)
	{
		case OUTHAL_CH_DCCD:
			if(temp>OUTHAL_MAX_DCCD_OC) temp = OUTHAL_MAX_DCCD_OC;
			if(dccd_en) OCR1A = temp;
			else OCR1A = 0x0000;
			break;
			
		case OUTHAL_CH_LED:
			OCR1B = temp;
			break;
			
		default:
			break;
	}
}

/**
 * @brief Set PWM duty cycle
 * @param [in] ch Timer output compare channel
 * @param [in] dc PWM Duty cycle in permille [0..1000]
 */
void OUTHAL_SetPWMPrecise(uint8_t ch, uint16_t dc)
{
	//Map PWM Duty cycle to TIM TOP range
	uint16_t temp = 0;
	if(dc>1000) temp = OUTHAL_TIM_TOP;
	else if(dc==0) temp = 0;
	else temp = (uint16_t)(((uint32_t)dc*OUTHAL_TIM_TOP)/1000);
	
	switch(ch)
	{
		case OUTHAL_CH_DCCD:
			if(temp>OUTHAL_MAX_DCCD_OC) temp = OUTHAL_MAX_DCCD_OC;
			if(dccd_en) OCR1A = temp;
			else OCR1A = 0x0000;
			break;
			
		case OUTHAL_CH_LED:
			OCR1B = temp;
			break;
			
		default:
			break;
	}
}

/**
 * @brief Enable DCCDs output channel (OC1A)
 */
void OUTHAL_EnableDCCDch(void)
{
	dccd_en = 1;
}

/**
 * @brief Disable DCCDs output channel (OC1A)
 */
void OUTHAL_DisableDCCDch(void)
{
	OCR1A = 0x0000;
	dccd_en = 0;
}

/**
 * @brief Set LED image
 * @param [in] image bitwise output states to set
 */
void OUTHAL_SetLEDs(uint8_t image)
{
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