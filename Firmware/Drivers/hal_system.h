/*
uDCCD controller
Inputs Hardware abstraction layer

Author: Andis Jargans

Revision history:
2021-MM-DD: Initial version
*/

#ifndef HAL_SYSTEM
#define HAL_SYSTEM
/**** Includes ****/

/**** Public definitions ****/
#define	INPUT_BOOTALL		0
#define	INPUT_BOOT0			1
#define	INPUT_BOOT1			2
#define	INPUT_BOOT2			3

/**** Public function declarations ****/
//Control functions
void SYSHAL_Init_systick(void);
void SYSHAL_Init_watchdog(void);
void SYSHAL_ReducePower(void);
void SYSHAL_ResetWatchdog(void);
void SYSHAL_InitBootstraps(void);
void SYSHAL_DeInitBootstraps(void);
uint8_t SYSHAL_LatchBootstraps(void);
uint8_t SYSHAL_GetBootstrap(uint8_t ch);

//Interrupt and loop functions

//Data retrieve functions

#endif