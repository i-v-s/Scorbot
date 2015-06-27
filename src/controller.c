#include "controller.h"
#include <stdlib.h>
#include <stdio.h>
#include "host_io.h"
#include "hw_config.h"
#include <string.h>
#include "kinematics.h"
#include "parser.h"

volatile int ticks = 0;

Motor motors[6] = {0};

float Apos() { return (motors[0].getPos() - axisA->zero) / axisA->scale;}
float Bpos() { return (-motors[1].getPos() - axisB->zero) / axisB->scale;}
float Cpos() { return (motors[2].getPos() - axisC->zero) / axisC->scale;}
float Dpos() { return (motors[3].getPos() + motors[4].getPos() - axisD->zero) / axisD->scale;}
float Epos() { return (motors[3].getPos() - motors[4].getPos() - axisE->zero) / axisD->scale;}
float Fpos() { return (motors[5].getPos() - axisF->zero) / axisF->scale;}

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
{ //               �/��     ����
    {Apos, Aref, Amove,  42.88900,  -58}, // A - ��� �������� ���������, �������
    {Bpos, Bref, Bmove,  33.02200, 1400}, // B - �������� ������, �������
    {Cpos, Cref, Cmove,  33.93700,  149}, // C - �������� ������, �������
    {Dpos, Dref, Dmove, -16.66666,  160}, // D - �������� ������, �������
    {Epos, Eref, Emove, -16.66666,  350}, // E - �������� ������, �������
    {Fpos, Fref, Fmove,   8.00000,    0}, // F - ��������� ������ � �������� ���������, %
    {Hpos, Hpos, Hmove},                  // H - ���� ������ ������, % (���� 0 ��� 100)
    {0},                                  // R - ������, ���������� �� ������ ��������� �� ��������, ��.
    {0}                                   // X, Y, Z - ���������� ������� ����� ����� ���������, �� (Z - ������)
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
int delay = 0;

#ifndef _TEST_
#ifdef __cplusplus
 extern "C" {
#endif

void SysTick_Handler(void)
{
    ticks++;
    for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor); m++)
    {
        int pos = m->getPos();
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
#endif

void addTickHandler(void (* h) (void * obj), void * obj)
{
    thDst->method = h;
    thDst->obj = obj;
    thDst++;
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

void onBlock(Motor * m)
{
    char buf[32];
    sprintf(buf, "\nCan not to move %d ", m - motors);
    out.log(buf);
    int pos = m->getPos(), ref = (pos < m->ref) ? (pos - 50) : (pos + 50);
    allStop();
    m->ref = ref;
}

int Motor::control(int l)
{
    int d = ref - getPos();
    bool f = d >  prec;
    bool r = d < -prec;
    
    if(f || r)
    {
        time = timeOut;
        if(l <= left()) // ��������
        {
            if(f)
            {
                forward();
                if(state < 10000) state++;
                else if(rate < 20) onBlock(this);
            }
            else
            {
                reverse();
                if(state > -10000) state--;
                else if(rate > -20) onBlock(this);                
            }
            return time;
        }
    } else if(time > 0) time--;
    stop();
    state -= state >> 3;
    return time;
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

bool pushCommand(int type, Command * cmd)
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
    int empty = (s - d - 2) & (sizeof(commands) / sizeof(Command) - 1); // ������� ����� ����� � ������ ��������
    if(empty < len) return 0;
    while(len)
    { // ������ � ��������� �����
        int left = commands + sizeof(commands) / sizeof(Command) - d; // ������� �� ����� �������
        if(left > len) left = len;
        memcpy(d, cmd, left * sizeof(Command));
        cmd += left;
        len -= left;
        d += left;
        if(d >= commands + sizeof(commands) / sizeof(Command)) d = commands;
    }
    d->axis = (Axis *)type;
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
        m->ref = m->getPos();
    }
    cmdSrc = commands;
    cmdDst = commands;
    cmdPtr = 0;
}

Command * moveTo(Command * c)
{
    RXYZ pos;
    int refs[sizeof(motors) / sizeof(Motor)], * rp = refs + sizeof(motors) / sizeof(Motor) - 1;
    for(Motor * m = motors + sizeof(motors) / sizeof(Motor) - 1; m >= motors; m--) *(rp--) = m->ref;
    
    int mask = 0; // "rxyz"
    for( ; c->axis > endCmd; c++)
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
        if(!XYZmoveTo(pos.X, pos.Y, pos.Z)) out.log("\nWrong XYZ ");
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
        if(!RZmoveTo(pos.R, pos.Z)) out.log("\nWrong RZ ");
    }
    
    rp = refs + sizeof(motors) / sizeof(Motor) - 1;
    for(Motor * m = motors + sizeof(motors) / sizeof(Motor) - 1; m >= motors; m--) 
        m->rDelta = LEFT_MAX / (m->ref - *(rp--));
    return c;
}

void runCmd() 
{               // ������ �������
    if(Command * cp = cmdPtr)  // �� ���������
    {
        if(cp->axis && cp->axis < endCmd) cp++;
        GPIOB->BSRR = 1 << 15; // �������� ������ ����������
        cp = moveTo(cp);
        if(!cp->axis)
        {
            if(doCount) {cp = program; doCount--;}
            else cp = 0;
        }
        cmdPtr = cp;
        return;
    }
    GPIOB->BRR = 1 << 15; // ��������� ������ ����������
    Command * s = cmdSrc, * d = cmdDst;
    if(d != s) // ��� �� �������
    {
        Command bf[16], * bp = bf;
        do
        {
            Command * t = s++;
            if(s >= commands + sizeof(commands) / sizeof(Command)) s = commands;
            if(t->axis < endCmd)
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
    
    if(motorsEnabled)
    {
        int leftSum = 0, count = 0;//minLeft = LEFT_MAX
        //Motor * minMotor = 0;
        for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor) - 1; m++) if(m->time)
        {
            int left = m->left();
            leftSum += left;
            //if(left < minLeft){ minLeft = left; minMotor = m;}
            count++;
        }
        if(count) leftSum /= count;
            
        for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor) - 1; m++)
            //if(m->left() < leftSum) m->stop();
            notReady |= m->control(leftSum - LEFT_PREC);
    }
    
    if(hold)
        notReady |= ctlHold(motors + 5);
    else
        if(motorsEnabled) notReady |= motors[5].control(0);

    if(motorsEnabled && !notReady) runCmd();
}
