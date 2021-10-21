/*
uDCCD controller
Global Hardware configuartion

Author: Andis Jargans

Revision history:
2021-MM-DD: Initial version
*/

/**** Hardware configuration ****
PA0 - 
PA1 - BOOT0 - Active low, needs integrated pull-up.
PA2 - BOOT1 - Active low, needs integrated pull-up.
PA3 - BOOT2 - Active low, needs integrated pull-up.

PB0 - DIMM_IN - Active low
PB1 - DCCD_CTRL - OC1A, Active high
PB2 - LED_PWM - OC1B, Active high
PB3 - MOSI
PB4 - MISO
PB5 - SCK
PB6 - HBRKAE_IN - Active low
PB7 - BRKAE_IN - Active low

PC0 - BAT_MON - Battery voltage, 20mV/LSB
PC1 - DCCD_UMON - DCCD Coil voltage, 20mV/LSB
PC2 - DCCD_IMON - DCCD Coil current, 10mA/LSB
PC3 - POT_IN - Analog force set
PC4 - UPSW_IN - Active low, Optional raw analog input
PC5 - DNSW_IN - Active low, Optional raw analog input
PC6 - RESET
PC7 - MODE_IN - Active low

PD0 - DSP0 - Active high GPO, OPEN
PD1 - DSP1 - Active high GPO, 20%
PD2 - DSP2 - Active high GPO, 40%
PD3 - DSP3 - Active high GPO, 60%
PD4 - DSP4 - Active high GPO, 80%
PD5 - DSP5 - Active high GPO, LOCK
PD6 - 
PD7 - 

Ftim = Fcpu/(2*TOP*DIV)
Fadc = Fcpu/DIV

ADC=Vin*1024/Vref

One conversion = 13.5 Fadc cycles
*/

#ifndef HW_CFG_FILE
#define HW_CFG_FILE

/**** Public definitions ****/

/**** Aplciation specific configuration ****/
//DCCD and LED timer static configuration
#define PWM_TIMER_PRESCALER	0x01
#define PWM_TIMER_TOP		256	
#define PWM_TIMER_DCCD_MAX	250

//ADC static configuartion
#define ADC_CLK_PRESCALER	0x06

//DCCD Driver configuartion
#define LOCK_CURRENT		4500
#define MAX_OUTPUT_VOLTAGE	10000

#define COIL_LAG_TIME		10

#define MAX_OUTPUT_CURRENT		8000
#define MAX_SUPPLY_VOLTAGE		18000
#define MIN_SUPPLY_VOLTAGE		7000
#define MIN_SUPPLY_VOLTAGE_CVM	10500
#define MAX_OUTPUT_POWER		40000
#define MIN_OUTPUT_IMPEDANCE	900
#define MAX_OUTPUT_IMPEDANCE	2200

#define FAULT_SUPPLY_DELAY	20
#define FAULT_OCP_DELAY		10
#define FAULT_OPP_DELAY		10
#define FAULT_SHORT_DELAY	10
#define FAULT_LOSS_DELAY	20

#define FAULT_OUTPUT_COOLDOWN	1000

//Inputs Driver configuartion


#endif