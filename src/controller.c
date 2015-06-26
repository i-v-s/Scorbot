#include "controller.h"
#include <stdlib.h>
#include "host_io.h"
#include "hw_config.h"
#include <string.h>
#include "kinematics.h"
#include "parser.h"

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

Axis axes[11] = 
{ //               ш/ед     ноль
    {Apos, Aref, Amove,  42.88900,  -58}, // A - ось вращения платформы, градусы
    {Bpos, Bref, Bmove,  33.02200, 1400}, // B - плечевой сустав, градусы
    {Cpos, Cref, Cmove,  33.93700,  149}, // C - локтевой сустав, градусы
    {Dpos, Dref, Dmove, -16.66666,  160}, // D - кистевой сустав, градусы
    {Epos, Eref, Emove, -16.66666,  350}, // E - вращение клешнёй, градусы
    {Fpos, Fref, Fmove,   8.00000,    0}, // F - положение клешни в разжатом состоянии, %
    {Hpos, Hpos, Hmove},                  // H - сила сжатия клешни, % (пока 0 или 100)
    {0},                                  // R - радиус, расстояние от центра платформы до захватов, мм.
    {0}                                   // X, Y, Z - координаты средней точки между захватами, мм (Z - высота)
};

const char axisNames[] = "abcdefhrxyz";

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


struct TickHandler
{ 
    void (* method) (void * obj);
    void * obj;
} tickHandlers[8], * thDst = tickHandlers;

int inStart = 0, inHalt = 0;

Command program[64] = {0};
Command * volatile cmdPtr = 0;
int doCount = 0;

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
    if(GPIOB->IDR & (1 << 14) && !cmdPtr)
    {
        inStart++;
        if(inStart >= 5)
        {
            if(!program->axis) load(0);
            cmdPtr = program;
            inStart = 0;
        }
    }
    if(GPIOB->IDR & (1 << 12) && cmdPtr)
    {
        inHalt++;
        if(inHalt >= 5)
        {
            allStop();
            inHalt = 0;            
        }
    }
    
    int sw = 0x4F00 & ~GPIOD->IDR;
    GPIOE->BSRR = sw | 0x4F000000;
    for(TickHandler * x = tickHandlers; x < thDst; x++) x->method(x->obj);
}

#ifdef __cplusplus
 }
#endif


void addTickHandler(void (* h) (void * obj), void * obj)
{
    thDst->method = h;
    thDst->obj = obj;
    thDst++;
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

bool pushCommand(Command * cmd)
{
    int len = 0;
    for(Command * t = cmd; t->axis; t++) len++;
    /*bool ap = false, bcp = false, rp = true;
    for(Command * t = cmd; t->axis; t++) 
    {
        len++;
        Axis * a = t->axis;       
        if(a == axisA) ap = true;
        else if(a == axisB || a == axisC) bcp = true;
        else if(a == axisR) 
        { 
            if(bcp) { out.log("Axes B&C cannot be used with R"); return false;}
        }
        else if(a == axisX || a == axisY)
        {
            if(bcp) { out.log("Axes B&C cannot be used with X&Y"); return false;}
            
        }
    }*/
    
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

Command * moveTo(Command * c)
{
    RXYZ pos;
    int mask = 0; // "rxyz"
    for( ; c->axis && c->axis != nextCmd; c++)
    {
        Axis * a = c->axis;
        if(a <= axisH) a->moveTo(c->pos);
        else 
        {
            int t = a - axisR;
            ((float *)&pos)[t] = c->pos;
            mask |= 1 << t;
        }
    }
    if(mask & 0x6) // xy(z)
    {
        if((mask & 0xE) != 0xE) 
        {
            RXYZ pp;
            RXYZref(&pp);
            if(!(mask & 0x2)) pos.X = pp.X;
            if(!(mask & 0x4)) pos.Y = pp.Y;
            if(!(mask & 0x8)) pos.Z = pp.Z;
        }
        XYZmoveTo(pos.X, pos.Y, pos.Z);
    } 
    else if(int m = mask & 0x9) // rz
    {
        if(m != 0x9)
        {
            RXYZ pp;
            RZref(&pp);
            if(!(m & 0x1)) pos.R = pp.R;
            if(!(m & 0x8)) pos.Z = pp.Z;
        }
        RZmoveTo(pos.R, pos.Z);
    }
    return c;
}

void runCmd() 
{               // Читаем команду
    if(Command * cp = cmdPtr)  // из программы
    {
        if(cp->axis == nextCmd) cp++;
        GPIOB->BSRR = 1 << 15; // Включаем сигнал выполнения
        cp = moveTo(cp);
        if(!cp->axis)
        {
            if(doCount) {cp = program; doCount--;}
            else cp = 0;
        }
        cmdPtr = cp;
        return;
    }
    GPIOB->BRR = 1 << 15; // Выключаем сигнал выполнения
    Command * s = cmdSrc, * d = cmdDst;
    if(d != s) // или из очереди
    {
        Command bf[16], * bp = bf;
        do
        {
            Command * t = s++;
            if(s >= commands + sizeof(commands) / sizeof(Command)) s = commands;
            if(t->axis == nextCmd) 
                break;
            else
                *(bp++) = *t;
        } while(d != s);
        bp->axis = 0;
        moveTo(bf);
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
