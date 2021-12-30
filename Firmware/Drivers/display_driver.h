/*
uDCCD controller
LED Display driver

Author: Andis Jargans

Revision history:
v0.0 - YYYY-MM-DD
*/

#ifndef DISPLAY_DRIVER
#define DISPLAY_DRIVER

/**** Includes ****/
#include <avr/io.h>
#include "udccd_hal.h"
#include "hw_config.h"

/**** Public definitions ****/
#define LED_DSP_DOT10	1
#define LED_DSP_DOT20	2

/**** Public function declarations ****/
//Control functions
void DSPDRV_Init(void);
void DSPDRV_SetDisplayBW(uint8_t image);
void DSPDRV_SetDisplayVal(uint8_t value, uint8_t style);
void DSPDRV_SetDisplayBrightness(uint8_t lvl);

//Interrupt and loop functions
void DSPDRV_RefreshDisplay(void);

//Data retrieve functions

#endif