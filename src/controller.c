#include "controller.h"
#include <stdlib.h>
#include "host_io.h"
#include "hw_config.h"
#include <string.h>


volatile int ticks = 0;

Motor motors[6] = {0};

float Apos() { return (getMotorPos(motors + 0) - axisA->zero) / axisA->scale;}
float Bpos() { return (-getMotorPos(motors + 1) - axisB->zero) / axisB->scale;}
float Cpos() { return (getMotorPos(motors + 2) - axisC->zero) / axisC->scale;}
float Dpos() { return (getMotorPos(motors + 3) + getMotorPos(motors + 4) - axisD->zero) / axisD->scale;}
float Epos() { return (getMotorPos(motors + 3) - getMotorPos(motors + 4) - axisE->zero) / axisD->scale;}
float Fpos() { return (getMotorPos(motors + 5) - axisF->zero) / axisF->scale;}

float Aref() { return (motors[0].ref - axisA->zero) / axisA->scale;}
float Bref() { return (-motors[1].ref - axisB->zero) / axisB->scale;}
float Cref() { return (motors[2].ref - axisC->zero) / axisC->scale;}
float Dref() { return (motors[3].ref + motors[4].ref - axisD->zero) / axisD->scale;}
float Eref() { return (motors[3].ref - motors[4].ref - axisE->zero) / axisD->scale;}
float Fref() { return (motors[5].ref - axisF->zero) / axisF->scale;}


void Amove(float pos) { motors[0].ref = (int)(pos * axisA->scale) + axisA->zero;}
void Bmove(float pos) { motors[1].ref = -(int)(pos * axisB->scale) - axisB->zero;}
void Cmove(float pos) { motors[2].ref = (int)(pos * axisC->scale) + axisC->zero;}

void Dmove(float pos)
{ 
    int p = (int) (pos * axisD->scale) + axisD->zero;
    int d = motors[3].ref - motors[4].ref;
    motors[3].ref = (p + d) >> 1;
    motors[4].ref = (p - d) >> 1;
}

void Emove(float pos)
{ 
    int p = (int) (pos * axisD->scale) + axisE->zero;
    int s = motors[3].ref + motors[4].ref;
    motors[3].ref = (s + p) >> 1;
    motors[4].ref = (s - p) >> 1;
}

void Fmove(float pos) { motors[5].ref = (int)(pos * axisF->scale) + axisF->zero;}
unsigned char hold = 0;

void Hmove(float pos) 
{
    Motor * m = motors + 5;
    if(pos > 100.0) pos = 100.0;
    hold = (char)(pos * 2.55);
    if(hold)
    {
        m->forward();
        m->time = ticks + 5;
    }
}

float Hpos() { return (float)hold / 2.55;}

Axis axes[7] = 
{ //               ш/ед     ноль
    {Apos, Aref, Amove,  42.88900,  -58},
    {Bpos, Bref, Bmove,  33.02200, 1400},
    {Cpos, Cref, Cmove,  33.93700,  149},
    {Dpos, Dref, Dmove, -16.66666,  160},
    {Epos, Eref, Emove, -16.66666,  350},
    {Fpos, Fref, Fmove,   8.00000,    0},
    {Hpos, Hpos, Hmove}
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

typedef void (* TickHandler) ();
TickHandler tickHandlers[8], * thDst = tickHandlers;

#ifdef __cplusplus
 extern "C" {
#endif

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
    for(TickHandler * x = tickHandlers; x < thDst; x++) (*x)();
}

#ifdef __cplusplus
 }
#endif


void addTickHandler(void (* h) ())
{
    *(thDst++) = h;
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
int doCount = 0;

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

Command commands[64];
Command * volatile cmdSrc = commands, * volatile cmdDst = commands;

char pushCommand(Command * cmd)
{
    int len = 0;
    for(Command * t = cmd; t->axis; t++) len++;
    
    Command * s = cmdSrc, * d = cmdDst;
    int empty = (s - d - 2) & (sizeof(commands) / sizeof(Command) - 1); // Сколько всего места в буфере осталось
    if(empty < len) return 0;
    while(len)
    { // Запись в кольцевой буфер
        int left = commands + sizeof(commands) / sizeof(Command) - d; // Сколько до конца отрезка
        if(left > len) left = len;
        memcpy(d, cmd, left * sizeof(Command));
        cmd += left;
        len -= left;
        d += left;
        if(d >= commands + sizeof(commands) / sizeof(Command)) d = commands;
    }
    d->axis = nextCmd;
    d++;
    if(d >= commands + sizeof(commands) / sizeof(Command)) d = commands;
    cmdDst = d;
    return 1;
}

void allStop()
{
    for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor) - 1; m++)
    {
        m->stop();
        m->ref = getMotorPos(m);
    }
    cmdSrc = commands;
    cmdDst = commands;
    cmdPtr = 0;
}

void runCmd() 
{               // Читаем команду
    if(cmdPtr)  // из программы
    {
        if(cmdPtr->axis == nextCmd) cmdPtr++;
        for( ; cmdPtr->axis && cmdPtr->axis != nextCmd; cmdPtr++)
            cmdPtr->axis->moveTo(cmdPtr->pos);
        if(!cmdPtr->axis)
        {
            if(doCount) {cmdPtr = program; doCount--;}
            else cmdPtr = 0;
        }
        return;
    }
    Command * s = cmdSrc, * d = cmdDst;
    if(d != s) // или из очереди
    {
        do
        {
            Command * t = s++;
            if(s >= commands + sizeof(commands) / sizeof(Command)) s = commands;
            if(t->axis == nextCmd) 
                break;
            else
                t->axis->moveTo(t->pos);
        } while(d != s);
        cmdSrc = s;
    }
}

void ctlLoop()
{
    int notReady = 0;
    if(motorsEnabled) for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor) - 1; m++)
            notReady |= ctlMotor(m);
    if(hold)
        notReady |= ctlHold(motors + 5);
    else
        if(motorsEnabled) notReady |= ctlMotor(motors + 5);

    if(motorsEnabled && !notReady) runCmd();
}
