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

/**** Private function declarations ****/

/**** Application ****/
int main(void)
{
	//System setup
	
	//main loop
	while(1)
	{
		//Gather data
		
		//Process protection
		
		//Determine next target output
		//Apply next target output
		//Set display
	}
} 


/**** Private function definitions ****/