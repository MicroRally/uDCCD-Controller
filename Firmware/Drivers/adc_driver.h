/*
uDCCD controller
Analog driver

Author: Andis Jargans

Revision history:
2021-09-15: Initial version
*/

#ifndef ADC_DRIVER
#define ADC_DRIVER

/**** Includes ****/

/**** Public function declarations ****/
//Control functions
void ADCDRV_Init(uint8_t wake);
void ADCDRV_Wake(void);
void ADCDRV_Sleep(void);

//Interrupt and loop functions
void ADCDRV_MeasureAll(void);

//Data retrieve functions
uint16_t ADCDRV_GetValue(uint8_t ch);

#endif