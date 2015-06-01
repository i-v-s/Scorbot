#include "mx_gpio.h"

typedef struct
{
    TIM_TypeDef * timer;
    int prec; // Допуск позиционирования
    int pos; // Текущая позиция (если не в timer)
    int ref; // Заданная позиция
    int time;
    int rate; // Скорость
    int oldPos; // Предыдущая позиция
    void (* forward)();
    void (* reverse)();
    void (* stop)();
} Motor;

typedef struct
{
    float (* getPos)();
    void (* moveTo)(float pos);
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

void ctlLoop();
void zero();