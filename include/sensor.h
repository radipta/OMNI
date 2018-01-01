#include "stm32f4xx_adc.h"
void ADC_Config(void);
void ADC_Config_GPIOA(void);
void ADCinit_SoftTrigger(void);
uint16_t ADC_read(ADC_TypeDef* ADCX,uint8_t channel,uint8_t ADC_SampleTime);
void read_sensor(void);
int sensorX();
int sensorY();
