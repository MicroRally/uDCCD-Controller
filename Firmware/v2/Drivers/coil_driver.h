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
#include "udccd_hal.h"
#include "hw_config.h"

/**** Public definitions ****/
#define COILDRV_WARNING_LOAD_LOSS	1
#define COILDRV_WARNING_OVERCURRENT	2
#define COILDRV_WARNING_OUT_HIGH	3
#define COILDRV_WARNING_OUT_LOW		4
#define COILDRV_WARNING_SUPPLY_HIGH	5
#define COILDRV_WARNING_SUPPLY_LOW	6

/**** Public function declarations ****/
//Control functions
void COILDRV_Init(void);
void COILDRV_SetTarget(uint16_t current);
void COILDRV_UpdateMeasurments(uint16_t current, uint16_t voltage, uint16_t supply);

//Interrupt and loop functions
void COILDRV_Process(void);

//Data retrieve functions
uint8_t COILDRV_GetFault(void);
uint8_t COILDRV_GetWarning(uint8_t warning_ch);

#endif