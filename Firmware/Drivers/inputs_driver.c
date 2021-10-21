/*
uDCCD controller
Inputs driver

Author: Andis Jargans

Revision history:
2021-MM-DD: Initial version
*/

/**** Hardware configuration ****

*/

/**** Includes ****/
#include "inputs_driver.h"
#include "hal_inputs.h"

/**** Private definitions ****/

/**** Private variables ****/

/**** Private function declarations ****/

/**** Public function definitions ****/

/**** Private function definitions ****/
/**
 * @brief Initializes Inputs Driver
 */
void INDRV_Init(void)
{
	inHalConfigDef halCfg;
	
	halCfg.adc_wake = 1;
	halCfg.pot_mode = INHAL_MODE_AIN;
	halCfg.upsw_mode = INHAL_MODE_DIN;
	halCfg.dnsw_mode = INHAL_MODE_DIN;
	halCfg.pot_pull = INHAL_PULL_NONE;
	halCfg.upsw_pull = INHAL_PULL_NONE;
	halCfg.dnsw_pull = INHAL_PULL_NONE;
	
	INHAL_Init(&inCfg);
}

/**** Private function definitions ****/