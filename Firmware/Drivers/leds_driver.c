/*
uDCCD controller
LEDs driver

Author: Andis Jargans

Revision history:
2021-09-16: Initial version
2021-10-21: Changed includes to fit new outputs HAL
*/

/**** Hardware configuration ****

*/

/**** Includes ****/
#include "leds_driver.h"
#include "hal_outputs.h"

/**** Private definitions ****/

/**** Private variables ****/
static volatile uint8_t set_image = 0;

/**** Private function declarations ****/

/**** Public function definitions ****/

/**** Private function definitions ****/
/**
 * @brief Initializes LED Driver
 */
void LEDRV_Init(void)
{
	OUTHAL_Init();
	OUTHAL_SetPWM(OUTHAL_CH_LED,0);
	OUTHAL_SetLEDs(0x00);
	
	set_image = 0x00;
}

/**
 * @brief Applies set image
 */
void LEDDRV_RefreshDisplay(void)
{
	OUTHAL_SetLEDs(set_image);
}

/**
 * @brief Set target image in raw, bitwise meaner
 */
void LEDDRV_SetDisplayBW(uint8_t image)
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
void LEDDRV_SetDisplayVal(uint8_t value, uint8_t style)
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
 * @brief Set display brightness
 * @param [in] lvl Percent value [0..100]
 */
void LEDDRV_SetDisplayBrightness(uint8_t lvl)
{
	OUTHAL_SetPWM(OUTHAL_CH_LED,lvl);
}