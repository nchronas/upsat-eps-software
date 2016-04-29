#ifndef __HAL_EPS_H
#define __HAL_EPS_H

#include "stm32l1xx_hal.h"

#define TIMED_EVENT_PERIOD ((uint8_t)50)//in miliseconds
#define MPPT_STEP_SIZE ((uint32_t)1)
#define MPPT_STARTUP_PWM_DUTYCYCLE ((uint32_t) 70)

/*Set up power modules pwm timer channels*/
#define PWM_TIM_CHANNEL_TOP TIM_CHANNEL_3
#define PWM_TIM_CHANNEL_BOTTOM TIM_CHANNEL_1
#define PWM_TIM_CHANNEL_LEFT TIM_CHANNEL_2
#define PWM_TIM_CHANNEL_RIGHT TIM_CHANNEL_4
/*Set up power modules adc current channels*/
#define ADC_CURRENT_CHANNEL_TOP ADC_CHANNEL_5
#define ADC_CURRENT_CHANNEL_BOTTOM ADC_CHANNEL_20
#define ADC_CURRENT_CHANNEL_LEFT ADC_CHANNEL_11
#define ADC_CURRENT_CHANNEL_RIGHT ADC_CHANNEL_18
/*Set up power modules adc voltage channels*/
#define ADC_VOLTAGE_CHANNEL_TOP ADC_CHANNEL_6
#define ADC_VOLTAGE_CHANNEL_BOTTOM ADC_CHANNEL_21
#define ADC_VOLTAGE_CHANNEL_LEFT ADC_CHANNEL_10
#define ADC_VOLTAGE_CHANNEL_RIGHT ADC_CHANNEL_19

/*Setup eps state adc channels*/
#define ADC_VBAT ADC_CHANNEL_1;
#define ADC_IBAT_PLUS ADC_CHANNEL_2;
#define ADC_IBAT_MINUS ADC_CHANNEL_3;
#define ADC_I3V3 ADC_CHANNEL_4;
#define ADC_I5V ADC_CHANNEL_12;

extern volatile uint8_t adc_reading_complete;//flag to check when dma transfer is complete.
extern volatile uint8_t EPS_soft_error_status;//initialize global software error status to false.
extern volatile uint8_t EPS_event_period_status;//initialize global timed event flag to true.


typedef struct {
	uint8_t su_p_switch;/*Science unit control switch - set to turn off - reset to turn on (!inverted logic!)*/
	uint8_t obc_p_switch;
	uint8_t adcs_p_switch;
	uint8_t comm_p_switch;
	uint8_t i2c_tc74_p_switch;
	/**/
	uint8_t deploy_left_switch;
	uint8_t deploy_right_switch;
	uint8_t deploy_bottom_switch;
	uint8_t deploy_ant1_switch;
	uint8_t umbilical_switch;
	uint8_t heaters_switch;
    /**/
	uint16_t module_left_voltage_avg;
	uint16_t module_left_current_avg;
	uint32_t module_left_power_avg;
    /**/
	uint16_t module_right_voltage_avg;
	uint16_t module_right_current_avg;
	uint32_t module_right_power_avg;
	    /**/
	uint16_t module_top_voltage_avg;
	uint16_t module_top_current_avg;
	uint32_t module_top_power_avg;
	    /**/
	uint16_t module_bottom_voltage_avg;
	uint16_t module_bottom_current_avg;
	uint32_t module_bottom_power_avg;
	/**/
	uint16_t v5_current_avg;
	uint16_t v3_3_current_avg;
	uint16_t battery_voltage;
	uint16_t battery_current_plus;
	uint16_t battery_current_minus;
	int8_t 	 battery_temp;
	uint32_t cpu_temperature;
}EPS_State;


typedef struct {
	uint16_t voltage; /*average voltage at each mppt step*/
	uint16_t current; /*average curret at each mppt step*/
	uint32_t previous_power; /*average power at previous mppt step*/
	uint8_t incremennt_flag;/*flag for mppt algorithm must be initialized to 1*/
	uint32_t pwm_duty_cycle; /*duty cycle of power module pwm output*/
	TIM_HandleTypeDef *htim_pwm;/*assign wich timer is assigned for this pwm output*/
	uint32_t timChannel;/*assign the proper timer channel assigned to module pwm output*/
	ADC_HandleTypeDef *hadc_power_module;/*adc handle for voltage and current measurements for each power module*/

}EPS_PowerModule;


void EPS_PowerModule_init(EPS_PowerModule *module_X, uint32_t starting_pwm_dutycycle, TIM_HandleTypeDef *htim, uint32_t timer_channel, ADC_HandleTypeDef *hadc_power_module, uint32_t ADC_channel_current, uint32_t ADC_channel_voltage);
void EPS_update_power_module_state(EPS_PowerModule *power_module);
void EPS_PowerModule_mppt_update_pwm(EPS_PowerModule *moduleX);
void EPS_PowerModule_mppt_apply_pwm(EPS_PowerModule *moduleX);

void EPS_state_init(volatile EPS_State *state, ADC_HandleTypeDef *hadc_eps);
void EPS_update_state(volatile EPS_State *state, ADC_HandleTypeDef *hadc_eps);

void ADC_EPS_POWER_MODULE_Init(ADC_HandleTypeDef *hadc_power_module, uint32_t ADC_channel_current, uint32_t ADC_channel_voltage);
void ADC_EPS_STATE_Init(ADC_HandleTypeDef *hadc_eps);




#endif
