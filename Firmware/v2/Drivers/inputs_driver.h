/*
uDCCD controller
Analog and Digital inputs driver

Author: Andis Jargans

Revision history:
v0.0 - YYYY-MM-DD
*/

#ifndef INPUTS_DRIVER
#define INPUTS_DRIVER

/**** Includes ****/
#include <avr/io.h>
#include "udccd_hal.h"
#include "hw_config.h"

/**** Public definitions ****/
#define INPUT_ACTIVE_LOW	0
#define INPUT_ACTIVE_HIGH	1

#define INPUT_MODE_DIGITAL	0
#define INPUT_MODE_ANALOG	1

#define	ANALOG_CH_BATTERY	1
#define	ANALOG_CH_DCCD_U	2
#define	ANALOG_CH_DCCD_I	3
#define	ANALOG_CH_POT		4
#define	ANALOG_CH_UPSW		5
#define	ANALOG_CH_DNSW		6

#define	INPUT_CH_ALL		0
#define	INPUT_CH_DIMM		1
#define	INPUT_CH_BRAKE		2
#define	INPUT_CH_HBRAKE		3
#define	INPUT_CH_UPSW		4
#define	INPUT_CH_DNSW		5
#define	INPUT_CH_MODE		6

typedef struct inInitStruct {
	uint8_t act_lvl_dimm;
	uint8_t act_lvl_brake;
	uint8_t act_lvl_hbrake;
	uint8_t act_lvl_upsw;
	uint8_t act_lvl_dnsw;
	uint8_t act_lvl_mode;
	uint8_t mode_upsw;
	uint8_t mode_dnsw;
}inInitDef;

/**** Public function declarations ****/
//Control functions
void INDRV_Init(inInitDef* initCfg);
uint8_t INDRV_ReadBootstraps(void);

//Interrupt and loop functions
void INDRV_ReadAll(void);

//Data retrieve functions
uint16_t INDRV_GetAnalog(uint8_t ch);
uint8_t INDRV_GetInputState(uint8_t ch);
uint8_t INDRV_GetInputChange(uint8_t ch);
void INDRV_ResetChangeFlag(uint8_t ch);
uint8_t INDRV_GetBootstrap(uint8_t ch);

#endif