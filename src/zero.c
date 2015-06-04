#include "host_io.h"
#include "controller.h"
#include <stdlib.h>

#define zt 1000

unsigned char testbf[1000];

#define initFwd() int on = 0, total = 0, stop = 0, cp = 0

#define traceFwd(pos, dpos, mask) total++; if(switches & (mask)) on++; \
    if(pos > cp) \
    { \
        int k = total ? (on << 8) / total : 0; \
        on = 0; total = 0; \
        if(k < 128) \
            stop = 0; \
        else \
            if(!stop) stop = cp + dpos; \
        cp = p; if(stop && pos >= stop) break; \
    }

#define initRvs(pos) on = 0; total = 0; int r = 0, sum = 0; cp = pos

#define traceRvs(pos, mask) total++; sum += pos; if(switches & (mask)) on++; \
    if(pos < cp) \
    { \
        int k = total ? (on << 8) / total : 255; \
        on = 0; total = 0; \
        if(k > 200) r = 1; \
        if(k < 100 && r) break; \
        cp = pos; \
    }

//#define calcOn(mask) total++; sum += p; if(switches & mask) on++

/*int traceOn(int mask, Axis * a, void (* fwd)(), void (* rvs)(), void (* stop)())
{
    
    return 0;
}*/

char zeroA()
{
    sendText("A>> "); motors[0].forward();
    int to = ticks + 10;
    char r = 0;
    for(int brk = zt; brk; brk--)
    {
        if(!(switches & 8)) brk = zt;
        if(abs(motors[0].rate) < 20 && ticks > to) // Время прошло, а двигатель не крутится
        {
            if(r)
            {
                sendText("A no move ");
                motors[0].stop();
                return 0;
            } 
            else
            { // Разворачиваем
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
    setMotorPos(motors + 0, 0);
    motors[0].stop(); sendText("Aok "); return 1;
}

char zeroB()
{
    sendText("B>> ");
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
            return 0;
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
        }
    }

    motors[1].reverse();
    sendText("B<< ");
    for(int brk = zt; brk; brk--)
        if(switches & 4) brk = zt;
    
    motors[1].stop();
    setMotorPos(motors + 1, 0);
    sendText("Bok ");
    return 1;
}

char zeroC()
{
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
    return 1;
}

char zeroD()
{
    sendText("D>> ");
    motors[3].forward();
    motors[4].forward();
    int to = ticks + 20;
    initFwd();
    while(1)
    {
        int p = getMotorPos(motors + 3) + getMotorPos(motors + 4);
        traceFwd(p, 100, 1);
        if((abs(motors[3].rate) < 20 || abs(motors[4].rate) < 20) && ticks > to) { sendText("D! "); break;}
    }
    motors[3].stop();
    motors[4].stop();
    while(abs(motors[3].rate) > 20 || abs(motors[4].rate) > 20);
    sendText("D<< ");
    motors[3].reverse();
    motors[4].reverse();
    initRvs(getMotorPos(motors + 3) + getMotorPos(motors + 4));
    to = ticks + 20;
    while(1)
    {
        int p = getMotorPos(motors + 3) + getMotorPos(motors + 4);
        traceRvs(p, 1);
        if((abs(motors[3].rate) < 20 || abs(motors[4].rate) < 20) && ticks > to) 
        {motors[3].stop(); motors[4].stop(); sendText("D! fail "); return 0;}
    }
    motors[3].stop();
    motors[4].stop();
    while(abs(motors[3].rate) > 20 || abs(motors[4].rate) > 20);

    setMotorPos(motors + 3, getMotorPos(motors + 3) - (cp >> 1));
    setMotorPos(motors + 4, getMotorPos(motors + 4) - (cp >> 1));
    
    sendText("Dok "); return 1;
}

char zeroE()
{
    sendText("E>> ");
    motors[4].reverse();
    motors[3].forward();
    initFwd();
    int strt = getMotorPos(motors + 3) - getMotorPos(motors + 4);
    while(1)
    {
        int a = getMotorPos(motors + 3), b = getMotorPos(motors + 4), p = a - b;
        if(p - strt >= 1440 * axisE->scale)
        {
            motors[3].stop();
            motors[4].stop();
            while(abs(motors[3].rate) > 20 || abs(motors[4].rate) > 20);
            setMotorPos(motors + 3, getMotorPos(motors + 3) - a);
            setMotorPos(motors + 4, getMotorPos(motors + 4) - b);
            sendText("E fail ");
            return 0;
        }
        traceFwd(p, 30, 0x40);
    }
    motors[3].stop();
    motors[4].stop();
    while(abs(motors[3].rate) > 20 || abs(motors[4].rate) > 20);
    
    sendText("E<< ");
    motors[4].forward();
    motors[3].reverse();
    strt = getMotorPos(motors + 3) - getMotorPos(motors + 4);
    initRvs(strt);
    while(1)
    {
        int a = getMotorPos(motors + 3), b = getMotorPos(motors + 4), p = a - b;
        traceRvs(p, 0x40);
        if(strt - p >= 1440 * axisE->scale)
        {
            motors[3].stop();
            motors[4].stop();
            while(abs(motors[3].rate) > 20 || abs(motors[4].rate) > 20);
            setMotorPos(motors + 3, getMotorPos(motors + 3) - a);
            setMotorPos(motors + 4, getMotorPos(motors + 4) - b);
            sendText("E fail ");
            return 0;
        }
    }
    
    motors[3].stop();
    motors[4].stop();
    int d = (getMotorPos(motors + 3) + getMotorPos(motors + 4)) >> 1;
    setMotorPos(motors + 3, d);
    setMotorPos(motors + 4, d);
    
    //setMotorPos(motors + 3, 0);
    //setMotorPos(motors + 4, 0);
    sendText("Eok ");
    return 1;
}

char zeroF()
{
    sendText("\nZero: F<< ");
    motors[5].reverse();
    int to = ticks + 10;
    
    while(ticks < to || motors[5].rate < -20);
    motors[5].stop();
    sendText("F! Fok ");
    setMotorPos(motors + 5, -50);    
    return 1;
}

char zero(char noZero)
{
    //char buf[20];
    noZero &= 0x3F;
    //int to, cp, total, on, stop, sum;
    //char r;

    if(noZero & (1 << 5)) if(zeroF()) noZero &= ~(1 << 5);
    if(noZero & (1 << 1)) if(zeroB()) noZero &= ~(1 << 1);
    if(noZero & (1 << 2)) if(zeroC()) noZero &= ~(1 << 2);
    if(noZero & (1 << 0)) if(zeroA()) noZero &= ~(1 << 0);
    if(noZero & (1 << 4)) if(zeroE()) noZero &= ~(1 << 4);
    if(noZero & (1 << 3)) if(zeroD()) noZero &= ~(1 << 3);
    
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
    
    for(Motor * m = motors; m < motors + sizeof(motors) / sizeof(Motor); m++) m->stop();
    return noZero;
}
