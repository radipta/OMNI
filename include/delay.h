#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
static __IO uint32_t sysTickCounter;

void SysTick_Init(void);
void TimeTick_Decrement(void);
void delay_us(u32 n);
void delay_1ms(void);
void delay_ms(u32 n);
void SysTick_Handler(void);
