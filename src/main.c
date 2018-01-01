#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
//#include <math.h>
#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "motion.h"
#include "Trace.h"
#include "usart.h"
#include "sensor.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"

//#include "enc.h"
//#include "tm_stm32f4_rotary_encoder.h"

int pwm1 = 0;
int pwm2 = 100;
int nshow=0;
char buffer[100];
int main();

void _init(){
	//main();

}

void Init_pwm(){
	/*
		 * TIM3_Config 		= roda 1		@ Pin B4 Pin B5
		 * TIM4_Config 		= roda 2 & 3	@ Pin D12 Pin D13 & Pin D14 Pin D15
		 *
	*/
	TIM12_Config();
	TIM4_Config();
}

void Init_usart(){
	/*
	 * 		Pin B6 & Pin B7
	 */
	USART_Config();
	sampling_RPM();

	NVIC_Config_1();
	sampling_IT_Enable();//PID sampling

	//NVIC_Config_2();
	//sampling_IT_Enable();//PID sampling
	//USART_IT_Enable();
}

void Init_sensor(){
	/*
	 * 		X pin A1 & Y Pin A2
	 */
	ADCinit_SoftTrigger();
	//testing
	//ADC_Config_try();

}

void maju(){				//example
		Motor1(pwm1,pwm2);
		Motor2(pwm1,pwm2);
		Motor3(pwm1,pwm2);
	}

void Init_rotary(){
	encodersInit_A();
	encodersInit_B();
	encodersInit_C();
	//encodersReset();
	//encodersInit_All();
	//encInit();
}


int main(){

	Init_pwm();
	Init_usart();
	Init_rotary();
	Init_sensor();

	while(1){
		read_sensor();
		sprintf(buffer,"\n\r pidX = %6.2f	pidY = %6.2f	P = %6.2f	I = %6.2f	%6.2f ", pidX, pidY, a, b, e);USART_puts(buffer);
	}
}



