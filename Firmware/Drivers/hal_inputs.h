/*
uDCCD controller
Inputs Hardware abstraction layer

Author: Andis Jargans

Revision history:
2021-10-21: Initial version
*/

#ifndef HAL_INPUTS
#define HAL_INPUTS

/**** Includes ****/

/**** Public definitions ****/
#define INHAL_AIN_BATMON	1
#define INHAL_AIN_UMON		2
#define INHAL_AIN_IMON		3
#define INHAL_AIN_POT		4
#define INHAL_AIN_UPSW		5
#define INHAL_AIN_DNSW		6
#define INHAL_AIN_TEMP		7
#define INHAL_AIN_INTREF	8
#define INHAL_AIN_GND		9

#define INHAL_DIN_DIMM		1
#define INHAL_DIN_BRAKE		2
#define INHAL_DIN_HBRAKE	3
#define INHAL_DIN_UPSW		4
#define INHAL_DIN_DNSW		5
#define INHAL_DIN_MODE		6
#define INHAL_DIN_BOOT0		7
#define INHAL_DIN_BOOT1		8
#define INHAL_DIN_BOOT2		9

#define INHAL_MODE_DIN		0
#define INHAL_MODE_AIN		1

#define INHAL_PULL_NONE		0
#define INHAL_PULL_UP		1
#define INHAL_PULL_DOWN		2

typedef struct inHalConfigStruct {
	uint8_t adc_wake;
	uint8_t upsw_mode;
	uint8_t dnsw_mode;
}inHalConfigDef;

/**** Public function declarations ****/
//Control functions
void INHAL_Init(inHalConfigDef* extCfg);
void INHAL_WakeADC(void);
void INHAL_SleepADC(void);
void INHAL_InitBootstraps(void);
void INHAL_DeInitBootstraps(void);

//Interrupt and loop functions

//Data retrieve functions
uint16_t INHAL_ADCRead(uint8_t ch);
uint8_t INHAL_GPIORead(uint8_t ch);

#endif