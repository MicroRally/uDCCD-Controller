/*
uDCCD controller
Outputs driver

Author: Andis Jargans

Revision history:
2020-09-24: Initial version	
*/

#ifndef OUTPUTS_DRIVER
#define OUTPUTS_DRIVER

/**** Includes ****/
#include <avr/io.h>

/**** Public definitions ****/
#define DSP_DOT		0
#define DSP_EXTDOT	1
#define DSP_BAR		2
#define DSP_UNLOCK	0
#define DSP_LOCK	1

#define OUTCFG_DEFRES	1500

#define PWM_LIMITED	0
#define PWM_FULL	1

/**** Public function declarations ****/
void OUTDRV_Init(void);

/***** DISPLAY CONTROL FUNCTIONS *****/
void OUTDRV_RefreshDisplay(void);
void OUTDRV_SetDisplayBW(uint8_t image);
void OUTDRV_SetDisplayVal(uint8_t value, uint8_t style);
void OUTDRV_SetDisplayPWM(uint8_t pwm);

/***** DCCD CONTROL FUNCTIONS *****/
void OUTDRV_SetDccdRaw(uint16_t ocval);
void OUTDRV_SetDccdPWM(uint8_t pwm, uint8_t mode);
void OUTDRV_ProcessCoil(uint16_t ubat);
void OUTDRV_FuseDccd(void);
void OUTDRV_UnfuseDccd(void);
uint8_t OUTDRV_GetFuseSate(void);
void OUTDRV_SetCoilConfig(uint16_t res, uint16_t nom_bat, uint16_t lock_current);
uint16_t OUTDRV_GetResistance(void);
uint8_t OUTDRV_GetDCValue(void);

#endif
