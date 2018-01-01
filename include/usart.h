
void USART_Config(void);
void USART_putchar(uint8_t ch);
void USART_puts(char *ch);
uint8_t USART_getchar(void);
void USART_IT_Enable();
void USART1_IRQHandler(void);
void setDataRPM(void);
void setDataPWM(void);
void NVIC_Config_2(void);
void NVIC_Config_1(void);
