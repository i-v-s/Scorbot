#include "controller.h"
#include <stdlib.h>
#include "host_io.h"
#include "hw_config.h"

volatile int ticks = 0;

Motor motors[6] = {0};

float Apos() { return (getMotorPos(motors + 0) - axisA->zero) / axisA->scale;}
float Bpos() { return (-getMotorPos(motors + 1) - axisB->zero) / axisB->scale;}
float Cpos() { return (getMotorPos(motors + 2) - axisC->zero) / axisC->scale;}
float Dpos() { return (getMotorPos(motors + 3) + getMotorPos(motors + 4) - (axisD->zero << 1)) / (2.0 * axisD->scale);}
float Epos() { return (getMotorPos(motors + 3) - getMotorPos(motors + 4) - (axisE->zero << 1)) / (2.0 * axisD->scale);}
float Fpos() { return (getMotorPos(motors + 5) - axisF->zero) / axisF->scale;}

void Amove(float pos) { motors[0].ref = (int)(pos * axisA->scale) + axisA->zero;}
void Bmove(float pos) { motors[1].ref = -(int)(pos * axisB->scale) - axisB->zero;}
void Cmove(float pos) { motors[2].ref = (int)(pos * axisC->scale) + axisC->zero;}

void Dmove(float pos)
{ 
    int p = (int) (pos * axisD->scale) + axisD->zero;
    int d = motors[3].ref - motors[4].ref;
    motors[3].ref = p + d;
    motors[4].ref = p - d;
}

void Emove(float pos)
{ 
    int p = (int) (pos * axisE->scale) + axisE->zero;
    int s = motors[3].ref + motors[4].ref;
    motors[3].ref = s + p;
    motors[4].ref = s - p;
}

void Fmove(float pos) { motors[5].ref = (int)(pos * axisF->scale) + axisF->zero;}
unsigned char hold = 0;

void Hmove(float pos) 
{
    Motor * m = motors + 5;
    hold = (char)(pos * 255);
    if(hold)
    {
        m->forward();
        m->time = ticks + 5;
    }
}

float Hpos() { return (float)hold / 255.0;}

Axis axes[7] = 
{ //               ш/ед     ноль
    {Apos, Amove, 42.88900,  -58},
    {Bpos, Bmove, 33.02200, 1400},
    {Cpos, Cmove, 33.93700,  149},
    {Dpos, Dmove, -8.33333,   40},
    {Epos, Emove,  4.16666,    0},
    {Fpos, Fmove,  8.00000,    0},
    {Hpos, Hmove}
};

const char axisNames[] = "abcdefh";

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

void setMotorPos(Motor * motor, int pos)
{
    TIM_TypeDef * tim = motor->timer;
    if(tim) tim->CNT = pos;
    else motor->pos = pos;
}

void SysTick_Handler(void)
{
    ticks++;
    for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor); m++)
    {
        int pos = getMotorPos(m);
        int d = pos - m->oldPos;
        m->oldPos = pos;
        int rate = m->rate;
        m->rate = rate - ((rate - d * SysTickFreq) >> 3);
    }   
    GPIOE->ODR = 0x4F00 & ~GPIOD->IDR;
}

void moveMotor(Motor * motor, int ref)
{
    motor->ref = ref;
}

char motorsEnabled = 1;

void motorsOff(void)
{
    motorsEnabled = 0;
    for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor); m++)
        m->stop();
}

void motorsOn(void)
{
    motorsEnabled = 1;
}

Command program[64] = {0};
Command * cmdPtr = 0;

#define timeOut 100

int ctlMotor(Motor * m)
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
    return m->time;
}

int ctlHold(Motor * m)
{
    if(m->rate > 20)
    {
        m->time = ticks + 5;
        return 1;
    }
    if(ticks > m->time)
    {
        //m->stop();
        return 0;
    }
    return 1;
}

void ctlLoop()
{
    int notReady = 0;
    if(motorsEnabled) 
    {
        for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor) - 1; m++)
            notReady |= ctlMotor(m);
        if(hold)
            notReady |= ctlHold(motors + 5);
        else
            notReady |= ctlMotor(motors + 5);
    }

    if(!notReady && cmdPtr && cmdPtr->axis)
    { 
        if(cmdPtr->axis == nextCmd) cmdPtr++;
        for( ; cmdPtr->axis && cmdPtr->axis != nextCmd; cmdPtr++)
            cmdPtr->axis->moveTo(cmdPtr->pos);
        if(!cmdPtr->axis) cmdPtr = 0;
    }
}
