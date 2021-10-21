/*
uDCCD controller
Main aplication

Author: Andis Jargans

Revision history:
v2.0 - Firmware version for r7+ boards
2021-MM-DD - Initial version
*/

/**** Hardware configuration ****
PA0 - 
PA1 - BOOT0
PA2 - BOOT1
PA3 - BOOT2

PB0 - DIMM_IN
PB1 - DCCD_CTRL - OC1A
PB2 - LED_PWM - OC1B
PB3 - MOSI
PB4 - MISO
PB5 - SCK
PB6 - HBRKAE_IN
PB7 - BRKAE_IN

PC0 - BAT_MON
PC1 - DCCD_UMON - 20mV/LSB
PC2 - DCCD_IMON - 10mA/LSB
PC3 - POT_IN
PC4 - UPSW_IN
PC5 - DNSW_IN
PC6 - RESET
PC7 - MODE_IN

PD0 - DSP0
PD1 - DSP1
PD2 - DSP2
PD3 - DSP3
PD4 - DSP4
PD5 - DSP5
PD6 - 
PD7 - 
*/

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

//#include "Drivers/analog_driver.h"
//#include "Drivers/inputs_driver.h"
//#include "Drivers/outputs_driver.h"

/**** Private definitions ****/

/**** Private variables ****/

/**** Private function declarations ****/

/**** Application ****/
int main(void)
{
	
	//main loop
	while(1)
	{
		
	}
	
} 


/**** Private function definitions ****/
void Init_systick(void)
{
	//Generic timing timer configuration
	TCCR0A = 0x00; //Normal mode
	TCNT0 = 0x00;
	OCR0A = 0x00;
	OCR0B = 0x00;
	TIMSK0 = 0x01; //OVF interrupt
		
	TCCR0A |= 0x03;  //Enable timer, with div64, T=2ms @8MHz
}

void Init_watchdog(void)
{
	//watchdog timer setup
	WDTCSR |= 0x10; //Change enable
	WDTCSR |= 0x0D; //System reset mode, 0.5s period.
	//use special instruction to reset watchdog timer
}

void Init_ReducePower(void)
{
	//Disable unnecessary peripherals 
	PRR = 0x84;  //TWI and SPI
}
