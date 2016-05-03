/*
 * eps_state.c
 *
 *  Created on: May 2, 2016
 *      Author: Aris Stathakis
 */

#include "eps_state.h"

extern volatile uint8_t adc_reading_complete;//flag to check when dma transfer is complete.
/*update eps state analog measurements*/
static void EPS_update_state_adc_measurements(volatile EPS_State *state, ADC_HandleTypeDef *hadc_eps);
/*get battery pack temperature*/
static int16_t get_batterypack_temperature(I2C_HandleTypeDef *h_i2c, uint8_t sensor_A_i2c_address, uint8_t sensor_B_i2c_address);



void EPS_update_state(volatile EPS_State *state, ADC_HandleTypeDef *hadc_eps, I2C_HandleTypeDef *h_i2c) {

	/*get eps adc measurements*/
	EPS_update_state_adc_measurements(state, hadc_eps);

	/*get eps switch states*/
	//rail
	state->su_p_switch = EPS_get_rail_switch_status(SU);
	state->obc_p_switch = EPS_get_rail_switch_status(OBC);
 	state->adcs_p_switch = EPS_get_rail_switch_status(ADCS);
 	state->comm_p_switch = EPS_get_rail_switch_status(COMM);
 	state->i2c_tc74_p_switch= EPS_get_rail_switch_status(TEMP_SENSOR);

 	state->deploy_left_switch = EPS_get_control_switch_status(DEPLOY_LEFT);
 	state->deploy_right_switch = EPS_get_control_switch_status(DEPLOY_RIGHT);
 	state->deploy_bottom_switch = EPS_get_control_switch_status(DEPLOY_BOTTOM);
 	state->deploy_ant1_switch = EPS_get_control_switch_status(DEPLOY_ANT1);
    state->umbilical_switch  = EPS_get_control_switch_status(UMBILICAL);
    state->heaters_switch = EPS_get_control_switch_status(BATTERY_HEATERS);


 	/*i2c temp sensors*/
    state->battery_voltage = get_batterypack_temperature( h_i2c, TC74_ADDRESS_A, TC74_ADDRESS_B);





}


static void EPS_update_state_adc_measurements(volatile EPS_State *state, ADC_HandleTypeDef *hadc_eps){

	/*initialize adc handle*/
	hadc_eps->NbrOfConversionRank = 6;
	HAL_ADC_Init(hadc_eps);

	/*setup conversion sequence for */
	ADC_ChannelConfTypeDef sConfig;
	sConfig.SamplingTime = ADC_SAMPLETIME_48CYCLES;

	/*Vbat*/
	sConfig.Channel = ADC_VBAT;
	sConfig.Rank = 1;
	HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

	/*Ibat+*/
	sConfig.Channel = ADC_IBAT_PLUS;
	sConfig.Rank = 2;
	HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

	/*Ibat-*/
	sConfig.Channel = ADC_IBAT_MINUS;
	sConfig.Rank = 3;
	HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

	/*I3v3*/
	sConfig.Channel = ADC_I3V3;
	sConfig.Rank = 4;
	HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

	/*I5v*/
	sConfig.Channel = ADC_I5V;
	sConfig.Rank = 5;
	HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

	/*cpu internal temp sensor*/
	sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
	sConfig.Rank = 6;
	HAL_ADC_ConfigChannel(hadc_eps, &sConfig);


	/*Start conversion and dma transfer*/
	uint32_t adc_measurement_dma_eps_state[13]= { 0 };//2*6 +1

	adc_reading_complete = 0;
	HAL_ADC_Start_DMA(hadc_eps, adc_measurement_dma_eps_state, 12);
	/*Wait till DMA ADC sequence transfer is ready*/
	while(adc_reading_complete==0){
 	}
	HAL_ADC_Stop_DMA(hadc_eps);


	state->battery_voltage = adc_measurement_dma_eps_state[6];
	state->battery_current_plus = adc_measurement_dma_eps_state[7];
	state->battery_current_minus = adc_measurement_dma_eps_state[8];
	state->v3_3_current_avg = adc_measurement_dma_eps_state[9];
	state->v5_current_avg = adc_measurement_dma_eps_state[10];
	state->cpu_temperature = adc_measurement_dma_eps_state[11];

}



int16_t get_batterypack_temperature(I2C_HandleTypeDef *h_i2c, uint8_t sensor_A_i2c_address, uint8_t sensor_B_i2c_address){

	 TC_74_STATUS statusA, statusB;
	 //wake up sensor1
	 statusA = device_wake_up( h_i2c, sensor_A_i2c_address);
	 //wake up sensor2
	 statusB = device_wake_up( h_i2c, sensor_B_i2c_address);

	 //check sensor1 status if still in standby try 3 times to wake up if not generate temp1 fault
	 statusA = read_device_status(h_i2c, sensor_A_i2c_address);

	 //check sensor2 status if still in standby try 3 times to wake up if not generate temp2 fault
	 statusB = read_device_status(h_i2c, sensor_B_i2c_address);


	 //get temperatue1
	 int8_t temperature_measurementA;
	 statusA= read_device_temperature(h_i2c, sensor_A_i2c_address, &temperature_measurementA);
	 //get temperatue2
	 int8_t temperature_measurementB;
	 statusB= read_device_temperature(h_i2c, sensor_B_i2c_address, &temperature_measurementB);
	 //get temperatue1
	 //get temperatue2
	 //sleep sensor1
	 statusA = device_sleep( h_i2c, sensor_A_i2c_address);
	 //sleep sensor2
	 statusB= device_sleep( h_i2c, sensor_B_i2c_address);
	 //calculate average of all measurements
	 return (temperature_measurementA + temperature_measurementB)>>1;

}



void EPS_state_init(volatile EPS_State *state){



	state->su_p_switch = EPS_SWITCH_RAIL_OFF;
	state->adcs_p_switch = EPS_SWITCH_RAIL_OFF;
	state->comm_p_switch = EPS_SWITCH_RAIL_OFF;
	state->obc_p_switch = EPS_SWITCH_RAIL_OFF;
	state->i2c_tc74_p_switch = EPS_SWITCH_RAIL_OFF;

	state->deploy_left_switch = EPS_SWITCH_CONTROL_OFF;
	state->deploy_right_switch = EPS_SWITCH_CONTROL_OFF;
	state->deploy_bottom_switch = EPS_SWITCH_CONTROL_OFF;
	state->deploy_ant1_switch = EPS_SWITCH_CONTROL_OFF;
	state->umbilical_switch = EPS_SWITCH_CONTROL_OFF;
	state->heaters_switch = EPS_SWITCH_CONTROL_OFF;

	state->v5_current_avg = 0x00;
	state->v3_3_current_avg = 0x00;
	state->battery_voltage = 0x00;
	state->battery_current_plus = 0x00;
	state->battery_current_minus = 0x00;
	state->battery_temp = 0x00;

	state->module_left_voltage_avg = 0x00;
	state->module_left_current_avg = 0x00;
	state->module_left_power_avg = 0x00;
	state->module_right_voltage_avg = 0x00;
	state->module_right_current_avg = 0x00;
	state->module_right_power_avg = 0x00;
	state->module_top_voltage_avg = 0x00;
	state->module_top_current_avg = 0x00;
	state->module_top_power_avg = 0x00;
	state->module_bottom_voltage_avg = 0x00;
	state->module_bottom_current_avg = 0x00;
	state->module_bottom_power_avg = 0x00;



}

void EPS_set_rail_switch(EPS_switch_rail eps_switch, EPS_switch_rail_status switch_status, EPS_State *state){

 	GPIO_PinState gpio_write_value;

	if(switch_status == EPS_SWITCH_RAIL_ON){
		gpio_write_value = GPIO_PIN_RESET;
 	}
	else{
		gpio_write_value = GPIO_PIN_SET;
 	}


	switch (eps_switch) {

	case SU:
		HAL_GPIO_WritePin(GPIO_SU_SWITCH_GPIO_Port, GPIO_SU_SWITCH_Pin, gpio_write_value);
		state->su_p_switch = switch_status;
		break;

	case OBC:
		HAL_GPIO_WritePin(GPIO_OBC_SWITCH_GPIO_Port, GPIO_OBC_SWITCH_Pin, gpio_write_value);
		state->obc_p_switch = switch_status;
		break;

	case ADCS:
		HAL_GPIO_WritePin(GPIO_ADCS_SWITCH_GPIO_Port, GPIO_ADCS_SWITCH_Pin, gpio_write_value);
		state->adcs_p_switch = switch_status;
		break;

	case COMM:
		HAL_GPIO_WritePin(GPIO_COMM_SWITCH_GPIO_Port, GPIO_COMM_SWITCH_Pin, gpio_write_value);
		state->comm_p_switch = switch_status;
		break;

	case TEMP_SENSOR:
		HAL_GPIO_WritePin(GPIO_TC74_POWER_GPIO_Port, GPIO_TC74_POWER_Pin, gpio_write_value);
		state->i2c_tc74_p_switch = switch_status;
		break;

// 	   default :
// 		   //YOU SHOULDNT BE HERE!
// 		   //ERROR HANDLING
	}


}




void EPS_set_control_switch(EPS_switch_control eps_switch, EPS_switch_control_status switch_status, EPS_State *state){

 	GPIO_PinState gpio_write_value;

	if(switch_status == EPS_SWITCH_CONTROL_ON){
		gpio_write_value = GPIO_PIN_RESET;
 	}
	else{
		gpio_write_value = GPIO_PIN_SET;
 	}


	switch (eps_switch) {


	case DEPLOY_LEFT:
		HAL_GPIO_WritePin(GPIO_DEPLOY_LEFT_GPIO_Port, GPIO_DEPLOY_LEFT_Pin, gpio_write_value);
		state->deploy_left_switch = switch_status;
		break;

	case DEPLOY_RIGHT:
		HAL_GPIO_WritePin(GPIO_DEPLOY_RIGHT_GPIO_Port, GPIO_DEPLOY_RIGHT_Pin, gpio_write_value);
		state->deploy_right_switch = switch_status;
		break;

	case DEPLOY_BOTTOM:
		HAL_GPIO_WritePin(GPIO_DEPLOY_BOTTOM_GPIO_Port, GPIO_DEPLOY_BOTTOM_Pin, gpio_write_value);
		state->deploy_bottom_switch = switch_status;
		break;

	case DEPLOY_ANT1:
		HAL_GPIO_WritePin(GPIO_DEPLOY_ANT1_GPIO_Port, GPIO_DEPLOY_ANT1_Pin, gpio_write_value);
		state->deploy_ant1_switch = switch_status;
		break;

	case BATTERY_HEATERS:
		HAL_GPIO_WritePin(GPIO_HEATERS_GPIO_Port, GPIO_HEATERS_Pin, gpio_write_value);
		state->heaters_switch = switch_status;
		break;
	case UMBILICAL:
		HAL_GPIO_WritePin(GPIO_UMBILICAL_GPIO_Port, GPIO_UMBILICAL_Pin, gpio_write_value);
		state->umbilical_switch = switch_status;
		break;

// 	   default :
// 		   //YOU SHOULDNT BE HERE!
// 		   //ERROR HANDLING
	}


}



EPS_switch_rail_status EPS_get_rail_switch_status(EPS_switch_rail eps_switch) {

	EPS_switch_rail_status return_status;
	GPIO_PinState gpio_read_value;

	switch (eps_switch) {

	case SU:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_SU_SWITCH_GPIO_Port, GPIO_SU_SWITCH_Pin);
		break;

	case OBC:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_OBC_SWITCH_GPIO_Port, GPIO_OBC_SWITCH_Pin);
		break;

	case ADCS:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_ADCS_SWITCH_GPIO_Port, GPIO_ADCS_SWITCH_Pin);
		break;

	case COMM:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_COMM_SWITCH_GPIO_Port, GPIO_COMM_SWITCH_Pin);
		break;

	case TEMP_SENSOR:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_TC74_POWER_GPIO_Port, GPIO_TC74_POWER_Pin);
		break;

// 	   default :
// 		   //YOU SHOULDNT BE HERE!
// 		   //ERROR HANDLING
	}

	if(gpio_read_value == GPIO_PIN_RESET){
		return_status = EPS_SWITCH_RAIL_ON;
	}
	else{
		return_status =  EPS_SWITCH_RAIL_OFF;
	}

	return return_status;

}

EPS_switch_control_status EPS_get_control_switch_status(EPS_switch_control eps_switch) {

	EPS_switch_control_status return_status;
	GPIO_PinState gpio_read_value;

	switch (eps_switch) {

	case DEPLOY_LEFT:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_DEPLOY_LEFT_GPIO_Port, GPIO_DEPLOY_LEFT_Pin);
		break;

	case DEPLOY_RIGHT:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_DEPLOY_RIGHT_GPIO_Port, GPIO_DEPLOY_RIGHT_Pin);
		break;

	case DEPLOY_BOTTOM:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_DEPLOY_BOTTOM_GPIO_Port, GPIO_DEPLOY_BOTTOM_Pin);
		break;

	case DEPLOY_ANT1:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_DEPLOY_ANT1_GPIO_Port, GPIO_DEPLOY_ANT1_Pin);
		break;

	case BATTERY_HEATERS:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_HEATERS_GPIO_Port, GPIO_HEATERS_Pin);
		break;
	case UMBILICAL:
		gpio_read_value =HAL_GPIO_ReadPin(GPIO_UMBILICAL_GPIO_Port, GPIO_UMBILICAL_Pin);
		break;

// 	   default :
// 		   //YOU SHOULDNT BE HERE!
// 		   //ERROR HANDLING
	}

	if(gpio_read_value == GPIO_PIN_RESET){
		return_status = EPS_SWITCH_CONTROL_OFF;
	}
	else{
		return_status =  EPS_SWITCH_CONTROL_ON;
	}

	return return_status;

}