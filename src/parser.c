#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "host_io.h"
#include <stdint.h>
#include "controller.h"
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

const char * commit()
{
    char buf[20];
    sprintf(buf, "commit(%d)", command);
    sendText(buf);
    switch(command)
    {
    case 0: return 0;
    case 1:
        pc->axis = 0;
        while(pc > params)
        {
            pc--;
            pc->axis->moveTo(pc->pos);
        }
    }
    command = 0;
    sendText(")");
    return 0;
}

const char * go(const char * cmd)
{
    cmdPtr = program;
    return 0;
}

float lastSaved[7];

const char * list(const char * cmd)
{
    sendText("List:");
    for(Command * p = program; p->axis; p++)
        if(p->axis != nextCmd)
        {
            char buf[20];
            sprintf(buf, " %c%.2f", axisNames[p->axis - axes] + ('A' - 'a'), p->pos);
            sendText(buf);
        }
        else
            sendText("; ");
    return 0;
}

const char * clear(const char * cmd)
{
    sendText("Cleared");
    program->axis = 0;
    dstPtr = program;
    return 0;
}

const char * save(const char * cmd)
{
    Command * dp = dstPtr;
    if(dstPtr == program)
        for(int x = 0; x < sizeof(axes) / sizeof(Axis); x++) // Пишем всё
        {
            dstPtr->axis = axes + x;
            float pos = axes[x].getPos();
            dstPtr->pos = pos;
            lastSaved[x] = pos;
            dstPtr++;
        }
    else
        for(int x = 0; x < sizeof(axes) / sizeof(Axis); x++) // Пишем отличия
        {
            Axis * a = axes + x;
            if(a == axisF && axisH->getPos()) continue;
            float pos = a->getPos();
            if(fabs(pos - lastSaved[x]) < 1.0) continue;
            dstPtr->axis = a;
            dstPtr->pos = pos;
            lastSaved[x] = pos;
            dstPtr++;
        }
    if(dstPtr == dp)
    {
        sendText("\nNothing to save");
        return 0;
    }
    (dstPtr++)->axis = nextCmd;
    char buf[20];
    sprintf(buf, "\nProgram size: %d ", dstPtr - program);
    sendText(buf);
    dstPtr->axis = 0;
    return 0;
}

const char * stop(const char * cmd)
{
    cmdPtr = 0;
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
    axisH->moveTo(1.0);
    return 0;
}

const char * put(const char * cmd)
{
    axisH->moveTo(0.0);
    return 0;
}

const char * pos(const char * cmd)
{
    sendText("\nPos:");
    char buf[12];
    for(char x = 0; x < sizeof(axes) / sizeof(Axis); x++)
    {
        sprintf(buf, " %c%.2f", axisNames[x] + ('A' - 'a'), axes[x].getPos());
        sendText(buf);
    }
    return 0;
}

const char * ptp(const char * cmd)
{
    sendText("\nPTP(");
    command = 1;
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
    sendText(buf);
    return 0;
}

const char * onSimple(const char * cmd)
{
    char const *(* method)(const char * cmd) = 0;
    if(!strcmp(cmd, "ptp")) method = ptp;
    else if(!strcmp(cmd, "go")) method = go;
    else if(!strcmp(cmd, "list")) method = list;
    else if(!strcmp(cmd, "off")) method = off;
    else if(!strcmp(cmd, "on")) method = on;
    else if(!strcmp(cmd, "save")) method = save;
    else if(!strcmp(cmd, "clear")) method = clear;
    else if(!strcmp(cmd, "stop")) method = stop;
    else if(!strcmp(cmd, "pos")) method = pos;
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
            sendText(bf);
            return 0;
        }
    }
    return "Unknown name";
}

const char * parse(const unsigned char * data, const unsigned char * end)
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