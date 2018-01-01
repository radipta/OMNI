
//available to the rest of the code
//speeds

void TIM12_Config(void);
void TIM4_Config(void);

void Motor1(uint8_t a,uint8_t b);
void Motor2(uint8_t a,uint8_t b);
void Motor3(uint8_t a,uint8_t b);

void RPM_A(int8_t rpm);
void RPM_B(int8_t rpm);
void RPM_C(int8_t rpm);

void moveX(int Vx, int Vy);

void encodersInit_A(void);
void encodersInit_B(void);
void encodersInit_C(void);

void encodersInit_All(void);

void encodersReset(void);
void encodersRead(void);
void initGPIO (void);

void sampling_RPM(void);
void sampling_IT_Enable(void);
void TIM7_IRQHandler(void);
void PID(float eX, float eY);
void motor(float pidX, float pidY);

extern float xdeg, ydeg;

extern float pidX;
extern float pidY;
extern float a,b,c;
extern double q,w,e;

