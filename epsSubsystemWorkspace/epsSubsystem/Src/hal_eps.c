
#include "hal_eps.h"


volatile uint8_t adc_reading_complete = 0;//flag to check when dma transfer is complete.
volatile uint8_t EPS_soft_error_status = 0x00;//initialize global software error status to false.
volatile uint8_t EPS_event_period_status = 0xFF;//initialize global timed event flag to true.



void EPS_update_state(volatile EPS_State *state, ADC_HandleTypeDef *hadc_eps){


	uint32_t adc_measurement_dma_eps_state[13]= { 0 };//2*6 +1

	adc_reading_complete = 0;
	HAL_ADC_Start_DMA(hadc_eps, adc_measurement_dma_eps_state, 12);
	/*Wait till DMA ADC sequence transfer is ready*/
	while(adc_reading_complete==0){
 	}

//	HAL_ADC_Stop_DMA(&hadc);

	state->battery_voltage = adc_measurement_dma_eps_state[6];
	state->battery_current_plus = adc_measurement_dma_eps_state[7];
	state->battery_current_minus = adc_measurement_dma_eps_state[8];
	state->v3_3_current_avg = adc_measurement_dma_eps_state[9];
	state->v5_current_avg = adc_measurement_dma_eps_state[10];
	state->cpu_temperature = adc_measurement_dma_eps_state[11];

	/*i2c temp sensors*/
	//	  state->battery_temp;


}



//void EPS_update_power_modules_state(EPS_PowerModule *module_top, EPS_PowerModule *module_bottom, EPS_PowerModule *module_left, EPS_PowerModule *module_right){
//
//	/*adc ting*/
//	ADC_EPS_POWER_MODULES_Init();
//	adc_reading_complete = 0;
//	HAL_ADC_Start_DMA(&hadc, adc_measurement_dma_power_modules, 136);
//
//	/*Process Measurements*/
//	uint32_t voltage_avg_top =0;
//	uint32_t current_avg_top =0;
//	uint32_t voltage_avg_bottom =0;
//	uint32_t current_avg_bottom =0;
//	uint32_t voltage_avg_left =0;
//	uint32_t current_avg_left =0;
//	uint32_t voltage_avg_right =0;
//	uint32_t current_avg_right =0;
//
//	/*Wait till DMA ADC sequence transfer is ready*/
//	while(adc_reading_complete==0){
//		//wait for dma transfer complete.
//	}
////	HAL_ADC_Stop_DMA(&hadc);//stop transfer and turn off adc peripheral.
//
//	//de-interleave and sum voltage and current measurements.
//	for (int sum_index = 8; sum_index < 136; sum_index+=8) {
//		/*top*/
//		current_avg_top = current_avg_top + adc_measurement_dma_power_modules[sum_index];
//		voltage_avg_top = voltage_avg_top + adc_measurement_dma_power_modules[sum_index+1];
//		/*bottom*/
//		current_avg_bottom = current_avg_bottom + adc_measurement_dma_power_modules[sum_index+2];
//		voltage_avg_bottom = voltage_avg_bottom + adc_measurement_dma_power_modules[sum_index+3];
//		/*left*/
//		current_avg_left = current_avg_left + adc_measurement_dma_power_modules[sum_index+4];
//		voltage_avg_left = voltage_avg_left + adc_measurement_dma_power_modules[sum_index+5];
//		/*right*/
//		current_avg_right = current_avg_right + adc_measurement_dma_power_modules[sum_index+6];
//		voltage_avg_right = voltage_avg_right + adc_measurement_dma_power_modules[sum_index+7];
//	}
//
//	/*filter ting*/
//	//average of 16 concecutive adc measurements.skip the first to avoid adc power up distortion.
//	module_top->voltage = voltage_avg_top>>4;
//	module_top->current = current_avg_top>>4;
//	module_bottom->voltage = voltage_avg_bottom>>4;
//	module_bottom->current = current_avg_bottom>>4;
//	module_left->voltage = voltage_avg_left>>4;
//	module_left->current = current_avg_left>>4;
//	module_right->voltage = voltage_avg_right>>4;
//	module_right->current = current_avg_right>>4;
//
//
//
//
//
//}




void EPS_state_init(volatile EPS_State *state, ADC_HandleTypeDef *hadc_eps){



	/*adc ting*/
	ADC_EPS_STATE_Init(hadc_eps);



	state->su_p_switch = 0xff;
	state->adcs_p_switch = 0xff;
	state->comm_p_switch = 0xff;
	state->obc_p_switch = 0xff;
	state->i2c_tc74_p_switch = 0xff;

	state->deploy_left_switch = 0x00;
	state->deploy_right_switch = 0x00;
	state->deploy_bottom_switch = 0x00;
	state->deploy_ant1_switch = 0x00;
	state->umbilical_switch = 0x00;
	state->heaters_switch = 0x00;
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
	state->v5_current_avg = 0x00;
	state->v3_3_current_avg = 0x00;
	state->battery_voltage = 0x00;
	state->battery_current_plus = 0x00;
	state->battery_current_minus = 0x00;
	state->battery_temp = 0x00;

}




/* ADC init function for eps state only conversion */
void ADC_EPS_STATE_Init( ADC_HandleTypeDef *hadc_eps)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
  hadc_eps->Instance = ADC1;
  hadc_eps->Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc_eps->Init.Resolution = ADC_RESOLUTION_12B;
  hadc_eps->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc_eps->Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc_eps->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc_eps->Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
  hadc_eps->Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
  hadc_eps->Init.ChannelsBank = ADC_CHANNELS_BANK_A;
  hadc_eps->Init.ContinuousConvMode = ENABLE;
  hadc_eps->Init.DiscontinuousConvMode = DISABLE;
  hadc_eps->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc_eps->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc_eps->Init.DMAContinuousRequests = ENABLE;
  hadc_eps->Init.NbrOfConversion = 6;
  HAL_ADC_Init(hadc_eps);

  /*Vbat*/
  sConfig.Channel = ADC_VBAT;
  sConfig.Rank = 9;
  HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

   /*Ibat+*/
  sConfig.Channel = ADC_IBAT_PLUS;
  sConfig.Rank = 10;
  HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

  /*Ibat-*/
  sConfig.Channel = ADC_IBAT_MINUS;
  sConfig.Rank = 11;
  HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

  /*I3v3*/
  sConfig.Channel = ADC_I3V3;
  sConfig.Rank = 12;
  HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

  /*I5v*/
  sConfig.Channel = ADC_I5V;
  sConfig.Rank = 13;
  HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

  /*cpu internal temp sensor*/
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 14;
  HAL_ADC_ConfigChannel(hadc_eps, &sConfig);

  HAL_ADC_Stop(hadc_eps);

}




void EPS_PowerModule_init(EPS_PowerModule *module_X, uint32_t starting_pwm_dutycycle, TIM_HandleTypeDef *htim, uint32_t timer_channel, ADC_HandleTypeDef *hadc_power_module, uint32_t ADC_channel_current, uint32_t ADC_channel_voltage){

	module_X->current =0;
	module_X->voltage =0;
	module_X->previous_power =0;
	module_X->pwm_duty_cycle = starting_pwm_dutycycle;
	module_X->htim_pwm = htim;
	module_X->timChannel = timer_channel;
	module_X->incremennt_flag = 1;
	module_X->hadc_power_module = hadc_power_module;

	//Start pwm with initialized from cube mx pwm duty cycle for timerX at timer_channel.
	HAL_TIM_PWM_Start(htim, timer_channel);

	//Properly Initialize adc handle for this pwm module
	ADC_EPS_POWER_MODULE_Init( hadc_power_module, ADC_channel_current, ADC_channel_voltage);


}


/* ADC init function to read eps state */
void ADC_EPS_POWER_MODULE_Init(ADC_HandleTypeDef *hadc_power_module, uint32_t ADC_channel_current, uint32_t ADC_channel_voltage)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
  hadc_power_module->Instance = ADC1;
  hadc_power_module->Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc_power_module->Init.Resolution = ADC_RESOLUTION_12B;
  hadc_power_module->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc_power_module->Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc_power_module->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc_power_module->Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
  hadc_power_module->Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
  hadc_power_module->Init.ChannelsBank = ADC_CHANNELS_BANK_A;
  hadc_power_module->Init.ContinuousConvMode = ENABLE;
  hadc_power_module->Init.DiscontinuousConvMode = DISABLE;
  hadc_power_module->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc_power_module->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc_power_module->Init.DMAContinuousRequests = ENABLE;
  hadc_power_module->Init.NbrOfConversion = 2;
  HAL_ADC_Init(hadc_power_module);


  /*module adc channel current*/
  sConfig.Channel = ADC_channel_current;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_4CYCLES;
  HAL_ADC_ConfigChannel(hadc_power_module, &sConfig);

  /*module adc channel voltage*/
  sConfig.Channel = ADC_channel_voltage;
  sConfig.Rank = 2;
  HAL_ADC_ConfigChannel(hadc_power_module, &sConfig);

  HAL_ADC_Stop(hadc_power_module);



}

void EPS_update_power_module_state(EPS_PowerModule *power_module){

	uint32_t adc_measurement_dma_power_modules[17]= { 0 };//2*8 +1

	adc_reading_complete = 0;
	HAL_ADC_Start_DMA(power_module->hadc_power_module, adc_measurement_dma_power_modules, 16);

	/*Process Measurements*/
	uint32_t voltage_avg =0;
	uint32_t current_avg =0;


	/*Wait till DMA ADC sequence transfer is ready*/
	while(adc_reading_complete==0){
		//wait for dma transfer complete.
	}
	//ADC must be stopped in the adc dma transfer complete callback.

	//de-interleave and sum voltage and current measurements.
	for (int sum_index = 0; sum_index < 16; sum_index+=2) {
		/*top*/
		current_avg = current_avg + adc_measurement_dma_power_modules[sum_index];
		voltage_avg = voltage_avg + adc_measurement_dma_power_modules[sum_index+1];
	}

	/*filter ting*/
	//average of 16 concecutive adc measurements.skip the first to avoid adc power up distortion.
	power_module->voltage = current_avg>>4;
	power_module->current = voltage_avg>>4;


}


void EPS_PowerModule_mppt_update_pwm(EPS_PowerModule *moduleX){

	//static uint8_t increment_flag = 1;//diplotriplotsekare oti to inint to kanei mono sthn arxh vlaks


	  /*power calculation*/

	  volatile uint32_t power_now_avg = moduleX->voltage * moduleX->current;
	  uint32_t duty_cycle = moduleX->pwm_duty_cycle;

	// decide duty cycle orientation - set increment flag.
	  if (power_now_avg  <(moduleX->previous_power)){

		  if (moduleX->incremennt_flag>0){
			  moduleX->incremennt_flag = 0;
		  }
		  else{
			  moduleX->incremennt_flag = 1;
		  }
	  }
    //add appropriate offset - create new duty cycle.


	  if(moduleX->incremennt_flag){
		  duty_cycle = duty_cycle+MPPT_STEP_SIZE;
	  }
	  else{
		  duty_cycle = duty_cycle-MPPT_STEP_SIZE;
	  }
 	  //Check for Overflow and Underflow
	  if (duty_cycle>350){duty_cycle=0;}
	  if (duty_cycle>320){duty_cycle=310;}
    // Set new PWM compare register
	  //duty_cycle =0;


	  moduleX->previous_power = power_now_avg;
	  moduleX->pwm_duty_cycle = duty_cycle;

}

void EPS_PowerModule_mppt_apply_pwm(EPS_PowerModule *moduleX){

	switch ( moduleX->timChannel ) {
	case TIM_CHANNEL_1:         /*  top module */
		moduleX->htim_pwm->Instance->CCR1 = moduleX->pwm_duty_cycle;
		break;
	case TIM_CHANNEL_2:         /*  bottom module */
		moduleX->htim_pwm->Instance->CCR2 = moduleX->pwm_duty_cycle;
		break;
	case TIM_CHANNEL_3:         /*  left module */
		moduleX->htim_pwm->Instance->CCR3 = moduleX->pwm_duty_cycle;
		break;
	case TIM_CHANNEL_4:         /*  right module */
		moduleX->htim_pwm->Instance->CCR4 = moduleX->pwm_duty_cycle;
		break;
	default:
		//error handling?
		break;
	}

}
