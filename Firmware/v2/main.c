/*
uDCCD controller
Main application

Author: Andis Jargans

Revision history:
v2.0 - YYYY-MM-DD: Initial version
*/

/**** Hardware configuration ****
*/

#include <avr/io.h>
#include <avr/eeprom.h>

#include "Drivers/inputs_driver.h"
#include "Drivers/coil_driver.h"
#include "Drivers/display_driver.h"
#include "Drivers/hw_config.h"

/**** Private definitions ****/

/**** Private variables ****/
uint16_t supply_u = 0;
uint16_t dccd_u = 0;
uint16_t dccd_i = 0;
uint16_t pot = 0;

uint8_t hbrake_act = 0;
uint8_t brakes_act = 0;
uint8_t dimm_act = 0;

uint8_t upsw_act = 0;
uint8_t dnsw_act = 0;
uint8_t mode_act = 0;

uint8_t brakes_mode = 0;
uint8_t user_force = 0;

uint8_t set_force = 0;
uint16_t set_current = 0;

/**** Private function declarations ****/
void GatherData(uint8_t times);
uint8_t ProcessUpDownButtons(uint8_t force);
uint8_t ProcessMode(uint8_t mode);
uint8_t GetNextTargetForce(void);
uint16_t ForceToCurrent(uint8_t force);
void ProcessDisplay(void);

/**** Application ****/
int main(void)
{
	//System setup
	inInitDef inpCfg;
	
	inpCfg.act_lvl_hbrake = INPUT_ACTIVE_LOW;
	inpCfg.act_lvl_brake = INPUT_ACTIVE_HIGH;
	inpCfg.act_lvl_dimm = INPUT_ACTIVE_HIGH;
	inpCfg.mode_upsw = INPUT_MODE_DIGITAL;
	inpCfg.act_lvl_upsw = INPUT_ACTIVE_LOW;
	inpCfg.mode_dnsw = INPUT_MODE_DIGITAL;
	inpCfg.act_lvl_dnsw = INPUT_ACTIVE_LOW;
	inpCfg.act_lvl_mode = INPUT_ACTIVE_LOW;
	
	INDRV_Init(&inpCfg);
	DSPDRV_Init();
	COILDRV_Init();
	
	GatherData(100);
	
	//main loop
	while(1)
	{
		//Gather data
		GatherData(1);
		COILDRV_UpdateMeasurments(dccd_i,dccd_u,supply_u);
	
		//Process inputs
		user_force = ProcessUpDownButtons(user_force);
		brakes_mode = ProcessMode(brakes_mode);
		
		//Determine next force output
		set_force = GetNextTargetForce();
		
		//Convert force to target current
		set_current = ForceToCurrent(set_force);
		COILDRV_SetTarget(set_current);
		
		//Process coil HW control
		COILDRV_Process();		
		
		//Process display
		ProcessDisplay();
		
	}
}

/**** Private function definitions ****/
void GatherData(uint8_t times)
{
	for(uint8_t i=0; i<times; i++)
	{
		INDRV_ReadAll();
	}
	
	supply_u = INDRV_GetAnalog(ANALOG_CH_BATTERY);
	dccd_u = INDRV_GetAnalog(ANALOG_CH_DCCD_U);
	dccd_i = INDRV_GetAnalog(ANALOG_CH_DCCD_I);
	pot = INDRV_GetAnalog(ANALOG_CH_POT);

	hbrake_act = INDRV_GetInputState(INPUT_CH_HBRAKE);
	brakes_act = INDRV_GetInputState(INPUT_CH_BRAKE);
	dimm_act = INDRV_GetInputState(INPUT_CH_DIMM);

	upsw_act = INDRV_GetInputState(INPUT_CH_UPSW);
	dnsw_act = INDRV_GetInputState(INPUT_CH_DNSW);
	mode_act = INDRV_GetInputState(INPUT_CH_MODE);
}

uint8_t ProcessUpDownButtons(uint8_t force)
{
	uint8_t upsw_changed = INDRV_GetInputChange(INPUT_CH_UPSW);
	uint8_t dnsw_changed = INDRV_GetInputChange(INPUT_CH_DNSW);
	
	INDRV_ResetChangeFlag(INPUT_CH_UPSW);
	INDRV_ResetChangeFlag(INPUT_CH_DNSW);
	
	if((upsw_changed)&&(upsw_act))
	{
		if(force<=90) force+=10;
		else force=100;
	};
	
	if((dnsw_changed)&&(dnsw_act))
	{
		if(force>=10) force-=10;
		else force=0;
	};
	
	return force;
}

uint8_t ProcessMode(uint8_t mode)
{
	uint8_t mode_changed = INDRV_GetInputChange(INPUT_CH_MODE);
	
	INDRV_ResetChangeFlag(INPUT_CH_MODE);
	
	if((mode_changed)&&(mode_act))
	{
		if(mode<2) mode++;
		else mode=0;
	};
	
	return mode;
}

uint8_t GetNextTargetForce(void)
{
	uint8_t force = 0;
	
	if(hbrake_act)
	{
		force = 0;
	}
	else if(brakes_act)
	{
		switch(brakes_mode)
		{
			case 1:
			force = user_force;
			break;
			
			case 2:
			force = 100;
			break;
			
			default:
			force = 0;
		}
	}
	else
	{
		force = user_force;
	}
	
	return force;
}

uint16_t ForceToCurrent(uint8_t force)
{
	if(force>=100) return COIL_MAX_SET_CURRENT;
	else if(force==0) return 0;
	else
	{
		uint32_t temp = ((uint32_t)force*COIL_MAX_SET_CURRENT)/100;
		return (uint16_t)temp;
	}
}

void ProcessDisplay(void)
{
	static uint16_t timer = 0;
	static uint8_t step = 0;
	
	//Display set force
	if( COILDRV_GetFault() )
	{
		//Flash LOCK LED
		if(timer) timer--;
		else
		{
			if(step)
			{
				//Turn on LOCK LED
				DSPDRV_SetDisplayBW(0x20);
				step=0;
			}
			else
			{
				//Turn off all LEDs
				DSPDRV_SetDisplayBW(0x00);
				step=1;
			}
			timer = DSP_FAULT_FLASH_PERIOD;
		}
	}
	else
	{
		DSPDRV_SetDisplayVal(set_force,LED_DSP_DOT10);
	}
	
	//Brightness control
	if(dimm_act) DSPDRV_SetDisplayBrightness(DSP_DIMM_PWM);
	else DSPDRV_SetDisplayBrightness(DSP_BRIGHT_PWM);
	
	//Process display HW control
	DSPDRV_RefreshDisplay();
}