/*
uDCCD controller
Outputs Hardware abstraction layer

Author: Andis Jargans

Revision history:
2021-10-21: Initial version
*/

#ifndef HAL_OUTPUTS
#define HAL_OUTPUTS

/**** Includes ****/

/**** Public definitions ****/
#define OUTHAL_CH_DCCD	1
#define OUTHAL_CH_LED	2

/**** Public function declarations ****/
//Control functions
void OUTHAL_Init(void);
void OUTHAL_SetPWMPercent(uint8_t ch, uint8_t dc);
void OUTHAL_SetPWM10k(uint8_t ch, uint16_t dc);
void OUTHAL_SetLEDs(uint8_t image);
void OUTHAL_EnableDCCDch(void);
void OUTHAL_DisableDCCDch(void);

//Interrupt and loop functions

//Data retrieve functions

#endif