/*
uDCCD controller
System hardware abstraction layer

Author: Andis Jargans

Revision history:
2021-MM-DD: Initial version	
*/

/**** Hardware configuration ****

*/

/**** Includes ****/
#include <avr/io.h>
#include <avr/wdt.h>
#include "hal_system.h"
#include "hw_config.h"

/**** Private variables ****/
static volatile uint8_t bootstraps = 0x00;

/**** Private function declarations ****/

/**** Public function definitions ****/
void SYSHAL_Init_systick(void)
{
	//Generic timing timer configuration
	TCCR0A = 0x00; //Normal mode
	TCNT0 = 0x00;
	OCR0A = 0x00;
	OCR0B = 0x00;
	TIMSK0 = 0x01; //OVF interrupt
		
	TCCR0A |= 0x03;  //Enable timer, with div64, T=2ms @8MHz
}

void SYSHAL_Init_watchdog(void)
{
	//watchdog timer setup
	WDTCSR |= 0x10; //Change enable
	WDTCSR |= 0x0D; //System reset mode, 0.5s period.
	//use special instruction to reset watchdog timer
}

void SYSHAL_ReducePower(void)
{
	//Disable unnecessary peripherals 
	PRR = 0x84;  //TWI and SPI
}

void SYSHAL_ResetWatchdog(void)
{
	//Do WDT reset instruction
}

/**
 * @brief Initializes bootstraps hardware
 */
void SYSHAL_InitBootstraps(void)
{	
	//Digital inputs configuration
	//Bootstraps
	DDRA &= ~0x0E; //Set as inputs
	PORTA |= 0x0E; //Enable MCU pull-up
	
	bootstraps = 0x00;
}

/**
 * @brief Turn off bootstrap pull-up's, to save power
 */
void SYSHAL_DeInitBootstraps(void)
{
	PORTA &= ~0x0E; //Disable pullup`s
}

/**
 * @brief Read and latch bootstraps
 */
uint8_t SYSHAL_LatchBootstraps(void)
{
	//Init HW
	bootstraps = 0x00;
	INHAL_InitBootstraps();
	
	//Read value
	uint8_t i = 0;
	i = INHAL_GPIORead(INHAL_DIN_BOOT0);
	bootstraps |= i;
	i = INHAL_GPIORead(INHAL_DIN_BOOT1);
	bootstraps |= (i<<1);
	i = INHAL_GPIORead(INHAL_DIN_BOOT2);
	bootstraps |= (i<<2);
	
	INHAL_DeInitBootstraps();
	
	return bootstraps;
}
/**
 * @brief Read chosen bootstrap
 * @param [in] ch Channel to read
 * @return Properly converted analog value as mV or mA
 */
uint8_t SYSHAL_GetBootstrap(uint8_t ch)
{
	uint8_t i=0;
	
	switch(ch)
	{
		case INPUT_BOOT0:
			i = bootstraps&0x01;
			break;
			
		case INPUT_BOOT1:
			i = bootstraps&0x02;
			break;
			
		case INPUT_BOOT2:
			i = bootstraps&0x04;
			break;
			
		default:
			return bootstraps;
	}
	
	if(i) return 1;
	else return 0;
}

/**** Private function definitions ****/