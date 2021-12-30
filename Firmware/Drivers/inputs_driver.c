/*
uDCCD controller
Analog and Digital inputs driver

Author: Andis Jargans

Revision history:
v0.0 - YYYY-MM-DD: Initial version
*/

/***** Includes *****/
#include "inputs_driver.h"

/***** Private definitions *****/
typedef struct inStateStruct {
	uint8_t type;
	uint8_t act_lvl;
	uint8_t dbnc_timer;
	uint8_t state;
	uint8_t changed;
}inStateDef;

typedef struct AnalogStruct {
	uint16_t battery;
	uint16_t dccd_u;
	uint16_t dccd_i;
	uint16_t pot;
	uint16_t upsw;
	uint16_t dnsw;
}analogDef;

/***** Private variables *****/
static inStateDef dimm;
static inStateDef brake;
static inStateDef hbrake;
static inStateDef upsw;
static inStateDef dnsw;
static inStateDef modesw;

static analogDef measurments;

static uint8_t bootstraps;

/***** Private function declarations *****/
void ReadAllAnalog(void);
void ReadAllDigital(void);
void DoDebounce(inStateDef* input, uint8_t level, uint8_t dbnc_limit);

/***** Public function definitions *****/
/**
 * @brief Initializes Inputs Driver
 * @param [in] initCfg Inputs configuration
 */
void INDRV_Init(inInitDef* initCfg)
{
	//HV input configuration
	dimm.type = INPUT_MODE_DIGITAL;
	dimm.act_lvl = initCfg->act_lvl_dimm;
	dimm.dbnc_timer = 0;
	dimm.state = 0;
	dimm.changed = 0;
	
	brake.type = INPUT_MODE_DIGITAL;
	brake.act_lvl = initCfg->act_lvl_brake;
	brake.dbnc_timer = 0;
	brake.state = 0;
	brake.changed = 0;
	
	hbrake.type = INPUT_MODE_DIGITAL;
	hbrake.act_lvl = initCfg->act_lvl_hbrake;
	hbrake.dbnc_timer = 0;
	hbrake.state = 0;
	hbrake.changed = 0;
	
	//Low voltage input configuration	
	upsw.type = initCfg->mode_upsw;
	upsw.act_lvl = initCfg->act_lvl_upsw;
	upsw.dbnc_timer = 0;
	upsw.state = 0;
	upsw.changed = 0;
	
	dnsw.type = initCfg->mode_dnsw;
	dnsw.act_lvl = initCfg->act_lvl_dnsw;
	dnsw.dbnc_timer = 0;
	dnsw.state = 0;
	dnsw.changed = 0;
	
	modesw.type = INPUT_MODE_DIGITAL;
	modesw.act_lvl = initCfg->act_lvl_mode;
	modesw.dbnc_timer = 0;
	modesw.state = 0;
	modesw.changed = 0;
	
	//HAL init preparation -------------
	halInpConfigDef halCfg;
	
	halCfg.adc_wake = 1;

	//UP switch input HAL config
	if(upsw.type==INPUT_MODE_ANALOG) halCfg.upsw_mode = HAL_INP_MODE_AIN;
	else halCfg.upsw_mode = HAL_INP_MODE_DIN;
	
	//DOWN switch input HAL config
	if(dnsw.type==INPUT_MODE_ANALOG) halCfg.dnsw_mode = HAL_INP_MODE_AIN;
	else halCfg.dnsw_mode = HAL_INP_MODE_DIN;
	
	HAL_InitInputs(&halCfg);
}

/**
 * @brief Process all input channels
 */
void INDRV_ReadAll(void)
{
	ReadAllAnalog();
	ReadAllDigital();
}

/**
 * @brief Read voltage\current for chosen channel
 * @param [in] ch Channel to read
 * @return Properly converted analog value as mV or mA
 */
uint16_t INDRV_GetAnalog(uint8_t ch)
{
	switch(ch)
	{
		case ANALOG_CH_BATTERY:
			return measurments.battery;
			
		case ANALOG_CH_DCCD_U:
			return measurments.dccd_u;
			
		case ANALOG_CH_DCCD_I:
			return measurments.dccd_i;
			
		case ANALOG_CH_POT:
			return measurments.pot;
			
		case ANALOG_CH_UPSW:
			return measurments.upsw;
			
		case ANALOG_CH_DNSW:
			return measurments.dnsw;
			
		default:
			return 0xFFFF;
	}
}

/**
 * @brief Read activity state for chosen channel
 * @param [in] ch Channel to read
 * @return Activity status, 0-off, 1-on
 */
uint8_t INDRV_GetInputState(uint8_t ch)
{
	switch(ch)
	{
		case INPUT_CH_DIMM:
			return dimm.state;
			
		case INPUT_CH_BRAKE:
			return brake.state;
			
		case INPUT_CH_HBRAKE:
			return hbrake.state;
			
		case INPUT_CH_UPSW:
			return upsw.state;
			
		case INPUT_CH_DNSW:
			return dnsw.state;
			
		case INPUT_CH_MODE:
			return modesw.state;
			
		default:
			return 0;
	}
}

/**
 * @brief Read change flag for chosen channel
 * @param [in] ch Channel to read
 * @return State change status, 0-same, 1-changed
 */
uint8_t INDRV_GetInputChange(uint8_t ch)
{
	switch(ch)
	{
		case INPUT_CH_DIMM:
			return dimm.changed;
			
		case INPUT_CH_BRAKE:
			return brake.changed;
			
		case INPUT_CH_HBRAKE:
			return hbrake.changed;
			
		case INPUT_CH_UPSW:
			return upsw.changed;
			
		case INPUT_CH_DNSW:
			return dnsw.changed;
			
		case INPUT_CH_MODE:
			return modesw.changed;
			
		default:
			return 0;
	}
}

/**
 * @brief Reset change flag for chosen channel
 * @param [in] ch Channel to reset
 */
void INDRV_ResetChangeFlag(uint8_t ch)
{
	switch(ch)
	{
		case INPUT_CH_ALL:
			dimm.changed = 0;
			brake.changed = 0;
			hbrake.changed = 0;
			upsw.changed = 0;
			dnsw.changed = 0;
			modesw.changed = 0;
			break;
			
		case INPUT_CH_DIMM:
			dimm.changed = 0;
			break;
			
		case INPUT_CH_BRAKE:
			brake.changed = 0;
			break;
			
		case INPUT_CH_HBRAKE:
			hbrake.changed = 0;
			break;
			
		case INPUT_CH_UPSW:
			upsw.changed = 0;
			break;
			
		case INPUT_CH_DNSW:
			dnsw.changed = 0;
			break;
			
		case INPUT_CH_MODE:
			modesw.changed = 0;
			break;
			
		default:
			break;
	}
}

/***** Private function definitions *****/
/**
 * @brief Read and convert all analog channels. Also does ADC mapping
 */
void ReadAllAnalog(void)
{
	uint16_t raw = 0;
	
	raw = HAL_ADCRead(HAL_AIN_BATMON);
	measurments.battery = raw*20; //mV
	
	raw = HAL_ADCRead(HAL_AIN_UMON);
	measurments.dccd_u = raw*20;  //mV
	
	raw = HAL_ADCRead(HAL_AIN_IMON);
	measurments.dccd_i = raw*10; //mA
	
	raw = HAL_ADCRead(HAL_AIN_POT);
	measurments.pot = (raw*64)/13;  //mV
	
	raw = HAL_ADCRead(HAL_AIN_UPSW);
	measurments.upsw = (raw*64)/13;  //mV
	
	raw = HAL_ADCRead(HAL_AIN_DNSW);
	measurments.dnsw = (raw*64)/13;	 //mV
}

/**
 * @brief Read and debounce all digital channels
 */
void ReadAllDigital(void)
{
	uint8_t hwlvl = 0;
	
	hwlvl = HAL_GPIORead(HAL_DIN_DIMM);
	DoDebounce(&dimm, hwlvl, DEBOUNCE_TIME_DIMM);
	
	hwlvl = HAL_GPIORead(HAL_DIN_BRAKE);
	DoDebounce(&brake, hwlvl, DEBOUNCE_TIME_BRKAE);
	
	hwlvl = HAL_GPIORead(HAL_DIN_HBRAKE);
	DoDebounce(&hbrake, hwlvl, DEBOUNCE_TIME_HBRAKE);
	
	hwlvl = HAL_GPIORead(HAL_DIN_MODE);
	DoDebounce(&modesw, hwlvl, DEBOUNCE_TIME_MODESW);
	
	if(upsw.type!=INPUT_MODE_ANALOG)
	{
		hwlvl = HAL_GPIORead(HAL_DIN_UPSW);
		DoDebounce(&upsw, hwlvl, DEBOUNCE_TIME_UPSW);
	};
	
	if(dnsw.type!=INPUT_MODE_ANALOG)
	{
		hwlvl = HAL_GPIORead(HAL_DIN_DNSW);
		DoDebounce(&dnsw, hwlvl, DEBOUNCE_TIME_DNSW);
	};
}

/**
 * @brief Do debounce algorithm
 * @param [in/out] input Pointer to input`s data structure
 * @param [in] level Read pin level
 * @param [in] dbnc_limit Debounce time limit
 */
void DoDebounce(inStateDef* input, uint8_t level, uint8_t dbnc_limit)
{	
	uint8_t s = 0;
	//Determine state
	if(level==(input->act_lvl)) s=1;
	else s=0;
	
	//Do debounce logic
	if(s!=(input->state))
	{
		if(input->dbnc_timer>=dbnc_limit)
		{
			input->state = s;
			input->changed = 1;
			input->dbnc_timer = 0;
		}
		else input->dbnc_timer++;
	}
	else input->dbnc_timer = 0;
}