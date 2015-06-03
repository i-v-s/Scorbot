#include "mx_gpio.h"

typedef struct
{
    TIM_TypeDef * timer;
    int prec; // Допуск позиционирования
    volatile int pos; // Текущая позиция (если не в timer)
    int ref; // Заданная позиция
    int time;
    volatile int rate; // Скорость
    int oldPos; // Предыдущая позиция
    void (* forward)();
    void (* reverse)();
    void (* stop)();
} Motor;

typedef struct
{
    float (* getPos)();
    void (* moveTo)(float pos);
    float scale;
    int zero;
    int (* getSwitch)();
} Axis;

typedef struct
{
    char axis;
    float pos;
} Command;

extern Command program[64];
extern Command * cmdPtr;

extern Motor motors[6];
extern Axis axes[6];

int getMotorPos(Motor * motor);
void initEncoder(Motor * motor, TIM_TypeDef * timer);
void initMotor(Motor * motor, void (* forward)(), void (* reverse)(), void (* stop)());

void moveMotor(Motor * motor, int ref);
void motorsOff(void);
void motorsOn(void);

void ctlLoop();
void zero();