#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_conf.h"
#include "stdlib.h"
#include "delay.h"
#include "usart.h"
#include "string.h"
#include "motion.h"
#include "stdio.h"



char data_received,bufferX[50], data[28],mtr1_a[5],mtr1_b[5],mtr2_a[5],mtr2_b[5],mtr3_a[6],mtr3_b[5], temp[10];
int8_t mtr1a,mtr1b,mtr2a,mtr2b,mtr3a,mtr3b;
int ms=0, i=0, n=0, x=0, aftc=0;
int stateA = 0, stateB = 0, stateC = 0;

void USART_Config(){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	//Enable GPIOB & USART1 Clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	//Set port serial sebagai Alternate Function (AF)
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //Output (Tx) Push-Pull
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //Input (Rx) Pull-Up
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//Konfigurasi UART
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}

void USART_putchar(uint8_t ch){
	USART_SendData(USART1, ch);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
}

void USART_puts(char *ch){
	uint8_t i;
	for(i=0; ch[i]!=0; i++){
		USART_putchar(ch[i]);
	}
}

uint8_t USART_getchar(void){
	uint8_t ch;
	while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE)==RESET);
	ch = USART_ReceiveData(USART1);
	return ch;
}

void USART_IT_Enable(){
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void USART1_IRQHandler(void){

	// the format of usart input is : Axx,xx,Bxx,xx,Cxx,xx#  which x is number, and dont forget the hashtag in the end
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE)==SET){
		// this is for receive data

		data_received=USART_ReceiveData(USART1);
		USART_putchar(data_received);
		data[i]=data_received;
		i++;
		/*
		USART_puts("\n\rPress Any Key On Keyboard !");
		data_received=USART_getchar();
		USART_puts("\tSerial Data Received : ");
		USART_putchar(data_received);
		*/
	}

	if(data_received=='#'){

		setDataPWM();
	}
	if(data_received=='*'){

		setDataRPM();
	}
	if(data_received=='X'){
		//do some command here for the motors

	}
}

void setDataPWM(){

		for( i = 0; i<28; i++){
			if(data[i]=='A'){					// PWM MOTOR 1
				stateA = 1; n=0;							//n as index of assigner
				for(x = i+1;x<6;x++){

					mtr1_a[n] = data[x];
					n++;
					if(data[x+1]=='#'||data[x+1]==','){
						aftc = x+2;
						//USART_puts("break it\n");
						break;
					}
				}
				n = 0;
				for(x = aftc;x<aftc+6;x++){
					mtr1_b[n] = data[x];
					n++;
					if(data[x+1]=='#'||data[x+1]==','){
						aftc = x+2;
						//USART_puts("break it again\n");
						break;
					}
				}
			}

			if(data[i]=='B'){					// PWM MOTOR 2
				stateB = 1; n=0;							//n as index of assigner
				for(x = i+1;x<i+6;x++){
					mtr2_a[n] = data[x];
					n++;
					if(data[x+1]=='#'||data[x+1]==','){
						aftc = x+2;
						//USART_puts("break it B\n");
						break;
					}
				}
				n = 0;
				for(x = aftc;x<aftc+6;x++){
					mtr2_b[n] = data[x];
					n++;
					if(data[x+1]=='#'||data[x+1]==','){
						aftc = x+2;
						//USART_puts("break it B again\n");
						break;
					}
				}
			}
			if(data[i]=='C'){					// PWM MOTOR 3
				stateC = 1; n=0;							//n as index of assigner
				for(x = i+1;x<i+6;x++){

					mtr3_a[n] = data[x];
					n++;
					if(data[x+1]=='#'||data[x+1]==','){
						aftc = x+2;
						//USART_puts("break it C\n");
						break;
					}
				}
				n = 0;
				for(x = aftc;x<aftc+6;x++){
					mtr3_b[n] = data[x];
					n++;
					if(data[x+1]=='#'||data[x+1]==','){
						aftc = x+2;
						//USART_puts("break it C again\n");
						break;
					}
				}
			}
		}

		//just show some info

		USART_puts(mtr1_a);USART_puts("\n");
		USART_puts(mtr1_b);USART_puts("\n");
		USART_puts(mtr2_a);USART_puts("\n");
		USART_puts(mtr2_b);USART_puts("\n");
		USART_puts(mtr3_a);USART_puts("\n");
		USART_puts(mtr3_b);USART_puts("\n");

		//convert array to int

		mtr1a = atoi(mtr1_a); mtr1b = atoi(mtr1_b);
		mtr2a = atoi(mtr2_a); mtr2b = atoi(mtr2_b);
		mtr3a = atoi(mtr3_a); mtr3b = atoi(mtr3_b);

		//running the motor
		if(stateA==1)Motor1(mtr1a,mtr1b);
		if(stateB==1)Motor2(mtr2a,mtr2b);
		if(stateC==1)Motor3(mtr3a,mtr3b);

		//clearing the buffer
		memset(&data[0],0,sizeof(data));
		memset(&mtr1_a[0],0,sizeof(mtr1_a));
		memset(&mtr1_b[0],0,sizeof(mtr1_b));
		memset(&mtr2_a[0],0,sizeof(mtr2_a));
		memset(&mtr2_b[0],0,sizeof(mtr2_b));
		memset(&mtr3_a[0],0,sizeof(mtr3_a));
		memset(&mtr3_b[0],0,sizeof(mtr3_b));
		i=0; stateA = stateB = stateC = 0;
}

void setDataRPM(){
	USART_puts("\nSerial Data Received : \n");
			for( i = 0; i<19; i++){

				if(data[i]=='A'){					// rpm MOTOR 1
					stateA = 1; n=0;							//n as index of assigner
					for(x = i+1;x<i+5;x++){
						mtr1_a[n] = data[x];
						n++;
						if(data[x+1]=='*'||data[x+1]==','){
							//USART_puts("break it\n");
							break;
						}
					}
				}

				if(data[i]=='B'){					// rpm MOTOR 2
					stateB = 1; n=0;							//n as index of assigner
					for(x = i+1;x<i+5;x++){
						mtr2_a[n] = data[x];
						n++;
						if(data[x+1]=='*'||data[x+1]==','){
							break;
						}
					}
				}
				if(data[i]=='C'){					// PWM MOTOR 3
					stateC = 1; n=0;							//n as index of assigner
					for(x = i+1;x<i+5;x++){
						mtr3_a[n] = data[x];
						n++;
						if(data[x+1]=='*'||data[x+1]==','){
							break;
						}
					}

				}
			}

			USART_puts(mtr1_a);USART_puts("\n");
			USART_puts(mtr2_a);USART_puts("\n");
			USART_puts(mtr3_a);USART_puts("\n");

			//convert array to int

			mtr1a = atoi(mtr1_a);
			mtr2a = atoi(mtr2_a);
			mtr3a = atoi(mtr3_a);
//			sprintf(bufferX,"\nRPM    A: %d      B: %d      C:%d	", mtr1a, mtr2a,mtr3a);
//			USART_puts(bufferX);

			//running the motor
			if(stateA==1)RPM_A(mtr1a);
			if(stateB==1)RPM_B(mtr2a);
			if(stateC==1)RPM_C(mtr3a);

			//clearing the buffer
			memset(&data[0],0,sizeof(data));
			memset(&data_received,0,sizeof(data_received));
			memset(&mtr1_a[0],0,sizeof(mtr1_a));
			memset(&mtr2_a[0],0,sizeof(mtr2_a));
			memset(&mtr3_a[0],0,sizeof(mtr3_a));
			i=0; stateA = stateB = stateC = 0;
}
void NVIC_Config_2(){
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//NVIC_InitTypeDef NVIC_InitStructure;
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void NVIC_Config_1(){
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}
//-----------------------------------------------------------------------



