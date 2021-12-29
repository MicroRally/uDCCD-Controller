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
#include "coil_driver.h"
#include "hal_outputs.h"
#include "hw_config.h"

/**** Private definitions ****/

/**** Private variables ****/

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

void COIlDRV_



/**** Private function definitions ****/