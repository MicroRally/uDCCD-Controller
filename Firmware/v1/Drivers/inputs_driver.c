/*
uDCCD controller
Inputs driver

Author: Andis Jargans

Revision history:
2020-09-24: Initial version	
*/

/**** Hardware configuration ****
PA1 - BOOT0 - Active low, pull-up
PA2 - BOOT1 - Active low, pull-up
PA3 - BOOT2 - Active low, pull-up

PB0 - DIMM_IN - Active low, pull-up
PB6 - HBRKAE_IN - Active high, pull-up
PB7 - BRKAE_IN - Active low, pull-up

PC4 - UPSW_IN - Active low, pull-up
PC5 - DNSW_IN - Active low, pull-up
PC7 - MODE_IN - Active low, pull-up 
*/

#include "inputs_driver.h"

/**** Private variables ****/
static volatile uint8_t dimm_sw = 0;
static volatile uint8_t hbrake_sw = 0;
static volatile uint8_t brake_sw = 0;
static volatile uint8_t up_sw = 0;
static volatile uint8_t down_sw = 0;
static volatile uint8_t mode_sw = 0;

static volatile uint8_t dimm_cnt = 0;
static volatile uint8_t hbrake_cnt = 0;
static volatile uint8_t brake_cnt = 0;
static volatile uint8_t up_cnt = 0;
static volatile uint8_t down_cnt = 0;
static volatile uint8_t mode_cnt = 0;

static volatile uint8_t dimm_chg = 0;
static volatile uint8_t hbrake_chg = 0;
static volatile uint8_t brake_chg = 0;
static volatile uint8_t up_chg = 0;
static volatile uint8_t down_chg = 0;
static volatile uint8_t mode_chg = 0;

/**** Public function definitions ****/
void INDRV_Init(void)
{
	//GPIO configuration
	DDRA &= ~0x0E; //Set as inputs
	PORTA |= 0x0E; //Enable pull-up
	
	DDRB &= ~0xC1; //Set as inputs
	PORTB |= 0xC1; //Enable pull-up
	
	DDRC &= ~0xB0; //Set as inputs
	PORTC |= 0xB0; //Enable pull-up
}

uint8_t INDRV_GetBootstrap(uint8_t ch)
{
	uint8_t ret_val = 0;
	
	switch(ch)
	{	
		case 0:
		if(PINA&0x02) ret_val = 1;
		else ret_val = 0;
		break;
		
		case 1:
		if(PINA&0x04) ret_val = 1;
		else ret_val = 0;
		break;
		
		case 2:
		if(PINA&0x08) ret_val = 1;
		else ret_val = 0;
		break;
		
		case 255:
		ret_val = ((PINA&0x0E)>>1);
		break;
		
		default:
		ret_val = 0;
	}
	
	return ret_val;
}

void INDRV_ReadAllTimed(void)
{
	uint8_t temp = 0;
	
	//Dimm input
	temp = (PINB&0x01);
	if(dimm_sw!=temp) dimm_cnt++;
	else dimm_cnt=0;
	
	if(dimm_cnt>10){dimm_sw = temp; dimm_chg = 1;};
	
	//Handbrake input
	temp = ((PINB&0x40)>>6);
	if(hbrake_sw!=temp) hbrake_cnt++;
	else hbrake_cnt=0;
	
	if(hbrake_cnt>10){hbrake_sw = temp; hbrake_chg = 1;};
	
	//Brake input
	temp = ((PINB&0x80)>>7);
	if(brake_sw!=temp) brake_cnt++;
	else brake_cnt=0;
	
	if(brake_cnt>10){brake_sw = temp; brake_chg = 1;};
	
	//Up switch input
	temp = ((PINC&0x10)>>4);
	if(up_sw!=temp) up_cnt++;
	else up_cnt=0;
	
	if(up_cnt>10){up_sw = temp; up_chg = 1;};
	
	//Down switch input
	temp = ((PINC&0x20)>>5);
	if(down_sw!=temp) down_cnt++;
	else down_cnt=0;
	
	if(down_cnt>10){down_sw = temp; down_chg = 1;};
	
	//Mode switch input
	temp = ((PINC&0x80)>>7);
	if(mode_sw!=temp) mode_cnt++;
	else mode_cnt=0;
	
	if(mode_cnt>10){mode_sw = temp; mode_chg = 1;};
}

uint8_t INDRV_GetInput(uint8_t ch)
{	
	switch(ch)
	{
		case 0:
		if(dimm_sw) return 0;
		else return 1;
		break;
		
		case 1:
		if(hbrake_sw) return 1;
		else return 0;
		break;
		
		case 2:
		if(brake_sw) return 0;
		else return 1;
		break;
		
		case 3:
		if(up_sw) return 0;
		else return 1;
		break;
		
		case 4:
		if(down_sw) return 0;
		else return 1;
		break;
		
		case 5:
		if(mode_sw) return 0;
		else return 1;
		break;
		
		default:
		return 0;
	}
}

uint8_t INDRV_GetInputChange(uint8_t ch)
{	
	switch(ch)
	{
		case 0:
		return dimm_chg;
		
		case 1:
		return hbrake_chg;
		
		case 2:
		return brake_chg;
		
		case 3:
		return up_chg;
		
		case 4:
		return down_chg;
		
		case 5:
		return mode_chg;
		
		default:
		return 0;
	}
}

void INDRV_ResetInputChange(uint8_t ch)
{
	switch(ch)
	{
		case 0:
		dimm_chg = 0;
		
		case 1:
		hbrake_chg = 0;
		
		case 2:
		brake_chg = 0;
		
		case 3:
		up_chg = 0;
		
		case 4:
		down_chg = 0;
		
		case 5:
		mode_chg = 0;
		
		default:
		return;
	}
}
