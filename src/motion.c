#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "motion.h"
#include "usart.h"
#include "Trace.h"
#include "sensor.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "inttypes.h"
//#include "enc.h"
#define PI 3.14159265
//motor peripheral
static volatile int16_t A_enc;
static volatile int16_t B_enc;
static volatile int16_t C_enc;
/*
volatile int16_t A_rpm;
volatile int16_t B_rpm;
volatile int16_t C_rpm;
*/
volatile int32_t A_rpm;
volatile int32_t B_rpm;
volatile int32_t C_rpm;

float KpX=80, KiX=0.01, KdX=0;
float KpY=80, KiY=0.01, KdY=0;

float xdeg = 0, ydeg = 0;	//var for tampung data sensor
float pidX=0, error_X=0, i_errorX=0, d_errorX=0, eX_last=0;
float pidY=0, error_Y=0, i_errorY=0, d_errorY=0, eY_last=0;
float a,b,c,x,y;
double q,w,e;
int samplingtime = 5;
char buffer[100];

/* timer used :
 * TIM 4 & 12 for PWM motor
 * TIM 7 for sampling rpm
 * TIM 2 -> encoder A
 * TIM 3 -> encoder B`
 * TIM 5 -> encoder C
 *
 *
 *	PIN Out :
 *	enc motor A = A15 - B3
 *	enc motor B = B4 - B5
 *	enc motor C = A0 - A1
 *
 *	PWM motor 1	= B14 - B15
 *	PWM motor 2	= D12 - D13
 *	PWM motor 3 = D14 - D150
 *
 *	TX USB TTL	= PB7
 *	RX USB TTL 	= PB6
 *
 *	sensor :  X = A3 & Y = A2 (2 pin adc)
 *
 *	the format of usart input is : Axx,xx,Bxx,xx,Cxx,xx#
 *		which x is number, and dont forget the hashtag in the end
 */
char show[10];
unsigned int total = 0;
int pwm_period = 140;//150;	// 100/pwm period = pwm frequency

void initGPIO (void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef(GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode                = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType               = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin                 = GPIO_Pin_15 ;
	GPIO_InitStruct.GPIO_PuPd                = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed               = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void TIM12_Config()//actually this is for TIM12_Config			//configuration of pwm motors
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;

	RCC_PCLK1Config(RCC_HCLK_Div16);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12,ENABLE);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14, GPIO_AF_TIM12); //pwm ch1
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15, GPIO_AF_TIM12); //pwm ch2
	GPIO_InitStructure.GPIO_Pin              = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode             = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed            = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType            = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd             = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	//fpwm                                   =(10.5MHz*2)/(210*100)=1000Hz
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_Prescaler     = 210-1;
	TIM_TimeBaseInitStruct.TIM_Period        = pwm_period-1;
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM12,&TIM_TimeBaseInitStruct);
	TIM_Cmd(TIM12,ENABLE);

	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OutputState         = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_OCMode              = TIM_OCMode_PWM1; //mode non inverting
	TIM_OCInitStruct.TIM_Pulse               = 0;
	TIM_OC1Init(TIM12,&TIM_OCInitStruct);
	TIM_OC2Init(TIM12,&TIM_OCInitStruct);
	TIM_Cmd(TIM12,ENABLE);

	TIM_OC1PreloadConfig(TIM12,TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM12,TIM_OCPreload_Enable);
	TIM_CCxCmd(TIM12,TIM_Channel_1,TIM_CCx_Enable);
	TIM_CCxCmd(TIM12,TIM_Channel_2,TIM_CCx_Enable);
}

/*
void TIM3_Config()			//TIM3_Config original for pwm before timer change
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;

	RCC_PCLK1Config(RCC_HCLK_Div16);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource4, GPIO_AF_TIM3); //pwm ch1
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource5, GPIO_AF_TIM3); //pwm ch2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	//fpwm=(10.5MHz*2)/(210*100)=1000Hz
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 210-1;
	TIM_TimeBaseInitStruct.TIM_Period = 100-1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);
	TIM_Cmd(TIM3,ENABLE);

	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; //mode non inverting
	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM3,&TIM_OCInitStruct);
	TIM_OC2Init(TIM3,&TIM_OCInitStruct);
	TIM_Cmd(TIM3,ENABLE);

	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);
	TIM_CCxCmd(TIM3,TIM_Channel_1,TIM_CCx_Enable);
	TIM_CCxCmd(TIM3,TIM_Channel_2,TIM_CCx_Enable);
}
*/

void TIM4_Config()			//configuration of pwm motors -
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;

	RCC_PCLK1Config(RCC_HCLK_Div16);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12, GPIO_AF_TIM4); //pwm ch1
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource13, GPIO_AF_TIM4); //pwm ch2
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14, GPIO_AF_TIM4); //pwm ch3
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource15, GPIO_AF_TIM4); //pwm ch4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD,&GPIO_InitStructure);

	//fpwm=(10.5MHz*2)/(210*100)=1000Hz
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 210-1;
	TIM_TimeBaseInitStruct.TIM_Period = pwm_period-1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStruct);
	TIM_Cmd(TIM4,ENABLE);

	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; //mode non inverting
	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM4,&TIM_OCInitStruct);
	TIM_OC2Init(TIM4,&TIM_OCInitStruct);
	TIM_OC3Init(TIM4,&TIM_OCInitStruct);
	TIM_OC4Init(TIM4,&TIM_OCInitStruct);
	TIM_Cmd(TIM4,ENABLE);

	TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM4,TIM_OCPreload_Enable);
	TIM_CCxCmd(TIM4,TIM_Channel_1,TIM_CCx_Enable);
	TIM_CCxCmd(TIM4,TIM_Channel_2,TIM_CCx_Enable);
	TIM_CCxCmd(TIM4,TIM_Channel_3,TIM_CCx_Enable);
	TIM_CCxCmd(TIM4,TIM_Channel_4,TIM_CCx_Enable);
}

void Motor3(uint8_t a,uint8_t b){
		TIM12->CCR1 = a;
		TIM12->CCR2 = b;
	}
void Motor2(uint8_t a,uint8_t b){
		TIM4->CCR1 = a;
		TIM4->CCR2 = b;
	}
void Motor1(uint8_t a,uint8_t b){
		TIM4->CCR3 = a;
		TIM4->CCR4 = b;
	}

void RPM_C(int8_t rpm){
	rpm = (rpm*100)/280;
	if (rpm<0){
		TIM12->CCR1 = abs(rpm); 	TIM12->CCR2 = 0;
	}
	else if(rpm>0){
		TIM12->CCR1 = 0; 	TIM12->CCR2 = abs(rpm);
	}
	else{
		TIM12->CCR1 = 0;	TIM12->CCR2 = 0;
	}
}
void RPM_B(int8_t rpm){
	rpm = (rpm*100)/280;
	if (rpm<0){
		TIM4->CCR1 = abs(rpm);	TIM4->CCR2 = 0;
	}
	else if(rpm>0){
		TIM4->CCR1 = 0;		TIM4->CCR2 = abs(rpm);
	}
	else{
		TIM4->CCR1 = 0;		TIM4->CCR2 = 0;
	}
}
void RPM_A(int8_t rpm){
	rpm = (rpm*100)/280;
	if (rpm<0){
		TIM4->CCR3 = rpm;	TIM4->CCR4 = 0;
	}
	else if(rpm>0){
		TIM4->CCR3 = 0;		TIM4->CCR4 = rpm;
	}
	else{
		TIM4->CCR3 = 0;		TIM4->CCR4 = 0;
	}
}

void encodersInit_C (void)	//configurations of encoder timer: A15 - B3
{
  GPIO_InitTypeDef GPIO_InitStructure;
  // turn on the clocks for each of the ports needed
  RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);

  // now configure the pins themselves
  // they are all going to be inputs with pullups
  GPIO_StructInit (&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init (GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init (GPIOB, &GPIO_InitStructure);

  // Connect the pins to their Alternate Functions
  GPIO_PinAFConfig (GPIOA, GPIO_PinSource15, GPIO_AF_TIM2);
  GPIO_PinAFConfig (GPIOB, GPIO_PinSource3, GPIO_AF_TIM2);

  // Timer peripheral clock enable
  RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM2, ENABLE);

  // set them up as encoder inputs
  // set both inputs to rising polarity to let it use both edges
  TIM_EncoderInterfaceConfig (TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_SetAutoreload (TIM2, 0xffff);


  // turn on the timer/counters
  TIM_Cmd(TIM2, ENABLE);

  encodersReset();
}

void encodersInit_B (void)	//configurations of encoder timer : PB 4, PB 5
{
  GPIO_InitTypeDef GPIO_InitStructure;
  // turn on the clocks for each of the ports needed
  RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);

  // now configure the pins themselves
  // they are all going to be inputs with pullups
  GPIO_StructInit (&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_Init (GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init (GPIOB, &GPIO_InitStructure);

  // Connect the pins to their Alternate Functions
  GPIO_PinAFConfig (GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
  GPIO_PinAFConfig (GPIOB, GPIO_PinSource5, GPIO_AF_TIM3);

  // Timer peripheral clock enable
  RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM3, ENABLE);

  // set them up as encoder inputs
  // set both inputs to rising polarity to let it use both edges
  TIM_EncoderInterfaceConfig (TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_SetAutoreload (TIM3, 0xffff);

  // turn on the timer/counters
  TIM_Cmd(TIM3, ENABLE);
  encodersReset();
}

void encodersInit_A (void)	//configurations of encoder timer : PA 0 , PA 1
{
  GPIO_InitTypeDef GPIO_InitStructure;
  // turn on the clocks for each of the ports needed
  RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE);

  // now configure the pins themselves
  // they are all going to be inputs with pullups
  GPIO_StructInit (&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init (GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_Init (GPIOA, &GPIO_InitStructure);

  // Connect the pins to their Alternate Functions
  GPIO_PinAFConfig (GPIOA, GPIO_PinSource0, GPIO_AF_TIM5);
  GPIO_PinAFConfig (GPIOA, GPIO_PinSource1, GPIO_AF_TIM5);

  // Timer peripheral clock enable
  RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM5, ENABLE);

  // set them up as encoder inputs
  // set both inputs to rising polarity to let it use both edges
  TIM_EncoderInterfaceConfig (TIM5, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_SetAutoreload (TIM5, 0xffff);

  // turn on the timer/counters
  TIM_Cmd(TIM5, ENABLE);

  encodersReset();
}

void encodersRead()			//reading the encoder data
{
	  //total = total + A_encoder;
  C_enc = TIM_GetCounter(TIM2) ;
  B_enc = TIM_GetCounter(TIM3);
  A_enc = TIM_GetCounter(TIM5);
  //trace_printf("\nA:%d  B:%d  C:%d", A_encoder, B_encoder, C_encoder);

  //sprintf(buffer,"\nRPM  A: %"PRId32"   B: %"PRId32"   C:%"PRId32" ", A_encoder, B_encoder,C_encoder);
  //sprintf(buffer,"\nRPM  A: %d   B: %d   C:%d ", A_encoder, B_encoder,C_encoder);
  USART_puts(buffer);
  if(A_enc > 31) encodersReset();
}

void encodersReset()				//reset the counter of encoder
{
  __disable_irq();
  TIM_SetCounter (TIM2, 0);
  TIM_SetCounter (TIM3, 0);
  TIM_SetCounter (TIM5, 0);
  __enable_irq();
}

void GetRPM(){
	C_enc = TIM_GetCounter(TIM2);
	B_enc = TIM_GetCounter(TIM3);
	A_enc = TIM_GetCounter(TIM5);

	C_rpm = A_enc*600/560;
	B_rpm = B_enc*600/560;
	A_rpm = C_enc*600/560;
	sprintf(buffer,"\nRPM    A: %ld      B: %ld      C:%ld	", A_rpm, B_rpm,C_rpm);USART_puts(buffer);
	encodersReset();
}


void sampling_RPM(){

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_PCLK1Config(RCC_HCLK_Div16);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	//TIM_TimeBaseStructure.TIM_Prescaler = 420-1;//this is 100 ms, 200ms(840-1), 1 second(4200-1);
	TIM_TimeBaseStructure.TIM_Prescaler = 4200-1;		//4200 is 100ms
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//TIM_TimeBaseStructure.TIM_Period = 5000-1;
	TIM_TimeBaseStructure.TIM_Period = 400-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM7,&TIM_TimeBaseStructure);
}

void sampling_IT_Enable(){
	TIM_Cmd(TIM7,ENABLE);
	TIM_ITConfig(TIM7,TIM_IT_Update, ENABLE);
}


void TIM7_IRQHandler()// rutin interrupt timer sampling
{
	if(TIM_GetITStatus(TIM7,TIM_IT_Update)!=RESET)
	{
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	//enable this if you want to get RPM value
		//GetRPM();
									//sprintf(buffer,"\12345678");USART_puts(buffer);
	//interupt for PID calculation

		//sprintf(buffer,"\n\r%6.2f		%6.2f", xdeg, ydeg);USART_puts(buffer);
		PID(xdeg, ydeg);


	}
}

void PID(float eX, float eY){
	if(eX>-1 && eX<1){eX = 0;}
	if(eY>-1 && eY<1){eY = 0;}

	if(eX !=0){
		i_errorX = i_errorX+(eX*samplingtime);
		d_errorX = (eX - eX_last)/samplingtime;
		eX_last = eX;
		a = KpX*eX;
		b = KiX*i_errorX;

		pidX = a+b+(KdX*d_errorX);
		//pidX = (KpX*eX) + (KiX*i_errorX) + (KdX*d_errorX);
	}
	else{
		i_errorX = 0;
		a = KpX*eX;
		b = KiX*i_errorX;
		pidX = 0;
	}

	if(eY != 0){
		i_errorY = i_errorY+(eY*samplingtime);
		d_errorX = (eY - eY_last)/samplingtime;
		eY_last = eY;
		pidY = (KpY*eY)+(KiY*i_errorY)+(KdY*d_errorY);
	}
	else{
		i_errorY = 0;
		pidY = 0;
	}


	/*
	i_errorX = i_errorX+(eX*samplingtime);
	d_errorX = (eX - eX_last)/samplingtime;
	eX_last = eX;
	a = KpX*eX;
	b = KiX*i_errorX;
	pidX = a+b+(KdX*d_errorX);

	i_errorY = i_errorY+(eY*samplingtime);
	d_errorX = (eY - eY_last)/samplingtime;
	eY_last = eY;
	pidY = (KpY*eY)+(KiY*i_errorY)+(KdY*d_errorY);
	*/

	if(pidX<-255)		pidX = -255;
	else if(pidX>255)	pidX = 255;
	if(pidY<-255) 		pidY = -255;
	else if(pidY>255) 	pidY = 255;

	e = atan2(eX,eY) * 180 / PI;

	//sprintf(buffer,"\n\r pidX = %6.2f	pidY = %6.2f	P = %6.2f	I = %6.2f	%6.2f ", pidX, pidY, a, b, e);USART_puts(buffer);
	//sprintf(buffer,"\n\r pidX = %6.2f	pidY = %6.2f	P = %6.2f	I = %6.2f	D = %6.2f", pidX, pidY, a, b, c );USART_puts(buffer);
	//sprintf(buffer,"\n\r pidX = %6.2f	pidY = %6.2f", pidX, pidY );USART_puts(buffer);
	motor(pidX, pidY);
}


void motor(float pidX, float pidY){

	float V1 = (-0.3333*pidX)+(0.5774*pidY);
	float V2 = (-0.3333*pidX)+(-0.5774*pidY);
	float V3 = (0.6667*pidX);

	//sprintf(buffer,"\n\r V1 = %6.2f	V2 = %6.2f	V3 = %6.2f",V1, V2, V3);USART_puts(buffer);
	if(V1<0){
		V1*=-1;	TIM4->CCR3 = (int)V1;	TIM4->CCR4 = 0;
	}
	else if(V1>0){
		TIM4->CCR3 = 0;		TIM4->CCR4 = (int)V1;
	}
	else{
		TIM4->CCR3 = 0;		TIM4->CCR4 = 0;
	}
	if(V2<0){
		V2*=-1;	TIM4->CCR1 = (int)V2;	TIM4->CCR2 = 0;
	}
	else if(V2>0){
		TIM4->CCR1 = 0;		TIM4->CCR2 = (int)V2;
	}
	else{
		TIM4->CCR1 = 0;	TIM4->CCR2 = 0;
	}
	if(V3<0){
		V3*=-1;	TIM12->CCR1 = (int)V3;	TIM12->CCR2 = 0;
	}
	else if(V3>0){
		TIM12->CCR1 = 0;		TIM12->CCR2 = (int)V3;
	}
	else{
		TIM12->CCR1 = 0;		TIM12->CCR2 = 0;
	}
}
