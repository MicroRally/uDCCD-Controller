/*
uDCCD controller
LEDs driver

Author: Andis Jargans

Revision history:
2021-09-17: Initial version
*/

#ifndef DCCD_HW_DRIVER
#define DCCD_HW_DRIVER

/**** Includes ****/

/**** Public definitions ****/

/**** Aplication specific configuration ****/
#define LOCK_CURRENT	4500
#define MAX_OUTPUT_VOLTAGE	10000

#define COIL_LAG_TIME	10

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

/**** Public function declarations ****/
//Control functions

//Interrupt and loop functions

//Data retrieve functions

#endif