/*
uDCCD controller
Coil constant current mode driver

Author: Andis Jargans

Revision history:
v0.0 - YYYY-MM-DD
*/

#ifndef COIL_DRIVER
#define COIL_DRIVER

/**** Includes ****/
#include <avr/io.h>

/**** Public definitions ****/

/**** Public function declarations ****/
//Control functions
void COILDRV_Init(void);
void COILDRV_SetTarget(uint16_t current);
void COILDRV_UpdateMeasurments(uint16_t current, uint16_t voltage, uint16_t supply);

//Interrupt and loop functions
void COILDRV_Process(void);

//Data retrieve functions

#endif