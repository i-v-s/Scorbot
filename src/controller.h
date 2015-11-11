#include "mx_gpio.h"

typedef struct
{
//private:
    volatile int pos; // Текущая позиция (если не в timer)
    TIM_TypeDef * timer;
//public:
    int prec; // Допуск позиционирования
    int ref; // Заданная позиция
    int time, state;
    volatile int rate; // Скорость
    int oldPos; // Предыдущая позиция
    void (* forward)();
    void (* reverse)();
    void (* stop)();
    int control();
    inline int getPos() { return timer ? (int16_t)timer->CNT : pos;}
} Motor;

typedef struct
{
    float (* getPos)();
    float (* getRef)();
    void (* moveTo)(float pos);
    float scale;
    int zero;
    int (* getSwitch)();
} Axis;

typedef struct
{
    Axis * axis;
    float pos;
} Command;

#define PROGRAM_COUNT 4
#define PROGRAM_SIZE 64

extern Command program[PROGRAM_COUNT][PROGRAM_SIZE];
extern Command * volatile cmdPtr;
extern int doCount;
extern int programNo;

extern Motor motors[6];
extern Axis axes[11];
extern const char axisNames[];
extern volatile int ticks;

#define axisA (axes + 0)
#define axisB (axes + 1)
#define axisC (axes + 2)
#define axisD (axes + 3)
#define axisE (axes + 4)
#define axisF (axes + 5)
#define axisH (axes + 6)
#define axisR (axes + 7)
#define axisX (axes + 8)
#define axisY (axes + 9)
#define axisZ (axes + 10)
#define nextCmd ((Axis *) 1)

void initEncoder(Motor * motor, TIM_TypeDef * timer);
void initMotor(Motor * motor, void (* forward)(), void (* reverse)(), void (* stop)());
void setMotorPos(Motor * motor, int pos);
Command * moveTo(Command * c);

#ifdef __cplusplus
 extern "C" {
#endif
void addTickHandler(void (* h) (void * obj), void * obj = 0);
#ifdef __cplusplus
 }
#endif

void moveMotor(Motor * motor, int ref);
void motorsOff(void);
void motorsOn(void);

void ctlLoop();
char zero(char noZero);
bool pushCommand(Command * cmd);
void allStop();
