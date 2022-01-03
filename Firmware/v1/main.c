/*
uDCCD controller

Author: Andis Jargans

Revision history:
2020-09-24: Initial version
2021-06-16: v1.1
* OCP too is sensitive. Coil inductance is larger than tough and current decays much slower. Need to give more time for current decay before triggering OCP.
* inrush_counter treshold, in OCP State machine, raised from 25 to 100.
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

#include "Drivers/analog_driver.h"
#include "Drivers/inputs_driver.h"
#include "Drivers/outputs_driver.h"

/**** Private definitions ****/
/*EEPROM config
Coil mode - V/C
Coil resistance - Rr
Coil voltage - Vv
Coil current - Aa
LED voltage - H/L
Brake mode - L/O
*/

#define EE_BMODE_ADD	0x00
#define EE_POTM_ADD		0x01
#define EE_LEDL_ADD		0x02
#define EE_LEDH_ADD		0x03
#define EE_ITRGT_ADDH	0x04
#define EE_ITRGT_ADDL	0x05

//DCCD state
#define DCCD_NOCAL	0
#define DCCD_

/*
Brake lock mode, O-0x4F, L-0x4C
Potentiometer mode, B-0x42, P-0x50
Led min PWM - dec2hex
Led max PWM - dec2hec
I target - dec2hex
*/

/**** Private variables ****/
//LED Config variables
uint8_t led_dimm_pwm = 10;
uint8_t led_high_pwm = 50;

//Potentiometer logic variables
uint8_t pot_en =0;
uint8_t pot_changed =0;
uint8_t pot_debounce =0;

//DCCD logic variables
uint8_t setforce = 0;
uint8_t actforce = 0;
uint8_t brakelock = 0;

//Protection logic variables
uint8_t ocp_state = 0;
uint16_t current_totmax = 6000; 
uint16_t current_actlim = 0;
uint8_t inrush_counter = 0;
uint16_t cooldown_counter = 0;
uint8_t retry_counter = 0;
uint8_t load_loss = 0;
uint8_t load_loss_debounce = 0;

//DCCD Operation state machine variables
uint8_t dccd_state = 0;
uint16_t calib_timer = 0;
uint8_t calibration_fail = 0;
uint16_t dccd_itarget = 3500;

//Display control variables
uint8_t show_cfg = 1;
uint16_t dsp_lock_timer = 0;

//Debugging variables
volatile uint16_t current = 0;
volatile uint16_t volatge = 0;
volatile uint16_t resistance = 0;

/**** Private function declarations ****/
void Init_systick(void);
void Init_watchdog(void);
void Init_ReducePower(void);
uint8_t Pot_Convert(uint16_t volt);

/**** Application ****/
int main(void)
{
	ANDRV_Init();  //T=0.4ms
	INDRV_Init();
	OUTDRV_Init();
	//Init_ReducePower();
	//Init_systick();
	//Init_watchdog();
	
	//Read default cfg from EEPROM
	uint8_t ee_cfg = 0;
	
	ee_cfg = eeprom_read_byte((uint8_t*)EE_BMODE_ADD);
	if(ee_cfg=='L') brakelock=1;
	else brakelock=0;
	
	ee_cfg = eeprom_read_byte((uint8_t*)EE_POTM_ADD);
	if(ee_cfg=='P') pot_en=1;
	else pot_en=0;
	
	led_dimm_pwm = eeprom_read_byte((uint8_t*)EE_LEDL_ADD);
	if(led_dimm_pwm>100) led_dimm_pwm=100;
	else if(led_dimm_pwm<5) led_dimm_pwm=5;
	
	led_high_pwm = eeprom_read_byte((uint8_t*)EE_LEDH_ADD);
	if(led_high_pwm>100) led_high_pwm=100;
	else if(led_high_pwm<5) led_high_pwm=5;
	
	ee_cfg = eeprom_read_word((uint16_t*)EE_ITRGT_ADDH);
	dccd_itarget = ((uint16_t)ee_cfg<<8);
	ee_cfg = eeprom_read_word((uint16_t*)EE_ITRGT_ADDL);
	dccd_itarget += ee_cfg;  
	
	if(dccd_itarget>4000) dccd_itarget = 4000;
	
	//Debugging
	//dccd_itarget = 3500;
	//led_hv= 1;
	//pot_en = 1;
	dccd_state = 0;
	ocp_state = 0;

	ANDRV_MeasureAll();
	INDRV_ReadAllTimed();
	//wdt_reset();	

	//main loop
	while(1)
	{
		/***** Sort of systick functions *****/
		ANDRV_MeasureAll();
		INDRV_ReadAllTimed();
	
		/***** Start of protection state machine *****/
				
		current = ANDRV_GetValue(AN_DCCDI);
		volatge = ANDRV_GetValue(AN_DCCDU);
		
		if(current) resistance = (uint16_t)(((uint32_t)volatge*1000)/current);
		else if(actforce) resistance = 0xFFFF;
		else resistance = 0;
		
		if(actforce) current_actlim = (uint16_t)(((uint32_t)actforce*current_totmax)/100);
		else current_actlim = current_totmax;
		
		//check values
		if(current>current_actlim)
		{
			if(inrush_counter<254) inrush_counter+=5;
			if(cooldown_counter) cooldown_counter = 2000;
		}
		else if((resistance<500)&&(current!=0))
		{
			if(inrush_counter<254) inrush_counter++;
			if(cooldown_counter) cooldown_counter = 2000;
		}
		else
		{
			//No overcurrent
			if(inrush_counter) inrush_counter--;
			if(cooldown_counter) cooldown_counter--;
		}
		
		//Check for under-current (load-loss)
		uint16_t res_lim = OUTDRV_GetResistance();
		res_lim *= 2;
		
		if(resistance>res_lim)
		{
			if(load_loss_debounce>20) load_loss = 1;
			else load_loss_debounce++;
		}
		else 
		{
			load_loss_debounce = 0;
			load_loss = 0;
		}
		
		//OCP state machine
		if(ocp_state==0)
		{
			//All is fine			
			if(inrush_counter>100)
			{
				//too long inrush, fuse
				OUTDRV_FuseDccd();
				actforce = 0;
				//go to FUSED state, and start cooldown time
				ocp_state = 1;
				cooldown_counter = 2000;
				if(dccd_state<2) dccd_state = 0;
			};
		}
		else if(ocp_state==1)
		{
			//Fused
			if(retry_counter>5)
			{
				//if still fused, after more that 5 retrys in row, then lock forever 
				ocp_state = 3;
			}
			else
			{
				if(!cooldown_counter)
				{
					//cooldown passed, retry
					OUTDRV_UnfuseDccd();
					ocp_state = 2;
					//Start time
					cooldown_counter = 2000;
				};
			}
		}
		else if(ocp_state==2)
		{
			//retrying, check if fault repeats
			if(inrush_counter>100)
			{
				//too long inrush, fuse
				OUTDRV_FuseDccd();
				actforce = 0;
				//go to FUSED state, and start cooldown time
				ocp_state = 1;
				cooldown_counter = 2000;
				retry_counter++;
			}
			else if(!cooldown_counter)
			{
				//no fault repeated, return to normal state
				ocp_state = 0;
				retry_counter = 0;
			}
		}
		else
		{
			//Lockdown
			//Just never unfuse DCCD Driver
		}
		/***** End of protection state machine *****/

		/***** Start of Input switch logic *****/	
		//Mode input processing
		if((INDRV_GetInput(IN_MODE))&&(INDRV_GetInputChange(IN_MODE)))
		{
			//do switch operations
			if(brakelock)
			{
				brakelock = 0;
				eeprom_write_byte((uint8_t*)EE_BMODE_ADD,'O');
			}
			else
			{
				brakelock = 1;
				eeprom_write_byte((uint8_t*)EE_BMODE_ADD,'L');
			}
			
			INDRV_ResetInputChange(IN_MODE);				
		};

		if(pot_en)
		{
			//analog potentiometer mode
			uint8_t new_force = Pot_Convert(ANDRV_GetValue(AN_POTU));
			if(setforce!=new_force)
			{
				pot_debounce++;
				
				if(pot_debounce>10)
				{
					//pot changed, set new force.
					setforce = new_force;
					pot_changed = 1;
					pot_debounce = 0;
				}
			}
			else
			{
				pot_changed = 0;
				pot_debounce = 0;
			}
		}
		else
		{
			//button mode
			if((INDRV_GetInput(IN_UPSW))&&(INDRV_GetInputChange(IN_UPSW)))
			{
				//do switch operations
				if(setforce>90) setforce = 100;
				else setforce += 10;
				INDRV_ResetInputChange(IN_UPSW);
			};
			
			if((INDRV_GetInput(IN_DOWN))&&(INDRV_GetInputChange(IN_DOWN)))
			{
				//do switch operations
				if(setforce<10) setforce = 0;
				else setforce -= 10;
				INDRV_ResetInputChange(IN_DOWN);
			};
		}
		
		/***** End of Input switch logic *****/

		/***** Start of DCCD force logic *****/
		if(OUTDRV_GetFuseSate())
		{
			actforce = 0;
		}
		else
		{
			if(INDRV_GetInput(IN_HBRAKE))
			{
				actforce = 0;
			}
			else
			{
				if(INDRV_GetInput(IN_BRAKE))
				{
					if(brakelock) actforce = 100;
					else actforce = 0;
				}
				else actforce = setforce;
			}
		}
		/***** End of DCCD force logic *****/
		
		/***** Start of DCCD Operation state machine *****/
		if(dccd_state==0)
		{
			//Set up for calibration
			//Force 20% PWM
			OUTDRV_SetDccdPWM(20,PWM_FULL);
			calib_timer = 500;
			dccd_state = 1;
		}
		else if(dccd_state==1)
		{
			//Calibration in progress, also wait for retry to end
			if((!calib_timer)&&(!ocp_state))
			{
				//uint16_t volatge = ANDRV_GetValue(AN_DCCDU);   //5.67V -> 1.38V
				//uint16_t current = ANDRV_GetValue(AN_DCCDI);   //1.75V -> 3.5A
				resistance = (uint16_t)(((uint32_t)volatge*1000)/current);
				
				if(resistance>10000)
				{
					//resistance too large, load loss.
					calibration_fail = 1;
					dccd_state = 0;
				}
				else
				{
					OUTDRV_SetCoilConfig(resistance,12000,dccd_itarget);
					OUTDRV_SetDccdPWM(actforce,1);
					dccd_state = 2;
					calibration_fail = 0;
				}
			}
			else
			{
				if(calib_timer) calib_timer--;
			}
		}
		else
		{
			//normal operation mode
			// Set previously determined DCCD force
			OUTDRV_SetDccdPWM(actforce,1);
		}
		
		OUTDRV_ProcessCoil(ANDRV_GetValue(AN_BATU));
				
		/***** End of DCCD Operation state machine *****/
		
		/***** Start of Display processing *****/
		//Dimm switch processing
		if(INDRV_GetInputChange(IN_DIMM))
		{
			if(INDRV_GetInput(IN_DIMM))
			{
				OUTDRV_SetDisplayPWM(led_dimm_pwm);
			}
			else
			{
				OUTDRV_SetDisplayPWM(led_high_pwm);
			}
			INDRV_ResetInputChange(IN_DIMM);
		};
		
		//Display value processing
		if((INDRV_GetInput(IN_MODE))||(show_cfg))
		{
			//while mode button is pressed show mode
			if(brakelock) OUTDRV_SetDisplayBW(0x38);
			else OUTDRV_SetDisplayBW(0x07);
			dsp_lock_timer = 2000;
			show_cfg = 0;
		}
		else
		{
			//while UP or DOWN button is pressed show set force value
			if((INDRV_GetInput(IN_UPSW))||(INDRV_GetInput(IN_DOWN))||(pot_changed))
			{
				OUTDRV_SetDisplayVal(setforce,DSP_EXTDOT);
				dsp_lock_timer = 2000;
			}
			else
			{
				
				if(dsp_lock_timer) dsp_lock_timer--;
				else
				{
					if(ocp_state>2)
					{
						//flash OCP fault LED						
						static uint8_t togle_led = 0;
						if(togle_led) OUTDRV_SetDisplayBW(0x20);
						else OUTDRV_SetDisplayBW(0x00);
						togle_led ^= 0x01;
						dsp_lock_timer = 1000;
					}
					else if((load_loss)||(calibration_fail))
					{
						//flash under-current (load loss) fault
						static uint8_t togle_led = 0;
						if((togle_led)&&(calibration_fail)) OUTDRV_SetDisplayBW(0x01);
						else if((togle_led)&&(!calibration_fail)) OUTDRV_SetDisplayVal(actforce,DSP_EXTDOT);
						else OUTDRV_SetDisplayBW(0x00);
						togle_led ^= 0x01;
						dsp_lock_timer = 500;
					}
					else
					{
						//show actual force
						OUTDRV_SetDisplayVal(actforce,DSP_EXTDOT);
					}
				}
			}
		}
				
		OUTDRV_RefreshDisplay();
		
		/***** End of Display processing *****/
		
		//wdt_reset();	

	}  //end of loop
	
} //end of main

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

uint8_t Pot_Convert(uint16_t volt)
{
	volt /= 50;
	if(volt>90) volt = 100;
	else if(volt<10) volt = 0;
	
	return (uint8_t)volt;
}
