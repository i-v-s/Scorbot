#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "host_io.h"
#include <stdint.h>
// PTP A4 B5 

char text[12], * tp = text;
char num[16], * np = num;

extern uint16_t encData[6];
extern uint8_t rab[6];
extern uint16_t e[6];
extern uint8_t nozero;

int command = 0;
struct Param
{
    char axis;
    float pos;
} params[10], * pc = params;


const char * commit()
{
    char buf[20];
    sprintf(buf, "commit(%d)", command);
    sendText(buf);
    switch(command)
    {
    case 0: return 0;
    case 1:
        while(pc > params)
        {
            pc--;
            char a = pc->axis;
            if(a >= 'a' && a <= 'f')
            {
                a -= 'a';
                rab[a] = 1;
                e[a] = (int)pc->pos;
            }
        }
    }
    command = 0;
    sendText(")");
    return 0;
}

const char * save(const char * cmd)
{
    return 0;
}

const char * reset(const char * cmd)
{
    return 0;
}

const char * pos(const char * cmd)
{
    sendText("Position:");
    for(char x = 0; x < 6; x++)
    {
        char buf[12];
        sprintf(buf, " %c%d", x + 'A', encData[x]);
        sendText(buf);
    }
    return 0;
}

const char * ptp(const char * cmd)
{
    sendText("PTP(");
    command = 1;
    return 0;
}

const char * setZero(const char * cmd)
{
    sendText("ZERO: ");
    nozero = 1;
    return 0;
}

const char * onSimple(const char * cmd)
{
    char const *(* method)(const char * cmd) = 0;
    if(!strcmp(cmd, "ptp")) method = ptp;
    else if(!strcmp(cmd, "reset")) method = reset;
    else if(!strcmp(cmd, "pos")) method = pos;
    else if(!strcmp(cmd, "zero")) method = setZero;
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
        if(c >= 'a' && c <= 'f') 
        {
            char bf[20];
            sprintf(bf, "%c%f ", c + 'A' - 'a', f);
            
            pc->axis = c;
            pc->pos = f;
            pc++;
            sendText(bf);
            return 0;
        }
        if(c >= 'x' && c <= 'z') return 0;
    }
    if(!strcmp(name, "save"))
    {
        if(command) commit();
        int d = atoi(data);
        char bf[20];
        sprintf(bf, "save %d", d);
        sendText(bf);
        return 0;
    }
    return "Unknown name";
}

//char 

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