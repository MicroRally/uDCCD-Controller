/*
uDCCD controller
Coil constant current mode driver

Author: Andis Jargans

Revision history:
v0.0 - YYYY-MM-DD: Initial version
*/

/***** Includes *****/
#include "coil_driver.h"

/***** Private definitions *****/
typedef struct pidStruct{
	uint8_t kp_mul = 0;
	uint8_t kp_div = 0;
	uint8_t ki_mul = 0;
	uint8_t ki_div = 0;
	uint8_t kd_mul = 0;
	uint8_t kd_div = 0;
	int32_t prev_error = 0;
	int32_t integral = 0;
	int32_t integral_min = 0;
	int32_t integral_max = 0;
}pidDef;

typedef struct measStruct{
	uint16_t supply = 0;
	uint16_t currnet = 0;
	uint16_t voltage = 0;
}measDef;

typedef struct warnStruct{
	uint8_t load_loss = 0;
	uint8_t overcurrnet = 0;
	uint8_t uout_high = 0;
	uint8_t uout_low = 0;
	uint8_t supply = 0;
}warnDef;


/***** Private variables *****/
static volatile uint16_t set_voltage = 0;
static volatile uint16_t target_current = 0;

static volatile pidDef pidData;
static volatile measDef measurments;
static volatile warnDef warnings;

/***** Private function declarations *****/

/***** Public function definitions *****/
/**
 * @brief Initializes DCCD hardware driver
 */
void COILDRV_Init(void)
{
	HAL_InitOutputs();
	HAL_EnableDCCDch();
	HAL_SetPWM(HAL_PWM_CH_DCCD,0);
	
	pidData.kp_mul = COIL_PID_KP_MUL;
	pidData.kp_div = COIL_PID_KP_DIV;
	pidData.ki_mul = COIL_PID_KI_MUL;
	pidData.ki_div = COIL_PID_KI_DIV;
	pidData.kd_mul = COIL_PID_KD_MUL;
	pidData.kd_div = COIL_PID_KD_DIV;
	pidData.prev_error = 0;
	pidData.integral = 0;
	pidData.integral_min = 0;
	pidData.integral_max = COIL_MAX_OUT_VOLTAGE;
	
	measurments.supply = 12000;
	measurments.currnet = 0;
	measurments.voltage = 0;
}

/**
 * @brief Set target output current in mA
 */
void COILDRV_SetTarget(uint16_t current)
{
	if(current>COIL_MAX_SET_CURRENT) target_current = COIL_MAX_SET_CURRENT;
	else if(current<COIL_MIN_SET_CURRENT) target_current = COIL_MIN_SET_CURRENT;
	else target_current = current;
}

/**
 * @brief Main coil driver procesing loop
 */
void COILDRV_Process(void)
{
	uint16_t new_output_voltage = 0;
	
	//Protections
	Get_Warnings(target_current,set_voltage,&measurments,&warnings);
	uint8_t fault = Process_Faults(&warnings);
	
	if(fault)
	{
		//Turn off output, wait for fault to end
		HAL_SetPWM(HAL_PWM_CH_DCCD,0);
		PID_Reset(&pidData);
	}
	else
	{
		//Get PID controller output
		int32_t pidout = PID_Controller(target_current,measurments.current,&pidData,warnings.load_loss);
			
		//Do limiting and casting
		if(pidout>COIL_MAX_OUT_VOLTAGE) new_output_voltage = COIL_MAX_OUT_VOLTAGE;
		else if(pidout<COIL_MIN_OUT_VOLTAGE) new_output_voltage = 0;
		else new_output_voltage = (uint16_t) pidout;
		
		//Convert set voltage to PWM value, to get set with current supply voltage
		uint16_t pwm = OutputVoltageToPWM(new_output_voltage,measurments.supply);
		HAL_SetPWM16b(HAL_PWM_CH_DCCD,pwm);
	}

	set_voltage	= new_output_voltage;
}

/**
 * @brief Update output measurment values
 */
void COILDRV_UpdateMeasurments(uint16_t current, uint16_t voltage, uint16_t supply)
{
	measurments.supply = supply;
	measurments.currnet = current;
	measurments.voltage = voltage;
}

/***** Private function definitions *****/

/**
 * @brief Process fault warning states
 */
uint8_t Process_Faults(warnDef* warn )
{
	static uint16_t fault_timer = 0;

	if((warn->overcurrnet)||(warn->uout_high)||(warn->uout_low))
	{
		//Start fault cooldown timer
		fault_timer = COIL_FAULT_COOLDOWN_TIME;
		return 1;
	}
	else if(fault_timer) 
	{
		//Decrement fault cooldown timer
		fault_timer--;
		return 1;
	}
	else
	{
		//No fault
		return 0;
	}
}

/**
 * @brief Process fault warning states
 */
void Get_Warnings(uint16_t set_i, uint16_t set_u, measDef* meas, warnDef* warn )
{
	//Check load loss condition
	if((set_u!=0)&&(meas->currnet==0)) warn->load_loss = 1;
	else warn->load_loss = 0;
	
	//Determine overcurrent treshold
	uint16_t ocp_limit = COIL_MAX_OUT_CURRENT;
	if(set_u!=0) ocp_limit = PercentOfValue_u16(set_i,COIL_OCP_LIMIT_PERCENT);
	
	//Check overcurrent condition
	if((meas->currnet)>ocp_limit) warn->overcurrnet = 1;
	else warn->overcurrnet = 0;
	
	//Determine over/under voltage treshold
	uint16_t ovp_limit = COIL_OUT_OVP_LIMIT_DEFAULT;
	uint16_t uvp_limit = 0;
	
	if(set_u!=0) ovp_limit = PercentOfValue_u16(set_u,COIL_OUT_OVP_LIMIT_PERCENT);
	if(set_u!=0) uvp_limit = PercentOfValue_u16(set_u,COIL_OUT_UVP_LIMIT_PERCENT);
	
	//Check overvolatge condition
	if((meas->voltage)>ovp_limit) warn->uout_high = 1;
	else warn->uout_high = 0;
	
	//Check undervolatge condition
	if(((meas->voltage)<uvp_limit)&&(uvp_limit!=0)) warn->uout_low = 1;
	else warn->uout_low = 0;
	
	//Check supply limits
	if(((meas->supply)<COIL_SUPPLY_MIN)||((meas->supply)>COIL_SUPPLY_MAX)) warn->supply = 1;
	else warn->supply = 0;
}

/**
 * @brief Do PID control
 */
int32_t PID_Controller(uint16_t target, uint16_t feedback, pidDef* pidCtrler, uint8_t zeroInt)
{
	//Calculate error, e(t)
	int32_t error = (int32_t)target - feedback;
	
	//Calculate de/dt, protect against over/underflow
	int32_t de = SaturatedSub_s32(error,pidCtrler->prev_error);	
	pidCtrler->prev_error = error;

	//Calculate e*dt, protect against over/underflow
	pidCtrler->integral = SaturatedAdd_s32(pidCtrler->integral,error);
	
	//Apply artifical integral value limit
	if(zeroInt) pidCtrler->integral = 0;
	else if( pidCtrler->integral > pidCtrler->integral_max ) pidCtrler->integral = pidCtrler->integral_max;
	else if( pidCtrler->integral < pidCtrler->integral_min ) pidCtrler->integral = pidCtrler->integral_min;
	
	//Calculate Kp,Ki,Kd
	int32_t kp = (error*pidCtrler->kp_mul)/pidCtrler->kp_div;
	int32_t ki = (pidCtrler->integral*pidCtrler->ki_mul)/pidCtrler->ki_div;
	int32_t kd = (de*pidCtrler->kd_mul)/pidCtrler->kd_div;
	
	//Calculate output value
	int32_t out = SaturatedAdd_s32(kp,ki);
	out = SaturatedAdd_s32(out,kd);
	
	return out;
}

/**
 * @brief Reset PID controller
 */
void PID_Reset(pidDef* pidCtrler)
{
	pidCtrler->integral = 0;
	pidCtrler->prev_error = 0;
}

/**
 * @brief Convert output-supply ratio to 16bit PWM value
 */
uint16_t OutputVoltageToPWM(uint16_t out, uint16_t supply)
{
	if((supply)&&(out))
	{
		//Compensate for supply voltage changes
		uint32_t temp = ((uint32_t)out*0xFFFF)/supply;
		if(temp>0xFFFF) return 0xFFFF;
		else return (uint16_t)temp;
	}
	else return 0;
}

/**
 * @brief Saturated 32bit add - can't overflow
 */
int32_t SaturatedAdd_s32(int32_t a,int32_t b)
{
	if(b<0)
	{
		int32_t sum = a+b;
		if(sum>a) return 0x80000000;	//Underflow happened
		else return sum;
	}
	else if(b>0)
	{
		int32_t sum = a+b;
		if(sum<a) return 0x7FFFFFFF;	//Overflow happened
		else return sum;
	}
	else return a;
}

/**
 * @brief Saturated 32bit add - can't underflow
 */
int32_t SaturatedSub_s32(int32_t a,int32_t b)
{
	if(b<0)
	{
		int32_t sum = a-b;
		if(sum<a) return 0x7FFFFFFF;	//Overflow happened
		else return sum;
	}
	else if(b>0)
	{
		int32_t sum = a-b;
		if(sum>a) return 0x80000000;	//Overflow happened
		else return sum;
	}
	else return a;
}

/**
 * @brief Get X percent value of given value
 */
uint16_t PercentOfValue_u16(uint16_t value, uint8_t percent)
{
	uint32_t temp = ((uint32_t)value*percent)/100;
	if(temp>0x0000FFFF) return 0xFFFF;
	else return (uint16_t)temp;
}