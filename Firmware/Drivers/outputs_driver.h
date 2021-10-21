/*
uDCCD controller
Outputs Hardware abstraction layer

Author: Andis Jargans

Revision history:
2021-09-17: Initial version
*/

#ifndef HAL_OUTPUTS
#define HAL_OUTPUTS

/**** Includes ****/

/**** Public definitions ****/
#define OUTHAL_CH_DCCD	1
#define OUTHAL_CH_LED	2

/**** Aplciation specific configuration ****/
#define OUTHAL_CLK_PRESCALER	1
#define OUTHAL_TIM_TOP			256	
#define OUTHAL_MAX_DCCD_OC		250

/**** Public function declarations ****/
//Control functions
void OUTHAL_Init(void);
void OUTHAL_SetPWM(uint8_t ch, uint8_t dc);
void OUTHAL_SetPWMPrecise(uint8_t ch, uint16_t dc);
void OUTHAL_EnableDCCDch(void);
void OUTHAL_DisableDCCDch(void);
void OUTHAL_SetLEDs(uint8_t image);

//Interrupt and loop functions

//Data retrieve functions

#endif