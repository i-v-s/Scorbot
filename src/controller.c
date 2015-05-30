#include "controller.h"
#include <stdlib.h>
#include "host_io.h"
#include "hw_config.h"

Motor motors[6] = {0};


float Apos() { return getMotorPos(motors + 0);}
float Bpos() { return -getMotorPos(motors + 1);}
float Cpos() { return getMotorPos(motors + 2);}
float Dpos() { return (getMotorPos(motors + 3) + getMotorPos(motors + 4)) / 2.0;}
float Epos() { return (getMotorPos(motors + 3) - getMotorPos(motors + 4)) / 2.0;}
float Fpos() { return getMotorPos(motors + 5);}

void Amove(float pos) { motors[0].ref = (int)pos;}
void Bmove(float pos) { motors[1].ref = -(int)pos;}
void Cmove(float pos) { motors[2].ref = (int)pos;}

void Dmove(float pos)
{ 
    int p = (int) pos;
    int d = motors[3].ref - motors[4].ref;
    motors[3].ref = p + d;
    motors[4].ref = p - d;
}

void Emove(float pos)
{ 
    int p = (int) pos;
    int s = motors[3].ref + motors[4].ref;
    motors[3].ref = s + p;
    motors[4].ref = s - p;
}

void Fmove(float pos) { motors[5].ref = (int)pos;}

Axis axes[6] = 
{
    {Apos, Amove},
    {Bpos, Bmove},
    {Cpos, Cmove},
    {Dpos, Dmove},
    {Epos, Emove},
    {Fpos, Fmove}
};
int ticks = 0;



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
        m->rate -= (m->rate - d * SysTickFreq) >> 3;
    }   
    GPIOE->ODR = 0x4F00 & ~GPIOD->IDR;
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
#define zt 150

void zero()
{
    //char buf[20];
    
    // ��� B
    sendText("\nZero: B>> ");
    motors[1].forward();
    int to = ticks + 30;
    for(int brk = zt, rv2 = zt; brk; brk--)
    {
        int sw = switches;
        if(!(sw & 4)) brk = zt;
        if(ticks > to && motors[1].rate < 10)
        {
            sendText("B no move ");
            motors[1].stop();
            motors[2].stop();
            return;
        }
        if(sw & 2) rv2--; else rv2 = zt;
        if(!rv2)//!axes[2].getSwitch())
        {
            motors[1].stop();
            setMotorPos(motors + 2, 500);
            motors[2].reverse();
            sendText("B<> C<< ");
            while(getMotorPos(motors + 2) > 0);
            motors[1].forward();
            sendText("B>> ");
            to = ticks + 30;
            //sendText("C<< ");
            //break;
        }
    }

    motors[1].reverse();
    sendText("B<< ");
    for(int brk = zt; brk; brk--)
        if(switches & 4) brk = zt;
    
    motors[1].stop();
    setMotorPos(motors + 1, 0);
    sendText("Bok ");
    
    // ��� C
    sendText("C>> ");
    motors[2].forward();
    for(int brk = zt; brk; brk--)
        if(!(switches & 2)) brk = zt;
    sendText("C<< ");
    motors[2].reverse();
    for(int brk = zt; brk; brk--)
        if(switches & 2) brk = zt;

    motors[2].stop();
    setMotorPos(motors + 2, 0);
    sendText("Cok ");
    
    // ��� A
    sendText("A>> ");
    motors[0].forward();
    to = ticks + 10;
    char r = 0;
    for(int brk = zt; brk; brk--)
    {
        if(!(switches & 8)) brk = zt;
        if(abs(motors[0].rate) < 20 && ticks > to) // ����� ������, � ��������� �� ��������
        {
            if(r)
            {
                sendText("A no move ");
                motors[0].stop();
                return;
            } 
            else
            { // �������������
                motors[0].reverse();
                sendText("A! A<< ");
                to = ticks + 10;
                r = 1;
            }
        }
    }
    sendText("A<< ");
    motors[0].reverse();
    for(int brk = zt; brk; brk--)
        if(switches & 8) brk = zt;

    motors[0].stop();
    setMotorPos(motors + 0, 0);
    sendText("Aok ");
    
    // ��� D
    sendText("D>> ");
    motors[3].forward();
    motors[4].forward();
    r = 0;
    to = ticks + 10;
    for(int brk = zt; brk; brk--)
    {
        if(!(switches & 1)) brk = zt;
        if((abs(motors[3].rate) < 20 || abs(motors[4].rate) < 20) && ticks > to) // ����� ������, � ��������� �� ��������
        {
            if(r)
            {
                sendText("D no move ");
                motors[3].stop();
                motors[4].stop();
                return;
            } 
            else
            { // �������������
                motors[3].reverse();
                motors[4].reverse();
                sendText("D! D<< ");
                to = ticks + 10;
                r = 1;
            }
        }
    }
    sendText("D<< ");
    motors[3].reverse();
    motors[4].reverse();
    for(int brk = zt; brk; brk--)
        if(switches & 1) brk = zt;
    motors[3].stop();
    motors[4].stop();
    setMotorPos(motors + 3, 0);
    setMotorPos(motors + 4, 0);
    sendText("Dok ");
    
    // ��� E
    sendText("E>> ");
    motors[4].reverse();
    motors[3].forward();
    for(int brk = zt; brk; brk--)
        if(!(switches & 0x40)) brk = zt;
    sendText("E<< ");
    motors[4].forward();
    motors[3].reverse();
    for(int brk = zt; brk; brk--)
        if(switches & 0x40) brk = zt;
        
    motors[3].stop();
    motors[4].stop();
    int d = (getMotorPos(motors + 3) + getMotorPos(motors + 4)) >> 1;
    setMotorPos(motors + 3, d);
    setMotorPos(motors + 4, d);
    
    //setMotorPos(motors + 3, 0);
    //setMotorPos(motors + 4, 0);
    sendText("Eok ");
    
    
    //sprintf(buf, "sw:(%X) ", switches);
    //sendText(buf);
    motors[1].stop();
    motors[2].stop();
    return;
}

struct Param
{
    char axis;
    int pos;
} par[20] = 
{
    {0},
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