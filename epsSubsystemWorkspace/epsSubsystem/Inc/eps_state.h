/*
 * eps_state.h
 *
 *  Created on: May 2, 2016
 *      Author: Aris Stathakis
 */

#ifndef INC_EPS_STATE_H_
#define INC_EPS_STATE_H_

#include "stm32l1xx_hal.h"
#include "eps_configuration.h"
//#include "tc74_temp_sensor.h"
#include "eps_soft_error_handling.h"
//#include "eps_safety.h"

typedef enum {
	SU =0,
	OBC,
	ADCS,
	COMM,
	TEMP_SENSOR,
	RAIL_LAST_VALUE
}EPS_switch_rail;

typedef enum {
	EPS_SWITCH_RAIL_ON,
	EPS_SWITCH_RAIL_OFF,
	EPS_SWITCH_RAIL_LAST_VALUE
}EPS_switch_rail_status;

typedef enum {
	DEPLOY_LEFT =0,
	DEPLOY_RIGHT,
	DEPLOY_BOTTOM,
	DEPLOY_ANT1,
	BATTERY_HEATERS,
	DEPLOY_TOP,
	CONTROL_LAST_VALUE
}EPS_switch_control;

typedef enum {
	EPS_SWITCH_CONTROL_OFF,
	EPS_SWITCH_CONTROL_ON,
	EPS_SWITCH_CONTROL_LAST_VALUE
}EPS_switch_control_status;


typedef enum {
	EPS_BATTERY_SENSOR_SYSTEM_OK,
	EPS_BATTERY_SENSOR_A_DEAD,
	EPS_BATTERY_SENSOR_B_DEAD,
	EPS_BATTERY_SENSOR_CPU_TEMP_ONLY,
	EPS_BATTERY_SENSOR_LAST_VALUE
}EPS_battery_tempsense_health;


typedef enum {
	EPS_NOMINAL_MODE=0,
	EPS_CRITICAL_MODE,
	EPS_ALMOST_DEAD_MODE,
	EPS_DEAD_MODE,
	EPS_MODE_LAST_VALUE
}EPS_mode_status;


typedef enum {
	EPS_SAFETY_MODE_BATTERY_NORMAL=0,
	EPS_SAFETY_MODE_BATTERY_CRITICAL,
	EPS_SAFETY_MODE_BATTERY_DEAD,
	EPS_SAFETY_MODE_BATTERY_OVERVOLTAGE,
	EPS_SAFETY_MODE_BATTERY_NOT_SET,
	EPS_SAFETY_MODE_BATTERY_LAST_VALUE
}EPS_safety_battery_status;

typedef enum {
	EPS_SAFETY_MODE_TEMPERATURE_NORMAL=0,
	EPS_SAFETY_MODE_TEMPERATURE_HIGH,
	EPS_SAFETY_MODE_TEMPERATURE_LOW,
	EPS_SAFETY_MODE_TEMPERATURE_NOT_SET,
	EPS_SAFETY_MODE_TEMPERATURE_LAST_VALUE
}EPS_safety_temperature_status;




typedef struct {
	EPS_mode_status eps_functional_mode;
	EPS_safety_battery_status EPS_safety_battery_mode;
	EPS_safety_temperature_status EPS_safety_temperature_mode;
	/**/
	EPS_switch_rail_status su_p_switch;/*Science unit control switch - set to turn off - reset to turn on (!inverted logic!)*/
	EPS_switch_rail_status obc_p_switch;
	EPS_switch_rail_status adcs_p_switch;
	EPS_switch_rail_status comm_p_switch;
	EPS_switch_rail_status i2c_tc74_p_switch;
	/**/
	EPS_switch_control_status deploy_left_switch;
	EPS_switch_control_status deploy_right_switch;
	EPS_switch_control_status deploy_bottom_switch;
	EPS_switch_control_status deploy_top_switch;
	EPS_switch_control_status deploy_ant1_switch;
	EPS_switch_control_status heaters_switch;
	/**/
	uint8_t umbilical_switch;//gpio input with inverse logic.
	/**/
	uint16_t v5_current_avg;
	uint16_t v3_3_current_avg;
	uint16_t battery_voltage;
	uint16_t battery_current_plus; /* current charging the battery */
	uint16_t battery_current_minus;/* current from battery to the load */
	int16_t  battery_temp;
	int32_t cpu_temperature;
	/**/
	EPS_battery_tempsense_health batterypack_health_status;
}EPS_State;


typedef enum {
	ADC_TRANSFER_NOT_COMPLETED=0,
	ADC_TRANSFER_COMPLETED,
	ADC_TRANSFER_LAST_VALUE
}EPS_adc_dma_transfer_status;

extern EPS_adc_dma_transfer_status adc_reading_complete;

EPS_soft_error_status EPS_state_init(volatile EPS_State *state);
EPS_soft_error_status EPS_update_state(volatile EPS_State *state, ADC_HandleTypeDef *hadc_eps, I2C_HandleTypeDef *h_i2c);

EPS_switch_rail_status EPS_get_rail_switch_status(EPS_switch_rail eps_switch);
void EPS_set_rail_switch(EPS_switch_rail eps_switch, EPS_switch_rail_status switch_status,volatile EPS_State *state);

EPS_switch_control_status EPS_get_control_switch_status(EPS_switch_control eps_switch);
void EPS_set_control_switch(EPS_switch_control eps_switch, EPS_switch_control_status switch_status,volatile EPS_State *state);

#endif /* INC_EPS_STATE_H_ */
