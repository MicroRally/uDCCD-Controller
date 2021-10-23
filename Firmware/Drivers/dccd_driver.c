/*
uDCCD controller
LEDs driver

Author: Andis Jargans

Revision history:
2021-09-16: Initial version
2021-10-21: Modified to fit new outputs HAL, added descriptions
*/

/**** Hardware configuration ****

*/

/**** Includes ****/
#include <avr/io.h>
#include "dccd_driver.h"
#include "hal_outputs.h"
#include "hw_config.h"

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
uint16_t OutputVoltageToPWM(uint16_t u_out, uint16_t u_sup);
void SatAdd(uint8_t *base, uint8_t delta);
void SatSub(uint8_t *base, uint8_t delta);

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

/**
 * @brief Set target force (convert to target voltage)
 * @param [in] Force to set, 0 to 100
 */
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
 * @brief Reads fault flag of chosen type
 * @param [in] type Type of fault to read
 * @returns fault flag, 0-inactive, 1-active
 */
uint8_t COILDRV_GetFaultFlag(uint8_t type)
{
	switch(type)
	{
		case FAULT_SUPPLY:
			return fault_supply;
			
		case FAULT_OUTPUT:
			return fault_output;
			
		case FAULT_LOAD_LOSS:
			return fault_loss;
			
		default:
			return 0;
	}
}

/**
 * @brief Does actual control of DCCD driver
 * @param [in] u_supply Supply voltage, in mV
 */
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
	OUTHAL_SetPrecisePWM(OUTHAL_CH_DCCD,pwm);
}

/**
 * @brief Determines fault state
 * @param [in] i_dccd DCCD current, in mA
 * @param [in] u_dccd DCCD voltage, in mV
 * @param [in] u_supply Supply voltage, in mV
 */
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
	if(warning_uvlo){SatAdd(&counter_uvlo,1);}
	else SatSub(&counter_uvlo,1);
	
	if(warning_ovlo){SatAdd(&counter_ovlo,1);}
	else SatSub(&counter_ovlo,1);
	
	if(warning_ocp){SatAdd(&counter_ocp,1);}
	else SatSub(&counter_ocp,1);
	
	if(warning_opp){SatAdd(&counter_opp,1);}
	else SatSub(&counter_opp,1);
	
	if((warning_short)&&(!protection_deadtime)&&(!protection_disable_zcheck))
	{
		SatAdd(&counter_short,1);
	}
	else SatSub(&counter_short,1);
	
	if((warning_load_loss)&&(!protection_deadtime)&&(!protection_disable_zcheck))
	{
		SatAdd(&counter_load_loss,1);
	}
	else SatSub(&counter_load_loss,1);
	
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
	//Just set flags, acting on faults takes place in logic processing
}

/**** Private function definitions ****/
/**
 * @brief Calculate necessary PWM duty cycle for target voltage
 * @param [in] u_out Desired output voltage
 * @return PWM value in 1/10k [0..10000]
 */
uint16_t OutputVoltageToPWM(uint16_t u_out, uint16_t u_sup)
{
	if((u_sup)&&(u_out))
	{
		//Compensate for supply voltage changes
		uint32_t temp = ((uint32_t)u_out*10000)/u_sup;
		if(temp>0x0000FFFF) return 0xFFFF;
		else return (uint16_t)temp;
	}
	else return 0;
}

/**
 * @brief Does saturated add
 * @param [in/out] base Starting value
 * @param [in] delta Value to add
 */
void SatAdd(uint8_t *base, uint8_t delta)
{
	if((255-*base)>delta) *base += delta;
	else *base=255;
}

/**
 * @brief Does saturated subtraction
 * @param [in/out] base Starting value
 * @param [in] delta Value to subtract
 */
void SatSub(uint8_t *base, uint8_t delta)
{
	if(delta>*base) *base -= delta;
	else *base = 0;
}