#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "host_io.h"
#include <stdint.h>
#include "controller.h"
#include "kinematics.h"
// PTP A4 B5 

char text[12], * tp = text;
char num[16], * np = num;

extern uint16_t encData[6];
extern uint8_t rab[6];
extern uint16_t e[6];
extern uint8_t noZero;

int command = 0;
Command params[10], * pc = params;

Command * dstPtr = program;

#pragma location=0x08002000 
__no_init volatile unsigned short flashBuf[1024];
#define flashBufPage 0x08002000

const char * commit()
{
    char buf[20];
    sprintf(buf, "commit(%d)", command);
    out.log(buf);
    switch(command)
    {
    case 0: return 0;
    case 1:
        pc->axis = 0;
        pc = params;
        if(!pushCommand(params)) out.log("\nUnable to send command");
        break;
    case 5:
        pc->axis = 0;
        pc = params;
        moveTo(params);
        break;
    }
    command = 0;
    out.log(")");
    return 0;
}

const char * go(const char * cmd)
{
    cmdPtr = program;
    return 0;
}

const char * list(const char * cmd)
{
    out.log("\nList:");
    for(Command * p = program; p->axis; p++)
        if(p->axis != nextCmd)
        {
            char buf[20];
            sprintf(buf, " %c%.1f", axisNames[p->axis - axes] + ('A' - 'a'), p->pos);
            out.log(buf);
        }
        else
            out.log("; ");
    return 0;
}

const char * clear(const char * cmd)
{
    out.log("Cleared");
    program->axis = 0;
    dstPtr = program;
    return 0;
}

const char * pop(const char * cmd)
{
    Command * dp = program;
    while(dp->axis) dp++; // *dp = 0;
    dp--; // *dp = ;
    while(dp-- > program) if(dp->axis == nextCmd)
    {
        dp++;
        dp->axis = 0;
        dstPtr = dp;
        break;
    }
    char buf[20];
    sprintf(buf, "\nProgram size: %d ", dstPtr - program);
    out.log(buf);
    return 0;
}

const char * save(const char * cmd)
{
    Command * dp = dstPtr;
    if(dstPtr == program)
        for(int x = 0; x < 7; x++) // Пишем всё
        {
            Axis * a = axes + x;
            dstPtr->axis = a;
            float pos = a->getPos(), ref = a->getRef();
            if(fabs(pos - ref) <= 2.0) pos = ref;
            dstPtr->pos = pos;
            dstPtr++;
        }
    else
        for(int x = 0; x < 7; x++) // Пишем отличия
        {
            Axis * a = axes + x;
            if(a == axisF && axisH->getPos()) continue;
            
            float pos = a->getPos(), ref = a->getRef();
            if(fabs(pos - ref) <= 2.0) pos = ref;
            for(Command * p = dstPtr; p >= program; p--) if(p->axis == a)
            {
                if(fabs(p->pos - pos) > 2.0) // Отличается - надо записать отличия
                {
                    dstPtr->axis = a;
                    dstPtr->pos = pos;
                    dstPtr++;                    
                }
                break;
            }
        }
    if(dstPtr == dp)
    {
        out.log("\nNothing to save");
        return 0;
    }
    (dstPtr++)->axis = nextCmd;
    char buf[20];
    sprintf(buf, "\nProgram size: %d ", dstPtr - program);
    out.log(buf);
    dstPtr->axis = 0;
    return 0;
}

const char * load(const char * cmd)
{
    Command * p = program;
    for(Command * c = (Command *)flashBuf; c->axis; c++)
        *(p++) = *c;
    p->axis = 0;
    dstPtr = p;
    char buf[20];
    sprintf(buf, "\nProgram size: %d ", p - program);
    out.log(buf);
    return 0;
}

const char * flash(const char * cmd)
{
    if(FLASH->CR & FLASH_CR_LOCK)
    {
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;
    }
    
//void Internal_Flash_Erase(unsigned int pageAddress) {
    while (FLASH->SR & FLASH_SR_BSY);
    if (FLASH->SR & FLASH_SR_EOP) {
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = flashBufPage;
    FLASH->CR |= FLASH_CR_STRT;
    while (!(FLASH->SR & FLASH_SR_EOP));
    FLASH->SR = FLASH_SR_EOP;
    FLASH->CR &= ~FLASH_CR_PER;
//} */   
//data - указатель на записываемые данные
//address - адрес во flash
//count - количество записываемых байт, должно быть кратно 2
//void Internal_Flash_Write(unsigned char* data, unsigned int address, unsigned int count) {

    while (FLASH->SR & FLASH_SR_BSY);
    if (FLASH->SR & FLASH_SR_EOP) {
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR |= FLASH_CR_PG;

    Command * p = program;
    for(; p->axis; p++);
    p++;
    volatile unsigned short * fb = flashBuf;
    for(unsigned short * pp = (unsigned short *)program; pp < (unsigned short *)p; pp++)
    {
        *(fb++) = *pp;
        while (!(FLASH->SR & FLASH_SR_EOP));
        FLASH->SR = FLASH_SR_EOP;
    }
    FLASH->CR &= ~(FLASH_CR_PG);
    return 0;
}

const char * stop(const char * cmd)
{
    allStop();
    return 0;
}

const char * off(const char * cmd)
{
    motorsOff();
    return 0;
}

const char * on(const char * cmd)
{
    motorsOn();
    return 0;
}

const char * get(const char * cmd)
{
    Command c[2] = {{axisH, 100.0}, {0}};
    pushCommand(c);
    return 0;
}

const char * put(const char * cmd)
{
    Command c[2] = {{axisH, 0.0}, {0}};
    pushCommand(c);
    return 0;
}

const char * pos(const char * cmd)
{
    out.log("\nPos:");
    char buf[64];
    for(char x = 0; x < 7; x++)
    {
        sprintf(buf, " %c%.1f", axisNames[x] + ('A' - 'a'), axes[x].getPos());
        out.log(buf);
    }
    RXYZ pos;
    RXYZpos(&pos);
    sprintf(buf, " R%.1f X%.1f Y%.1f Z%.1f", pos.R, pos.X, pos.Y, pos.Z);
    out.log(buf);
    return 0;
}

const char * ptp(const char * cmd)
{
    out.log("\nPTP(");
    command = 1;
    return 0;
}

const char * ref(const char * cmd)
{
    out.log("\nREF(");
    command = 5;
    return 0;
}

const char * setZero(const char * cmd)
{
    noZero = 0x3F;
    return 0;
}

const char * getSwitch(const char * cmd)
{
    char buf[20];
    sprintf(buf, "\nSw: %X ", switches);
    out.log(buf);
    return 0;
}

const char * onSimple(const char * cmd)
{
    char const *(* method)(const char * cmd) = 0;
    if(!strcmp(cmd, "ptp")) method = ptp;
    else if(!strcmp(cmd, "ref")) method = ref;
    else if(!strcmp(cmd, "go")) method = go;
    else if(!strcmp(cmd, "list")) method = list;
    else if(!strcmp(cmd, "off")) method = off;
    else if(!strcmp(cmd, "on")) method = on;
    else if(!strcmp(cmd, "save")) method = save;
    else if(!strcmp(cmd, "flash")) method = flash;
    else if(!strcmp(cmd, "load")) method = load;
    else if(!strcmp(cmd, "clear")) method = clear;
    else if(!strcmp(cmd, "stop")) method = stop;
    else if(!strcmp(cmd, "pos")) method = pos;
    else if(!strcmp(cmd, "pop")) method = pop;
    else if(!strcmp(cmd, "get")) method = get;
    else if(!strcmp(cmd, "put")) method = put;
    else if(!strcmp(cmd, "zero")) method = setZero;
    else if(!strcmp(cmd, "sw")) method = getSwitch;
    if(method)
    {
        if(command) commit();
        tp = text;
        return method(cmd);
    }
    return 0;
}

const char * onComplex(const char * name, const char * data)
{
    char c = name[0];
    if(c && !name[1])
    {
        float f = atof(data);
        for(const char * t = axisNames; *t; t++) if(c == *t)
        {
            char bf[20];
            sprintf(bf, "%c%.2f ", c + 'A' - 'a', f);
            
            pc->axis = axes + (t - axisNames);
            pc->pos = f;
            pc++;
            out.log(bf);
            return 0;
        }
    }
    if(!strcmp(name, "count"))
    {
        doCount = atoi(data);
        return 0;
    }
    return "Unknown name";
}

const char * parse(const char * data, const char * end)
{    
    while(data < end)
    {
        unsigned char c = *(data++);
        if(c >= 'A' && c <= 'Z') c -= 'A' - 'a'; 
        if(c >= 'a' && c <= 'z')
        {
            if(tp >= text + sizeof(text) - 1) { tp = text; return "Too long name";}
            *(tp++) = c;
            continue;
        }
        if(tp != text)
        {
            *tp = 0;
            const char * e = onSimple(text);
            if(e) return e;
        }
        
        if((c >= '0' && c <= '9') || c == '-' || c == '.')
        {
            if(tp == text) return "Name required";
            if(np >= num + sizeof(num) - 1) {np = num; return "Too long number";}
            *(np++) = c;
            continue;
        }
        if(tp != text && np != num)
        {
            tp = text;
            *np = 0;
            np = num;
            const char * e = onComplex(text, num);
            if(e) return e;
        }

        if(c == ';') 
        {
            if(tp != text && np == num) {tp = text; return "Argument required";}
            if(command) commit();
        }
    }
    return 0;
}