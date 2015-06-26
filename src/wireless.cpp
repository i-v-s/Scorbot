#include "host_io.h"
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "wireless.h"
#include "controller.h"

/////////////////////////////////// ������ ��� ������ � ESP8266 ////////////////////////////////////

ESPCMD * ESP::onCmd(void * obj, ESPCMD * cmd, ESPCMD * end)
{
    ESP * esp = (ESP *) obj;
    if(esp->expectResult) return cmd;
    esp->tx.log(cmd->cmd);
    esp->expectResult = cmd->result;
    return cmd + 1;
}

int ledoff = 0;
int gloff = 0;
int pwmval = 0, pwmct = 0, pwmdst = 0;


void ESP::tickHandler(void * obj)
{
    ESP * esp = (ESP *) obj;
    if(esp->timeOut) esp->timeOut--;
    else
    {
        GPIOE->BSRR = 1 << 13;
        esp->tx.log("AT\r\n");
        esp->timeOut = 1000;
    }
    if(ledoff)
    {
        ledoff--;
        if(!ledoff) GPIOE->BRR = 1 << 13;
    }
    if(pwmct == (pwmval >> 2)) GPIOE->BSRR = 1 << 15;
    if(!pwmct)
    {
        GPIOE->BRR = 1 << 15;
        pwmct = 5;
        if(pwmval > pwmdst) pwmval--;
        else if(pwmval < pwmdst) pwmval++;
    }
    else pwmct--;    
}

//Queue<char, 1024> debug;

char * ESP::parseRX(void * obj, char * text, char * end)
{
    ESP * esp = (ESP *) obj;
    //esp->timeOut = 1000;
    char * &dst = esp->dst;
    int &state = esp->state;
    char * const var = esp->var;
    char * const val = esp->val;
    int &size = esp->size;
    while(text < end)
    {
        char c = *(text++);
        switch(state)
        {
        case 0:
            if(c == '\n') { if(dst > var) ; state = 1; dst = val; continue;}
            break;
        case 1:
            if(!esp->expectResult)
            {
                if(c >= '0' && c <= '9') { state = 4; esp->client = c - '0'; break;}
                else if(c == '+') { state = 2; dst = var; break;}
                else if(c == '>') { state = 9; break;}
            }
            if(c == '\n' || c == '\r') 
            { 
                if(dst > val) 
                {
                    *dst = 0;
                    if(!strcmp(val, "OK"))
                    {
                        esp->busy = false;
                        ledoff = 20;
                    }
                    //debug.log(val);
                    if(esp->expectResult && !strcmp(val, esp->expectResult))
                    {
                        esp->expectResult = 0; 
                        esp->cmds.pull();
                    }
                    else if(!strcmp(val, "SEND OK"))
                    {
                        esp->busy = false;
                        out.pull();
                    }
                    else if(!strcmp(val, "ERROR"))
                    {
                        //char buf[64];
                        out.log("ESP error:");
                        //if(esp->outList) out.log(esp->outList[-1]);
                        //if(esp->onDone) esp->onDone(false);
                    }
                }; 
                dst = val;
                continue;
            }
            else *(dst++) = c;
            break;
        case 2:
            if(c >= 'A' && c <= 'Z') *(dst++) = c;
            else if(c == ',') 
            { 
                *dst = 0; 
                if(!strcmp(var, "IPD")) { esp->client = 0; state = 6;}
            }
            else if(c == ':') { state = 3; *dst = 0; dst = val;}
            break;
        case 3:
            if(c != '\r' && c != '\n') *(dst++) = c;
            else if(dst > val) 
            {
                dst = 0;
                dst = var;
                state = 0;
                if(!strcmp(var, "CWLAP")) esp->onCWLAP(val);
                
            }
            break;
        case 4:
            if(c >= '0' && c <= '9') esp->client = esp->client * 10 + (c - '0');
            else if(c == ','){ dst = val; state = 5;}
            break;
        case 5:
            if(c >= 'A' && c <= 'Z') *(dst++) = c;
            else if(c == '\n' || c == '\r')
            {
                *dst = 0;
                int mask = 1 << esp->client;
                if(!strcmp(val, "CONNECT")) 
                {
                    esp->connects |= mask;
                    esp->ready &= ~mask;

                    /*char buf[64];
                    sprintf(buf, "CONNECT(r:%d)", esp->ready);
                    debug.log(buf);*/

                    out.output.set(esp->send, esp);
                    esp->tx.source = &out;
                    out.pull();

                    pwmval = 20;
                    pwmdst = 20;
                }
                else if(!strcmp(val, "CLOSED")) 
                {
                    esp->ready &= esp->connects &= ~mask;
    
                    /*char buf[64];
                    sprintf(buf, "CLOSED(r:%d)", esp->ready);
                    debug.log(buf);*/
    
                    if(!esp->connects)
                    {
                        out.output.set(0);
                        esp->tx.source = 0;
                        pwmdst = 0;
                    }
                }
                state = 0;
            }
            break;
        case 6:
            if(c >= '0' && c <= '9') esp->client = esp->client * 10 + (c - '0');
            else if(c == ','){ state = 7; size = 0;}
            break;
        case 7:
            if(c != ':')
            {
                if(c >= '0' && c <= '9') size = size * 10 + (c - '0');
                break;
            }
            state = 8;
        case 8:
            {
                char * e = text + size;
                if(e > end) e = end;
                int mask = 1 << esp->client;
                if(esp->ready & mask)
                {                    
                    const char * msg = parse(text, e);
                    if(msg) out.log(msg);
                }
                size -= e - text;
                text = e;
                if(!size)
                {
                    state = 0;
                    esp->ready |= mask;
                }
            }
            break;
        case 9: // ����������� � �������� ������
            {
                if(c == ' ')
                {
                    esp->busy = false;
                    out.pull();
                }
                state = 0;
            }
        }
    }
    return end;
}

const char * ESP::send(void * obj, const char * data, const char * end)
{
    ESP * esp = (ESP *)obj;

    /*char buf[16];
    sprintf(buf, (esp->busy ? "i:%d " : "b:%d "), end - data);
    debug.log(buf);*/
    
    if(esp->busy || !esp->connects) return data;
    int len = esp->sendSize;
    if(!len)
    {
        len = out.length();
        int m = esp->connects;
        int id = 0;
        while(m && !(m & 1)) { m >>= 1; id++;}
        
        char buf[20];
        int l = sprintf(buf, "AT+CIPSEND=%d,%d\r\n", id, len);
        //debug.log(buf);
        esp->sendSize = len;
        esp->busy = true;
        esp->tx.push(buf, buf + l);
        return data;
    }
    
    if(end > data + len) end = data + len;
    const char * result = esp->tx.push(data, end);
    
    /*sprintf(buf, "sent:%d ", result - data);
    debug.log(buf);*/
    
    esp->sendSize = len - (result - data);
    esp->busy = !esp->sendSize;
    return result;
}

void ESP::onCWLAP(char * val)
{
    int t = 0;
    const char * k = 0;
    while(char c = *(val++)) if(!k || c == '"') switch(c)
    {
    case ',': t++; break;
    case '"': if(k)
        {
            if(aps)
            {
                val[-1] = 0;
                for(const AP * x = aps; x->name; x++) if(!strcmp(x->name, k))
                {
                    currentAP = x;
                    break;
                }
            }
            k = 0;
        }
        else k = val;
        break;
    }
}

void ESP::echo(bool e, void (* done)(bool Ok))
{
    onDone = done;
    tx.log(e ? "ATE1\r\n" : "ATE0\r\n");
}

/*void ESP::query(const char * query, void (* done)(const char * res))
{
    char buf[32];
    sprintf(buf, "AT+%s?\r\n", query);
    while(uartSend(buf));
    outDisable(); // ���� ���������, ����� ����� �����
    for(Query * x = queries; x < queries + sizeof(queries) / sizeof(Query); x++)
        if(!x->var) { x->var = query; x->done = done; break;}
}*/

void ESP::set(const char * var, const char * value, void (* done)(bool Ok))
{
    char buf[64];
    sprintf(buf, "AT+%s=%s\r\n", var, value);
    tx.log(buf);
    //outDisable();    
    onDone = done;
}

void ESP::espInit(const AP * list, const char * ap)
{
    GPIOB->BRR = GPIO_Pin_1;//GPIO_Pin_13; //move
    for(int x = 100000; x--;);
    GPIOB->BSRR = GPIO_Pin_1;
    //for(int x = 100000; x--;);

    expectResult = "invalid";
    //cmds.push(ESPCMD("AT+RST\r\n", "invalid"));
    cmds.push(ESPCMD("ATE0\r\n"));
    aps = list;
    if(list)
    {
        cmds.push(ESPCMD("AT+CWMODE=1\r\n"));
        cmds.push(ESPCMD("AT+CWLAP\r\n"));
    }
    else
    {
        cmds.push(ESPCMD("AT+CWMODE=2\r\n"));
        cmds.push(ESPCMD(ap));
    }
    cmds.push(ESPCMD("AT+CIPMUX=1\r\n"));
    cmds.push(ESPCMD("AT+CIPSERVER=1,8080\r\n"));
    cmds.push(ESPCMD("AT+CIFSR\r\n"));
    addTickHandler(tickHandler, this);
}

ESP esp;
UART &uart = esp;


void initESP()
{   
    esp.espInit(0/*aps*/, "AT+CWSAP=\"Scorbot\",\"aaa\",1,0\r\n");
}
