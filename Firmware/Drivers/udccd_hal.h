/*
uDCCD controller
Hardware abstraction layer

Author: Andis Jargans

Revision history:
v0.0 - YYYY-MM-DD: Initial version
*/

#ifndef HAL_UDCCD
#define HAL_UDCCD
/**** Includes ****/

/**** Public definitions ****/
#define	HAL_BOOTALL			0
#define	HAL_BOOT0			1
#define	HAL_BOOT1			2
#define	HAL_BOOT2			3

#define HAL_AIN_BATMON		1
#define HAL_AIN_UMON		2
#define HAL_AIN_IMON		3
#define HAL_AIN_POT			4
#define HAL_AIN_UPSW		5
#define HAL_AIN_DNSW		6
#define HAL_AIN_TEMP		7
#define HAL_AIN_INTREF		8
#define HAL_AIN_GND			9

#define HAL_DIN_DIMM		1
#define HAL_DIN_BRAKE		2
#define HAL_DIN_HBRAKE		3
#define HAL_DIN_UPSW		4
#define HAL_DIN_DNSW		5
#define HAL_DIN_MODE		6
#define HAL_DIN_BOOT0		7
#define HAL_DIN_BOOT1		8
#define HAL_DIN_BOOT2		9

#define HAL_INP_MODE_DIN	0
#define HAL_INP_MODE_AIN	1

#define HAL_INP_PULL_NONE	0
#define HAL_INP_PULL_UP		1
#define HAL_INP_PULL_DOWN	2

#define HAL_PWM_CH_DCCD		1
#define HAL_PWM_CH_LED		2

typedef struct halInpConfigStruct {
	uint8_t adc_wake;
	uint8_t upsw_mode;
	uint8_t dnsw_mode;
}halInpConfigDef;

/**** Public function declarations ****/
//Init functions
void HAL_InitInputs(halInpConfigDef* extCfg);
void HAL_InitOutputs(void);
void HAL_InitSystick(void);
void HAL_InitWatchdog(void);
void HAL_InitBootstraps(void);
void HAL_DeInitBootstraps(void);

//Control functions
void HAL_WakeADC(void);
void HAL_SleepADC(void);

void HAL_SetPWM(uint8_t ch, uint8_t dc);
void HAL_SetPWM16b(uint8_t ch, uint16_t dc);
void HAL_SetLEDs(uint8_t image);
void HAL_EnableDCCDch(void);
void HAL_DisableDCCDch(void);

void HAL_ReducePower(void);
void HAL_ResetWatchdog(void);

//Interrupt and loop functions

//Data retrieve functions
uint16_t HAL_ADCRead(uint8_t ch);
uint8_t HAL_GPIORead(uint8_t ch);

uint8_t HAL_LatchBootstraps(void);
uint8_t HAL_GetBootstrap(uint8_t ch);

#endif