#include "controller.h"
#include <stdlib.h>
#include "host_io.h"
#include "hw_config.h"

Motor motors[6] = {0};

#define axisA (axes + 0)
#define axisB (axes + 1)
#define axisC (axes + 2)
#define axisD (axes + 3)
#define axisE (axes + 4)
#define axisF (axes + 5)

float Apos() { return (getMotorPos(motors + 0) - axisA->zero) / axisA->scale;}
float Bpos() { return (-getMotorPos(motors + 1) - axisB->zero) / axisB->scale;}
float Cpos() { return (getMotorPos(motors + 2) - axisC->zero) / axisC->scale;}
float Dpos() { return (getMotorPos(motors + 3) + getMotorPos(motors + 4)) / 2.0;}
float Epos() { return (getMotorPos(motors + 3) - getMotorPos(motors + 4)) / 2.0;}
float Fpos() { return (getMotorPos(motors + 5) - axisF->zero) / axisF->scale;}

void Amove(float pos) { motors[0].ref = (int)(pos * axisA->scale) + axisA->zero;}
void Bmove(float pos) { motors[1].ref = -(int)(pos * axisB->scale) - axisB->zero;}
void Cmove(float pos) { motors[2].ref = (int)(pos * axisC->scale) + axisC->zero;}

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

void Fmove(float pos) { motors[5].ref = (int)(pos * axisF->scale) + axisF->zero;}

Axis axes[6] = 
{ //               �/��   ����
    {Apos, Amove, 42.889,  -58},
    {Bpos, Bmove, 33.022, 1400},
    {Cpos, Cmove, 33.937,  149},
    {Dpos, Dmove},
    {Epos, Emove},
    {Fpos, Fmove, 16.000,    0}
};

volatile int ticks = 0;

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
    /*int d = ref - getMotorPos(motor);
    if(!d) motor->stop();
    else if(d > 0) motor->forward();
    else motor->reverse();
    motor->d = d;*/
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


#define timeOut 100
#define zt 1000

unsigned char testbf[1000];

#define traceFwd(p) if(p > cp) { \
    int k = total ? (on << 8) / total : 0; \
    on = 0; total = 0; \
    if(k < 128) \
        stop = 0; \
    else \
        if(!stop) stop = cp + 100; \
    cp = p; if(p == stop) break;}

#define traceRvs(p) if(p < cp) { \
    int k = total ? (on << 8) / total : 255; \
    on = 0; total = 0; \
    if(k < 128) \
        break; \
    cp = p;} \

#define initTrace(tm) on = 0; total = 0; to = ticks + (tm)
#define initRvs(tm) on = 0; total = 0; to = ticks + (tm); r = 0
#define calcOn(mask) total++; sum += p; if(switches & mask) on++

void zero()
{
    //char buf[20];
    int to, cp, total, on, stop, sum;
    char r;

    // ��� F ������
    sendText("\nZero: F<< ");
    motors[5].reverse();
    to = ticks + 10;
    
    while(ticks < to || motors[5].rate < -20);
    motors[5].stop();
    sendText("F! Fok ");
    setMotorPos(motors + 5, -50);    
    
    // ��� B
    sendText("B>> ");
    motors[1].forward();
    to = ticks + 30;
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
    r = 0;
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
    
    /*motors[3].forward();
    motors[4].forward();
    memset(testbf, 0, sizeof(testbf));
    int mp, total = 0, on = 0, cp = 0;
    do
    {
        mp = getMotorPos(motors + 3) + getMotorPos(motors + 4);
        if(mp > cp)
        {
            int res = total ? (on << 7) / total : 0;
            if(res > 255) res = 255;
            if(res == 0) res = 1;
            testbf[cp] = res;
            cp = mp;
            total = 0;
            on = 0;
        }
        //testbf[mp] += switches & 1;
        total++;
        if(switches & 1) on++;
    }
    while(mp < 1000);
    motors[3].stop();
    motors[4].stop();*/
    
    // ��� D
    sendText("D>> ");
    motors[3].forward();
    motors[4].forward();
    initTrace(30);
    while(1)
    {
        int p = getMotorPos(motors + 3) + getMotorPos(motors + 4);
        traceFwd(p);
        calcOn(1);
        if((abs(motors[3].rate) < 20 || abs(motors[4].rate) < 20) && ticks > to) { sendText("D! "); break;}
    }
    motors[3].stop();
    motors[4].stop();
    while(abs(motors[3].rate) > 20 || abs(motors[4].rate) > 20);
    sendText("D<< ");
    motors[3].reverse();
    motors[4].reverse();
    initRvs(30);
    r = 10;
    cp = getMotorPos(motors + 3) + getMotorPos(motors + 4);
    int steps = 0;
    while(1)
    {
        int p = getMotorPos(motors + 3) + getMotorPos(motors + 4);
        //traceRvs(p);
        if(p < cp) 
        {
            int k = total ? (on << 8) / total : 255;
            if(k > 200) {if(r > 5) r--; else r = 5;}
            if(k < 50 && r <= 5) 
            {
                if(r == 5 && total) steps = sum / total;
                r--;
                if(!r) break;
            }
            on = 0; total = 0; sum = 0;
            cp = p;
        }
        calcOn(1);
        if((abs(motors[3].rate) < 20 || abs(motors[4].rate) < 20) && ticks > to) 
        { 
            sendText("D! fail "); 
            motors[3].stop();
            motors[4].stop();
            return;
        }
    }
    
    motors[3].stop();
    motors[4].stop();
    while(abs(motors[3].rate) > 20 || abs(motors[4].rate) > 20);
    setMotorPos(motors + 3, getMotorPos(motors + 3) - (steps >> 2));
    setMotorPos(motors + 4, getMotorPos(motors + 4) - (steps >> 2));
    
    sendText("Dok ");
    
    // ��� E
    /*sendText("E>> ");
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
    sendText("Eok ");*/
    
    
    //sprintf(buf, "sw:(%X) ", switches);
    //sendText(buf);
    motors[1].stop();
    motors[2].stop();
    return;
}

Command program[64] = {0};
Command * cmdPtr = 0;

void ctlLoop()
{
    char ready = 1;
    if(motorsEnabled) for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor); m++)
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
    if(ready && cmdPtr && cmdPtr->axis)
    {
        if(cmdPtr->axis == ';') cmdPtr++;
        for( ; cmdPtr->axis && cmdPtr->axis != ';'; cmdPtr++)
        {
            char a = cmdPtr->axis;
            if(a >= 'a' && a <= 'f') 
                axes[a - 'a'].moveTo(cmdPtr->pos);
                //moveMotor(motors + a - 'a', pcp->pos);
        }
    }
}