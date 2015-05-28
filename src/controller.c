#include "controller.h"
#include <stdlib.h>

Motor motors[6] = {0};


void initEncoder(Motor * motor, TIM_TypeDef * timer)
{
    motor->timer = timer;
    timer->SMCR = 3;
    timer->CCER = 0;
    timer->ARR = 0xFFFF;
    timer->CR1 = 1; // CEN
}

void initMotor(Motor * m, void (* forward)(), void (* reverse)(), void (* stop)())
{
    m->forward = forward;
    m->reverse = reverse;
    m->stop = stop;
    m->prec = 2;
}

int getMotorPos(Motor * motor)
{
    TIM_TypeDef * tim = motor->timer;
    if(tim) return (int16_t)tim->CNT;
    return motor->pos;
}

void moveMotor(Motor * motor, int ref)
{
    motor->ref = ref;
    /*int d = ref - getMotorPos(motor);
    if(!d) motor->stop();
    else if(d > 0) motor->forward();
    else motor->reverse();
    motor->d = d;*/
}
#define timeOut 100

struct Param
{
    char axis;
    int pos;
} par[20] = 
{
    {'a', 200}, {'b', 300}, {';'},
    {'a', -100}, {'b', 0}, {';'},
    {'c', 500}, {';'},
    {'d', 500}, {';'},
    {'e', 500}, {';'},
    {'a', 500}, {'c', 0}, {'d', 0}, {'e', 0}, {0}}, * pcp = par;


void ctlLoop()
{
    char ready = 1;
    for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor); m++)
    {
        int d = m->ref - getMotorPos(m);
        if(d > m->prec) 
        {
            m->forward();
            m->time = timeOut;
        }
        else if(d < -m->prec)
        {
            m->reverse();
            m->time = timeOut;
        }
        else
        {
            m->stop();
            if(m->time > 0) m->time--;
        }
        if(m->time) ready = 0;
    }
    if(ready && pcp->axis)
    {
        if(pcp->axis == ';') pcp++;
        for( ; pcp->axis && pcp->axis != ';'; pcp++)
        {
            char a = pcp->axis;
            if(a >= 'a' && a <= 'f') moveMotor(motors + a - 'a', pcp->pos);
        }
    }
}