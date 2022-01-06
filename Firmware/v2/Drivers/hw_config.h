/*
uDCCD controller
Global Hardware configuartion

Author: Andis Jargans

Revision history:
v0.0 - YYYY-MM-DD: Initial version
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

//Hardware specific configuration
#define CFG_ADC_CLK_PRESCALER		0x06
#define CFG_PWM_TIMER_PRESCALER		0x01
#define CFG_PWM_TIMER_TOP			256	
#define CFG_PWM_TIMER_DCCD_MAX		250

//Coil Driver configuration
#define COIL_MAX_OUT_CURRENT		6000
#define COIL_MAX_SET_CURRENT		4500
#define COIL_MIN_SET_CURRENT		100
#define COIL_MAX_OUT_VOLTAGE		10000
#define COIL_MIN_OUT_VOLTAGE		100
#define COIL_FAULT_COOLDOWN_TIME	1000
#define COIL_OCP_LIMIT_PERCENT		160
#define COIL_OUT_OVP_LIMIT_DEFAULT	6000
#define COIL_OUT_OVP_LIMIT_PERCENT	120
#define COIL_OUT_UVP_LIMIT_PERCENT	80
#define COIL_SUPPLY_MIN				10000
#define COIL_SUPPLY_MAX				18000

//PID unsigned 8bit coeficients
//Kp=3, Ki = 1, Kd = 0.1
#define COIL_PID_KP_MUL		3
#define COIL_PID_KP_DIV		1
#define COIL_PID_KI_MUL		1
#define COIL_PID_KI_DIV		1
#define COIL_PID_KD_MUL		1
#define COIL_PID_KD_DIV		10

//Inputs Driver configuration
#define DEBOUNCE_TIME_DIMM		10
#define DEBOUNCE_TIME_BRKAE		10
#define DEBOUNCE_TIME_HBRAKE	10
#define DEBOUNCE_TIME_UPSW		10
#define DEBOUNCE_TIME_DNSW		10
#define DEBOUNCE_TIME_MODESW	10

//Application configuration
#define DSP_FAULT_FLASH_PERIOD	500
#define DSP_BRIGHT_PWM			50
#define DSP_DIMM_PWM			10

#define POT_RANGE_BOTTOM		100
#define POT_RANGE_TOP			4900
	
#endif