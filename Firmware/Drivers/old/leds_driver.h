/*
uDCCD controller
LEDs driver

Author: Andis Jargans

Revision history:
2021-09-16: Initial version
*/

#ifndef LED_DRIVER
#define LED_DRIVER

/**** Includes ****/

/**** Public definitions ****/
#define LED_DSP_BAR		0
#define LED_DSP_DOT10	1
#define LED_DSP_DOT20	2

/**** Public function declarations ****/
//Control functions
void LEDRV_Init(void);
void LEDDRV_SetDisplayBW(uint8_t image);
void LEDDRV_SetDisplayVal(uint8_t value, uint8_t style);
void LEDDRV_SetDisplayBrightness(uint8_t lvl);

//Interrupt and loop functions
void LEDDRV_RefreshDisplay(void);

//Data retrieve functions

#endif