#include "mx_gpio.h"

typedef struct
{
    TIM_TypeDef * timer;
    int prec;
    int pos;
    int ref;
    int time;
    void (* forward)();
    void (* reverse)();
    void (* stop)();
} Motor;

extern Motor motors[6];

int getMotorPos(Motor * motor);
void initEncoder(Motor * motor, TIM_TypeDef * timer);
void initMotor(Motor * motor, void (* forward)(), void (* reverse)(), void (* stop)());

void moveMotor(Motor * motor, int ref);

void ctlLoop();
