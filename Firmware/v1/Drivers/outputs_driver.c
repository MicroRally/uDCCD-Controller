/*
uDCCD controller
Outputs driver

Author: Andis Jargans

Revision history:
2020-09-24: Initial version	
*/

/***** Hardware configuration *****
PB1 - DCCD_CTRL - OC1A
PB2 - LED_PWM - OC1B

PD0 - DSP0 - Active high GPO, OPEN
PD1 - DSP1 - Active high GPO, 20%
PD2 - DSP2 - Active high GPO, 40%
PD3 - DSP3 - Active high GPO, 60%
PD4 - DSP4 - Active high GPO, 80%
PD5 - DSP5 - Active high GPO, LOCK

Ftim = Fcpu/(2*TOP*DIV)
*/

#include "outputs_driver.h"

/**** Private variables ****/
static volatile uint8_t tim1_clk = 0x01; //8kHz with 1000, @8MHz
static volatile uint16_t tim1_top = 256; //
static volatile uint16_t tim1_maxpwm = 250;//250; //93%

static volatile uint8_t set_image = 0x00;
static volatile uint16_t leds_timer = 0x0000;


static volatile uint16_t coil_resistance = 0;  //mR, no calibration done
static volatile uint16_t nominal_bat = 12000; //mV, no calibration done

static volatile uint16_t lock_pwm = 0;

static volatile uint8_t fuse_dccd = 0;
static volatile uint16_t coil_set_pwm = 0;

/**** Public function definitions ****/
void OUTDRV_Init(void)
{
	//GPIO configuration
	PORTB &= ~0x06; //Set low
	DDRB |= 0x06;  //Set as outputs
	
	PORTD &= ~0x3F; //Set low
	DDRD |= 0x3F;  //Set as outputs
	
	//DCCD and LED PWM Timer configuration
	TCCR1A = 0xA0; //Connect OC1A and OC1B
	TCCR1B = 0x10; //PWM, Phase & Frequency Correct ICR1 top, no clock
	TCCR1C = 0x00;
	TCNT1 = 0x0000;
	OCR1A = 0x0000;
	OCR1B = 0x0000;
	ICR1 = tim1_top; //TOP
	
	TCCR1B |= tim1_clk;  //Enable timer
}

/***** DISPLAY CONTROL FUNCTIONS *****/

//Put this in systick timer
void OUTDRV_RefreshDisplay(void)
{	
	uint8_t temp = PORTD;
	temp &= 0x3F;
	
	if(set_image!=temp)
	{
		//clear display
		PORTD &= ~0x3F;
		
		//set display
		PORTD |= (set_image&0x3F);
	};
}

void OUTDRV_SetDisplayBW(uint8_t image)
{
	//remove extra bits
	image &= 0x3F;
	
	set_image=image;
}

void OUTDRV_SetDisplayVal(uint8_t value, uint8_t style)
{	
	uint8_t image = 0x00;
	
	if(style==DSP_DOT)
	{
		//dot
		if(value<20) image = 0x01;
		else if(value<40) image = 0x02;
		else if(value<60) image = 0x04;
		else if(value<80) image = 0x08;
		else if(value<100) image = 0x10;
		else image = 0x20;
	}
	if(style==DSP_EXTDOT)
	{
		//extended dot
		if(value<10) image = 0x01;       //0
		else if(value<20) image = 0x03;  //10
		else if(value<30) image = 0x02;  //20
		else if(value<40) image = 0x06;  //30
		else if(value<50) image = 0x04;  //40
		else if(value<60) image = 0x0C;  //50
		else if(value<70) image = 0x08;  //60
		else if(value<80) image = 0x18;  //70
		else if(value<90) image = 0x10;  //80
		else if(value<100) image = 0x30; //90
		else image = 0x20;               //100
	}
	else
	{
		//bar
		if(value<20) image = 0x01;
		else if(value<40) image = 0x03;
		else if(value<60) image = 0x07;
		else if(value<80) image = 0x0F;
		else if(value<100) image = 0x1F;
		else image = 0x3F;
	}
	
	set_image = image;
}

void OUTDRV_SetDisplayPWM(uint8_t pwm)
{
	//truncate value to range
	if(pwm>100) pwm=100;
	
	uint16_t temp = (uint16_t)(((uint32_t)pwm*tim1_top)/100);
	
	OCR1B = temp;
}

/***** DCCD CONTROL FUNCTIONS *****/
void OUTDRV_SetDccdRaw(uint16_t ocval)
{
	if(fuse_dccd)
	{
		OCR1A = 0x0000;
		return;
	};
	
	//truncate value to range
	if(ocval>tim1_maxpwm) ocval=tim1_maxpwm;
	
	OCR1A = ocval;
}

void OUTDRV_SetDccdPWM(uint8_t pwm, uint8_t mode)
{
	//truncate value to range
	if(pwm>100) pwm=100;
	
	//If no calibration done, force mode 0
	if((!coil_resistance)||(!nominal_bat)) mode = 0;
	
	//map pwm value to LOCK PWM range or TIM1 TOP range
	if(mode) coil_set_pwm = (uint16_t)(((uint32_t)pwm*lock_pwm)/100);
	else coil_set_pwm = (uint16_t)(((uint32_t)pwm*tim1_top)/100);
}

void OUTDRV_ProcessCoil(uint16_t ubat)
{
	//Set actual value, and compensate for voltage changes	
	if(fuse_dccd)
	{
		OCR1A = 0x0000;
		return;
	};
	
	//Compensate Battery voltage
	uint16_t temp = 0;
	
	//If nominal battery voltage is set, and actual voltage is known, the do compensation
	if((ubat)&&(nominal_bat))
	{
		uint32_t a = (uint32_t)coil_set_pwm*nominal_bat;
		a = a/((uint32_t)ubat);
		if(a>0x0000FFFF) a = 0x0000FFFF;
		temp = (uint16_t)a;
	}
	else temp=coil_set_pwm;
	
	if(temp>tim1_maxpwm) temp=tim1_maxpwm;
	
	OCR1A = temp;
}

void OUTDRV_FuseDccd(void)
{
	fuse_dccd = 1;
	OCR1A = 0x0000;
}

void OUTDRV_UnfuseDccd(void)
{
	fuse_dccd = 0;
	OCR1A = 0x0000;
}

uint8_t OUTDRV_GetFuseSate(void)
{
	return fuse_dccd;
}

void OUTDRV_SetCoilConfig(uint16_t res, uint16_t nom_bat, uint16_t lock_current)
{
	uint32_t temp = 0;
	
	if((!res)||(!nom_bat)||(!lock_current)) return;
	
	coil_resistance = res;
	nominal_bat = nom_bat;
		
	//Calculate lock voltage
	temp = ((uint32_t)coil_resistance*lock_current)/1000; //target mV, add 5%
	
	//scale to full range if necessary lock voltage is above battery voltage
	if(temp>((uint32_t)nominal_bat)) temp = (uint32_t)nominal_bat; 
	
	//Calculate lock voltage PWM value
	temp = (temp*tim1_top)/((uint32_t)nominal_bat);  //target pwm
	
	lock_pwm = (uint16_t)temp;
}

uint16_t OUTDRV_GetResistance(void)
{
	return coil_resistance;
}

uint8_t OUTDRV_GetDCValue(void)
{
	uint32_t temp = (uint32_t)OCR1A;
	
	temp = (temp*100)/tim1_top;
	return (uint8_t) temp;
}