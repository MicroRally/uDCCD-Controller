/*
uDCCD controller
LEDs driver

Author: Andis Jargans

Revision history:
2021-09-17: Initial version
2021-10-21: Configuration defines moved to hw_config.h
*/

#ifndef DCCD_DRIVER
#define DCCD_DRIVER

/**** Includes ****/

/**** Public definitions ****/
#define FAULT_SUPPLY	1
#define FAULT_OUTPUT	2
#define FAULT_LOAD_LOSS	3

/**** Public function declarations ****/
//Control functions
void COILDRV_Init(void);
void COILDRV_SetFroce(uint8_t force);

//Interrupt and loop functions
void COILDRV_ProcessLogic(uint16_t u_supply);
void COILDRV_ProcessProtection(uint16_t i_dccd, uint16_t u_dccd, uint16_t u_supply);

//Data retrieve functions
uint8_t COILDRV_GetFaultFlag(uint8_t type);

#endif