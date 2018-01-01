#include <stdio.h>
#include "motion.h"
#include "usart.h"
#include "Trace.h"
#include "sensor.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"

//sensor variable
float xtmp, ytmp,z;

int16_t x, y;

char bufADC[50];

void ADCinit_SoftTrigger()		//configurations of adc(sensor) reader
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;

	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2 | RCC_APB2Periph_ADC3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin                    = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode                   = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd                   = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_CommonInitStructure.ADC_Mode               = ADC_DualMode_RegSimult; //dual reading, independently
	ADC_CommonInitStructure.ADC_Prescaler          = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode      = ADC_DMAAccessMode_Disabled;
	//ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_8Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitStructure.ADC_Resolution               = ADC_Resolution_10b;
	ADC_InitStructure.ADC_ScanConvMode             = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode       = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge     = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv         = ADC_ExternalTrigConv_T1_CC1;

	ADC_InitStructure.ADC_DataAlign                = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion          = 1;

	ADC_Init(ADC2, &ADC_InitStructure);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_2, 1, ADC_SampleTime_3Cycles);
	ADC_Init(ADC3, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 1, ADC_SampleTime_3Cycles);

	ADC_Cmd(ADC2, ENABLE);
	ADC_Cmd(ADC3, ENABLE);
}

// function to read ADC_pins
uint16_t ADC_read(ADC_TypeDef* ADCX,uint8_t channel,uint8_t ADC_SampleTime){
	ADC_RegularChannelConfig(ADCX,channel,1,ADC_SampleTime); //baca ADCX, ch x, diletakkan pada SQ1, ADC_SampleTime
	ADC_SoftwareStartConv(ADCX);
	while(ADC_GetFlagStatus(ADCX, ADC_FLAG_EOC)    ==0);
	return ADC_GetConversionValue(ADCX);
}


// function to read sensor
void read_sensor(){

	//this is the origin of read sensor
	x=ADC_read(ADC2,ADC_Channel_2,ADC_SampleTime_3Cycles); //pin A2
	y=ADC_read(ADC3,ADC_Channel_3,ADC_SampleTime_3Cycles); //pin A3
	xtmp = x;
	ytmp = y;
/*									1st set conversion
	if (x<=511){
		xtmp = 513-x;
		xdeg = xtmp/17.06*-1;
	}
	else {
		xtmp = x-511;
		xdeg = xtmp/17.06;
	}

	if (y<=511){
		ytmp = 513-y;
		ydeg = ytmp/17.06*-1;
	}
	else {
		ytmp = y-511;
		ydeg = ytmp/17.06;
	}
	if(xdeg>-1 && xdeg<1)xdeg = 0;
	if(ydeg>-1 && ydeg<1)ydeg = 0;
*/

	if (x<=506){
			xtmp = 506-x;
			xdeg = xtmp/16.866*-1;
		}
		else {
			xtmp = x-507;
			xdeg = xtmp/17.233;
		}

		if (y<=510){
			ytmp = 510-y;
			ydeg = ytmp/17*-1;
		}
		else {
			ytmp = y-510;
			ydeg = ytmp/17.1;
		}

/*
	//sprintf(bufADC,"\n\rX : %6.2f     Y : %6.2f",xdeg, ydeg ); USART_puts(bufADC);
	//sprintf(bufADC,"\n\rX : %d     Y : %d",x, y ); USART_puts(bufADC);

	//with PID n.b. disable this if PID with interupt
	//PID(xdeg, ydeg);

	//no PID
	//motor(xdeg, ydeg, 1);
	 */
}
