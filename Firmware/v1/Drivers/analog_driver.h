/*
uDCCD controller
Analog driver

Author: Andis Jargans

Revision history:
2020-09-24: Initial version
*/

#ifndef ANALOG_DRIVER
#define ANALOG_DRIVER

/**** Includes ****/
#include <avr/io.h>

/**** Public definitions ****/
#define AN_BATU		0
#define AN_DCCDU	1
#define AN_DCCDI	2
#define AN_POTU		3

/**** Public function declarations ****/
void ANDRV_Init(void);
void ANDRV_MeasureAll(void);
uint16_t ANDRV_GetValue(uint8_t ch);

#endif