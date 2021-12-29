/*
uDCCD controller
Main application

Author: Andis Jargans

Revision history:
v2.0 - Firmware version for r7+ boards
2021-MM-DD - Initial version
*/

/**** Hardware configuration ****
*/

#include <avr/io.h>
#include <avr/eeprom.h>

#include "Drivers/hal_system.h"
#include "Drivers/inputs_driver.h"
#include "Drivers/dccd_driver.h"
#include "Drivers/leds_driver.h"

/**** Private definitions ****/

/**** Private variables ****/

/**** Private function declarations ****/
static void DataAcquisition(uint8_t times);

/**** Application ****/
int main(void)
{
	//System setup
	SYSHAL_InitBootstraps();
	LEDRV_Init();
	LEDDRV_SetDisplayBrightness(5);
	LEDDRV_SetDisplayVal(100,LED_DSP_BAR);
	LEDDRV_RefreshDisplay();
	COILDRV_Init();
	SYSHAL_LatchBootstraps();
		
	inInitDef inputsCfg;
	inputsCfg.act_lvl_dimm = INPUT_ACTIVE_HIGH;
	inputsCfg.act_lvl_brake = INPUT_ACTIVE_HIGH;
	inputsCfg.act_lvl_hbrake = INPUT_ACTIVE_LOW;
	inputsCfg.act_lvl_upsw = INPUT_ACTIVE_LOW;
	inputsCfg.act_lvl_dnsw = INPUT_ACTIVE_LOW;
	inputsCfg.act_lvl_mode = INPUT_ACTIVE_LOW;
	inputsCfg.mode_upsw = INPUT_MODE_DIGITAL;
	inputsCfg.mode_dnsw = INPUT_MODE_DIGITAL;
	
	INDRV_Init(&inputsCfg);
	
	DataAcquisition(100);	//Wait for inputs to stabilize, ~100ms
	INDRV_ResetChangeFlag(INPUT_CH_ALL);
	
	//main loop
	while(1)
	{
		//Gather data
		DataAcquisition(1);
		
		//Process protection
		
		//Determine next target output
		//Apply next target output
		//Set display
	}
} 


/**** Private function definitions ****/
void DataAcquisition(uint8_t times)
{
	do
	{
		INDRV_ReadAll();
		if(times) times--;
	}
	while(times>0)
}

uint8_t VoltsToForce(uint16_t mvolts)
{
	const uint16_t dzb = 500;	//bottom deadzone
	const uint16_t dzt = 500;	//top deadzone
	
	if(mvolts<=dzb) return 0;
	else if(mvolts>=dzt) return 100;
	else
	{
		uint32_t t = 100*((uint32_t)mvolts-dzb);
		t = t/(5000-(dzb+dzt));
		if(t>100) t=100;
		return (uint8_t)t;
	}
}