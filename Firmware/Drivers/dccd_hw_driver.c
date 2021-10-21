/*
uDCCD controller
LEDs driver

Author: Andis Jargans

Revision history:
2021-09-16: Initial version
*/

/**** Hardware configuration ****

*/

/**** Includes ****/
#include <avr/io.h>
#include "dccd_hw_driver.h"
#include "outputs_driver.h"

/**** Private definitions ****/

/**** Private variables ****/
static volatile uint16_t target_voltage = 0;
static volatile uint16_t set_output_voltage = 0;

static volatile uint16_t coil_resistance = 0;

static volatile uint8_t protection_deadtime = 0;
static volatile uint8_t protection_disable_zcheck = 0;
static volatile uint16_t cooldown_output = 0;

static volatile uint16_t out_z = 0;
static volatile uint16_t out_pwr = 0;

static volatile uint8_t warning_uvlo = 0;
static volatile uint8_t warning_ovlo = 0;
static volatile uint8_t warning_ocp = 0;
static volatile uint8_t warning_opp = 0;
static volatile uint8_t warning_short = 0;
static volatile uint8_t warning_load_loss = 0;

static volatile uint8_t counter_uvlo = 0;
static volatile uint8_t counter_ovlo = 0;
static volatile uint8_t counter_ocp = 0;
static volatile uint8_t counter_opp = 0;
static volatile uint8_t counter_short = 0;
static volatile uint8_t counter_load_loss = 0;

static volatile uint8_t fault_supply = 0;
static volatile uint8_t fault_output = 0;
static volatile uint8_t fault_loss = 0;

static volatile uint8_t fault_output_cnt = 0;

/**** Private function declarations ****/

/**** Public function definitions ****/
/**
 * @brief Initializes DCCD hardware driver
 */
void COILDRV_Init(void)
{
	OUTHAL_Init();
	OUTHAL_DisableDCCDch();
	OUTHAL_SetPWM(OUTHAL_CH_DCCD,0);
}


void COILDRV_SetFroce(uint8_t force)
{
	if(coil_resistance)
	{
		//Calculate target voltage
		uint32_t temp = (uint32_t)coil_resistance*LOCK_CURRENT;
		
		if(force>100) target_voltage = (uint16_t)temp;
		else if(force==0) target_voltage = 0;
		else target_voltage = (uint16_t)((temp/100)*force);
	}
	else
	{
		//Scale target voltage from lock voltage
		if(force>100) target_voltage = MAX_OUTPUT_VOLTAGE;
		else if(force==0) target_voltage = 0;
		else target_voltage = (uint16_t)(((uint32_t)force*MAX_OUTPUT_VOLTAGE)/100);
	}
}

/**
 * @brief Calculate necessary PWM duty cycle for target voltage
 * @param [in] u_out Desired output voltage
 * @return PWM value in permille [0..1000]
 */
uint16_t OutputVoltageToPWM(uint16_t u_out, uint16_t u_sup)
{
	if((u_sup)&&(u_out))
	{
		//Compensate for supply voltage changes
		uint32_t temp = ((uint32_t)u_out*1000)/u_sup;
		if(temp>0x0000FFFF) return 0xFFFF;
		else return (uint16_t)temp;
	}
	else return 0;
}


void COILDRV_ProcessLogic(uint16_t u_supply)
{
	if((fault_output)||(fault_supply))
	{
		//Turn off output
		OUTHAL_DisableDCCDch();
		OUTHAL_SetPWM(OUTHAL_CH_DCCD,0);
		set_output_voltage = 0;
	}
	else
	{
		if(set_output_voltage!=target_voltage)
		{
			protection_deadtime = COIL_LAG_TIME;
			set_output_voltage = target_voltage;
		}
	}
	
	if(set_output_voltage) protection_disable_zcheck = 0;
	else protection_disable_zcheck = 1;
	
	uint16_t pwm = OutputVoltageToPWM(set_output_voltage,u_supply);
	OUTHAL_SetPWMPrecise(OUTHAL_CH_DCCD,pwm);
}

void COILDRV_ProcessProtection(uint16_t i_dccd, uint16_t u_dccd, uint16_t u_supply)
{
	uint32_t temp = 0;
		
	//Calculate output impedance
	if(i_dccd)
	{
		temp = ((uint32_t)u_dccd*1000)/((uint32_t)i_dccd);
		if(temp>0x0000FFFF) out_z = 0xFFFF;
		else out_z = (uint16_t)temp;
	}
	else out_z = 0xFFFF;
	
	//Calculate output power
	temp = ((uint32_t)u_dccd*i_dccd)/1000;
	if(temp>0x0000FFFF) out_pwr = 0xFFFF;
	else out_pwr = (uint16_t)temp;
	
	//Check for various warnings -------------
	//Under voltage lockout
	if(coil_resistance)
	{
		if((u_supply<MIN_SUPPLY_VOLTAGE)&&(MIN_SUPPLY_VOLTAGE!=0)) warning_uvlo = 1;
		else warning_uvlo = 0;
	}
	else
	{
		if((u_supply<MIN_SUPPLY_VOLTAGE_CVM)&&(MIN_SUPPLY_VOLTAGE_CVM!=0)) warning_uvlo = 1;
		else warning_uvlo = 0;
	}
	
	//Over voltage lockout
	if((u_supply>MAX_SUPPLY_VOLTAGE)&&(MAX_SUPPLY_VOLTAGE!=0)) warning_ovlo = 1;
	else warning_ovlo = 0;
	
	//Over current protection
	if((i_dccd>MAX_OUTPUT_CURRENT)&&(MAX_OUTPUT_CURRENT!=0)) warning_ocp = 1;
	else warning_ocp = 0;
	
	//Over power protection
	if((out_pwr>MAX_OUTPUT_POWER)&&(MAX_OUTPUT_POWER!=0)) warning_opp = 1;
	else warning_opp = 0;
	
	//Short circuit protection
	if((out_z<MIN_OUTPUT_IMPEDANCE)&&(MIN_OUTPUT_IMPEDANCE!=0)) warning_short = 1;
	else warning_short = 0;
	
	//Load loss protection
	if((out_z>MAX_OUTPUT_IMPEDANCE)&&(MAX_OUTPUT_IMPEDANCE!=0)) warning_load_loss = 1;
	else warning_load_loss = 0;
	
	//Fault counters -------------------	
	if(warning_uvlo){ if(counter_uvlo<255)counter_uvlo++;}
	else counter_uvlo--;
	
	if(warning_ovlo){ if(counter_ovlo<255)counter_ovlo++;}
	else counter_ovlo--;
	
	if(warning_ocp){ if(counter_ocp<255)counter_ocp++;}
	else counter_ocp--;
	
	if(warning_opp){ if(counter_opp<255)counter_opp++;}
	else counter_opp--;
	
	if((warning_short)&&(!protection_deadtime)&&(!protection_disable_zcheck))
	{
		if(counter_short<255)counter_short++;
	}
	else counter_short--;
	
	if((warning_load_loss)&&(!protection_deadtime)&&(!protection_disable_zcheck))
	{
		if(counter_load_loss<255)counter_load_loss++;
	}
	else counter_load_loss--;
	
	//Check fault counters
	if((counter_uvlo>FAULT_SUPPLY_DELAY)||(counter_ovlo>FAULT_SUPPLY_DELAY))
	{
		//Set supply voltage fault
		fault_supply = 1;
	}
	else
	{
		//Reset supply voltage fault
		fault_supply = 0;
	}
	
	if((counter_ocp>FAULT_OCP_DELAY)||(counter_opp>FAULT_OPP_DELAY)||(counter_short>FAULT_SHORT_DELAY))
	{
		//Set output power fault
		if(!fault_output) fault_output_cnt++;
		fault_output = 1;
		cooldown_output = FAULT_OUTPUT_COOLDOWN;
	}
	else
	{
		if(cooldown_output) cooldown_output--;
		else
		{
			//Cooldown ended, reset fault flag
			fault_output = 0;
		}
	}
	
	if(counter_load_loss>FAULT_LOSS_DELAY)
	{
		//Set load loss fault
		fault_loss = 1;
	}
	else
	{
		//Reset load loss fault
		fault_loss = 0;
	}
	
	//Reduce protection dead time
	if(protection_deadtime) protection_deadtime--;
	
	//Act on faults - disable output
	
}