/*
uDCCD controller
LED Display driver

Author: Andis Jargans

Revision history:
v0.0 - YYYY-MM-DD: Initial version
*/

/***** Includes *****/
#include "display_driver.h"

/***** Private definitions *****/

/***** Private variables *****/
static uint8_t set_image = 0;

/***** Private function declarations *****/

/***** Public function definitions *****/
/**
 * @brief Initializes display driver
 */
void DSPDRV_Init(void)
{
	HAL_InitOutputs();
	HAL_SetPWM(HAL_PWM_CH_LED,0);
	HAL_SetLEDs(0x00);
	
	set_image = 0x00;
}

/**
 * @brief Set target image in raw, bitwise meaner
 */
void DSPDRV_SetDisplayBW(uint8_t image)
{
	//remove extra bits
	image &= 0x3F;
	
	set_image=image;
}

/**
 * @brief Converts percent value to image, and sets it
 * @param [in] value Percent value [0..100]
 * @param [in] style Display style, dot or bar
 */
void DSPDRV_SetDisplayVal(uint8_t value, uint8_t style)
{
	uint8_t image = 0x00;
	
	switch(style)
	{
		case LED_DSP_DOT20:
			//dot
			if(value<20) image = 0x01;
			else if(value<40) image = 0x02;
			else if(value<60) image = 0x04;
			else if(value<80) image = 0x08;
			else if(value<100) image = 0x10;
			else image = 0x20;
			break;
			
		case LED_DSP_DOT10:
			//extended dot
			if(value<10) image = 0x01;       //0
			else if(value<20) image = 0x03;  //10
			else if(value<30) image = 0x02;  //20
			else if(value<40) image = 0x06;  //30
			else if(value<50) image = 0x04;  //40
			else if(value<60) image = 0x0C;  //50
			else if(value<70) image = 0x08;  //60
			else if(value<80) image = 0x18;  //70
			else if(value<90) image = 0x10;  //80
			else if(value<100) image = 0x30; //90
			else image = 0x20;               //100
			break;

		default:
			//bar
			if(value<20) image = 0x01;
			else if(value<40) image = 0x03;
			else if(value<60) image = 0x07;
			else if(value<80) image = 0x0F;
			else if(value<100) image = 0x1F;
			else image = 0x3F;
			break;
	}
	
	set_image = image;
}

/**
 * @brief Applies set image
 */
void DSPDRV_RefreshDisplay(void)
{
	HAL_SetLEDs(set_image);
}

/**
 * @brief Set display brightness
 * @param [in] lvl Percent value [0..100]
 */
void DSPDRV_SetDisplayBrightness(uint8_t lvl)
{
	HAL_SetPWM(HAL_PWM_CH_LED,lvl);
}


/***** Private function definitions *****/
