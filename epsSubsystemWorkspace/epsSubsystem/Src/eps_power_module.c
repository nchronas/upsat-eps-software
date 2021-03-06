/*
 * eps_power_module.c
 *
 *  Created on: May 2, 2016
 *      Author: Aris Stathakis
 */

#include "eps_power_module.h"
#include "eps_state.h"


extern ADC_HandleTypeDef hadc;
extern TIM_HandleTypeDef htim3;


void EPS_PowerModule_init(EPS_PowerModule *module_X, uint32_t starting_pwm_dutycycle, TIM_HandleTypeDef *htim, uint32_t timer_channel, ADC_HandleTypeDef *hadc_power_module, uint32_t ADC_channel_current, uint32_t ADC_channel_voltage){

	//initialize properly all power module entries.
	module_X->module_state = POWER_MODULE_ON;
	module_X->current =0;
	module_X->voltage =0;
	module_X->previous_power =0;
	module_X->previous_voltage =0;
	module_X->pwm_duty_cycle = starting_pwm_dutycycle;
	module_X->htim_pwm = htim;
	module_X->timChannel = timer_channel;
	module_X->incremennt_flag = 1;//start by incrementing
	module_X->hadc_power_module = hadc_power_module;
	module_X->ADC_channel_current = ADC_channel_current;
	module_X->ADC_channel_voltage = ADC_channel_voltage;

	//Start pwm with initialized from cube mx pwm duty cycle for timerX at timer_channel.
	HAL_TIM_PWM_Start(htim, timer_channel);

}


EPS_soft_error_status EPS_PowerModule_init_ALL(EPS_PowerModule *module_top, EPS_PowerModule *module_bottom, EPS_PowerModule *module_left, EPS_PowerModule *module_right){

	EPS_soft_error_status power_module_init_status = EPS_SOFT_ERROR_POWER_MODULE_INIT_ALL;

	/*start timer3 pwm base generation (initialized pwm duty cycle from mx must be 0) */
	HAL_TIM_Base_Start(&htim3);

	/* initialize all power modules and dem mppt cotrol blocks.*/
	power_module_init_status = EPS_SOFT_ERROR_POWER_MODULE_INIT_TOP;
	EPS_PowerModule_init(module_top, MPPT_STARTUP_PWM_DUTYCYCLE, &htim3, PWM_TIM_CHANNEL_TOP, &hadc, ADC_CURRENT_CHANNEL_TOP, ADC_VOLTAGE_CHANNEL_TOP);
	power_module_init_status = EPS_SOFT_ERROR_POWER_MODULE_INIT_BOTTOM;
	EPS_PowerModule_init(module_bottom, MPPT_STARTUP_PWM_DUTYCYCLE, &htim3, PWM_TIM_CHANNEL_BOTTOM, &hadc, ADC_CURRENT_CHANNEL_BOTTOM, ADC_VOLTAGE_CHANNEL_BOTTOM);
	power_module_init_status = EPS_SOFT_ERROR_POWER_MODULE_INIT_LEFT;
	EPS_PowerModule_init(module_left, MPPT_STARTUP_PWM_DUTYCYCLE, &htim3, PWM_TIM_CHANNEL_LEFT, &hadc, ADC_CURRENT_CHANNEL_LEFT, ADC_VOLTAGE_CHANNEL_LEFT);
	power_module_init_status = EPS_SOFT_ERROR_POWER_MODULE_INIT_RIGHT;
	EPS_PowerModule_init(module_right, MPPT_STARTUP_PWM_DUTYCYCLE, &htim3, PWM_TIM_CHANNEL_RIGHT, &hadc, ADC_CURRENT_CHANNEL_RIGHT, ADC_VOLTAGE_CHANNEL_RIGHT);

	power_module_init_status = EPS_SOFT_ERROR_OK;
	return power_module_init_status;
}


void EPS_update_power_module_state(EPS_PowerModule *power_module){



    //setup adc handle sequence and initialize adc peripheral

	/*initialize adc handle*/

	///////////////////////////////
//	power_module->hadc_power_module->Instance = ADC1;
//	power_module->hadc_power_module->Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
//	power_module->hadc_power_module->Init.Resolution = ADC_RESOLUTION_12B;
//	power_module->hadc_power_module->Init.DataAlign = ADC_DATAALIGN_RIGHT;
//	power_module->hadc_power_module->Init.ScanConvMode = ADC_SCAN_ENABLE;
//	//power_module->hadc_power_module->Init.EOCSelection = ADC_EOC_SEQ_CONV;
//	power_module->hadc_power_module->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
//	power_module->hadc_power_module->Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
//	power_module->hadc_power_module->Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
//	power_module->hadc_power_module->Init.ChannelsBank = ADC_CHANNELS_BANK_A;
//	power_module->hadc_power_module->Init.ContinuousConvMode = ENABLE;
////	power_module->hadc_power_module->Init.NbrOfConversion = 2;
//	power_module->hadc_power_module->Init.DiscontinuousConvMode = DISABLE;
//	power_module->hadc_power_module->Init.ExternalTrigConv = ADC_SOFTWARE_START;
//	power_module->hadc_power_module->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
//	power_module->hadc_power_module->Init.DMAContinuousRequests = ENABLE;


	//////////////////////////////


	power_module->hadc_power_module->Init.NbrOfConversion = 2;
	//power_module->hadc_power_module->NbrOfConversionRank = 2;
	HAL_ADC_Init(power_module->hadc_power_module);

	/*setup conversion sequence for */
	ADC_ChannelConfTypeDef sConfig;
	sConfig.SamplingTime = ADC_SAMPLETIME_192CYCLES;

	/*power module current*/
	sConfig.Channel = power_module->ADC_channel_current ;
	sConfig.Rank = 1;
	HAL_ADC_ConfigChannel(power_module->hadc_power_module, &sConfig);

	/*power module voltage*/
	sConfig.Channel = power_module->ADC_channel_voltage ;
	sConfig.Rank = 2;
	HAL_ADC_ConfigChannel(power_module->hadc_power_module, &sConfig);



	//start dma transfer from adc to memory.

	uint32_t adc_measurement_dma_power_modules[67]= { 0 };//2*64 +1
//////////////////////////////////////////////////////////////////////////
//	for (int zero_index = 0; zero_index < 67; zero_index+=1) {
//
//		adc_measurement_dma_power_modules[zero_index] =0;
//
//	}
//////////////////////////////////////////////////////////////////////////


	adc_reading_complete = ADC_TRANSFER_NOT_COMPLETED;//external global flag defined in main and shared with the adc complete transfer interrupt handler.
	HAL_ADC_Start_DMA(power_module->hadc_power_module, adc_measurement_dma_power_modules, 66);

	/*Process Measurements*/
	uint32_t voltage_avg =0;
	uint32_t current_avg =0;

	/*Wait till DMA ADC sequence transfer is ready*/
	while(adc_reading_complete==ADC_TRANSFER_NOT_COMPLETED){
		//wait for dma transfer complete.
	}
	//ADC must be stopped in the adc dma transfer complete callback.

	HAL_ADC_Stop_DMA(power_module->hadc_power_module);
	//HAL_ADC_Stop(power_module->hadc_power_module);

	////////////////////////////////////////////////////////////////
	//HAL_ADC_DeInit(power_module->hadc_power_module);
	////////////////////////////////////////////////////////////////


	//de-interleave and sum voltage and current measurements. TODO:overflow strategy??? : 2^12(max adc value) * 32 = 2^17 < 2^32 so you do not need one!
	for (int sum_index = 2; sum_index < 66; sum_index+=2) {
		/*top*/
		current_avg = current_avg + adc_measurement_dma_power_modules[sum_index];
		voltage_avg = voltage_avg + adc_measurement_dma_power_modules[sum_index+1];
	}

	/*filter ting*/
	//average of 16 concecutive adc measurements.skip the first to avoid adc power up distortion.
	power_module->voltage = voltage_avg>>5;
	power_module->current = current_avg>>5;


}


void EPS_PowerModule_mppt_update_pwm(EPS_PowerModule *moduleX){



	/*power calculation*/

	volatile uint32_t power_now_avg = moduleX->voltage * moduleX->current;
	uint32_t duty_cycle = moduleX->pwm_duty_cycle;


	/*if solar cell voltage is below threshold, reset mppt point search starting from startup dutycycle*/
	if(moduleX->voltage<MPPT_VOLTAGE_THRESHOLD){

		duty_cycle = MPPT_STARTUP_PWM_DUTYCYCLE;

	}
	else{

		uint32_t step_size = MPPT_STEP_SIZE;

		//	  if (moduleX->current<1000){
		//		  step_size = MPPT_STEP_SIZE + 5;
		//		  //moduleX->incremennt_flag = 1;
		//	  }


		// decide duty cycle orientation - set increment flag.
		if (power_now_avg  <(moduleX->previous_power)){

			if (moduleX->incremennt_flag>0){

				if(moduleX->voltage  <(moduleX->previous_voltage -5)){
					moduleX->incremennt_flag = 0;
				}

			}
			else{
				moduleX->incremennt_flag = 1;
			}
		}
		//add appropriate offset - create new duty cycle.

		if(moduleX->incremennt_flag){
			duty_cycle = duty_cycle+step_size;
		}
		else{
			duty_cycle = duty_cycle-step_size;
		}
		//Check for Overflow and Underflow
		if (duty_cycle>(160+MPPT_STEP_SIZE)){//first check for underflow
			duty_cycle= MPPT_STARTUP_PWM_DUTYCYCLE;//
		}
		if (duty_cycle==(160+MPPT_STEP_SIZE)){//then check for overflow
			duty_cycle=160;
		}
		// Set new PWM compare register
		//duty_cycle =0;

	}

	  moduleX->previous_power = power_now_avg;
	  moduleX->previous_voltage = moduleX->voltage;
	  moduleX->pwm_duty_cycle = duty_cycle;

}


void EPS_PowerModule_mppt_apply_pwm(EPS_PowerModule *moduleX){

	uint32_t pwm_output;
	if (moduleX->module_state ==POWER_MODULE_OFF){
		pwm_output = 0;
	}
	else if(moduleX->module_state ==POWER_MODULE_ON){
		pwm_output = moduleX->pwm_duty_cycle;
	}
	else{
		pwm_output = 0;
	}

	switch ( moduleX->timChannel ) {
	case TIM_CHANNEL_1:         /*  top module */
		moduleX->htim_pwm->Instance->CCR1 = pwm_output;
		break;
	case TIM_CHANNEL_2:         /*  bottom module */
		moduleX->htim_pwm->Instance->CCR2 = pwm_output;
		break;
	case TIM_CHANNEL_3:         /*  left module */
		moduleX->htim_pwm->Instance->CCR3 = pwm_output;
		break;
	case TIM_CHANNEL_4:         /*  right module */
		moduleX->htim_pwm->Instance->CCR4 = pwm_output;
		break;
	default:
		//error handling?
		break;
	}

}

