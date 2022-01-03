/*
uDCCD controller
Inputs driver

Author: Andis Jargans

Revision history:
2020-09-24: Initial version	
*/

#ifndef INPUTS_DRIVER
#define INPUTS_DRIVER

/**** Includes ****/
#include <avr/io.h>

/**** Public definitions ****/
#define IN_DIMM		0
#define IN_HBRAKE	1
#define IN_BRAKE	2
#define IN_UPSW		3
#define IN_DOWN		4
#define IN_MODE		5

/**** Public function declarations ****/
void INDRV_Init(void);
uint8_t INDRV_GetBootstrap(uint8_t ch);
void INDRV_ReadAllTimed(void);
uint8_t INDRV_GetInput(uint8_t ch);
uint8_t INDRV_GetInputChange(uint8_t ch);
void INDRV_ResetInputChange(uint8_t ch);

#endif