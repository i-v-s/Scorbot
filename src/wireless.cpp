#include "host_io.h"
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "wireless.h"

/////////////////////////////////// Классы для работы с ESP8266 ////////////////////////////////////

ESPCMD * ESP::onCmd(void * obj, ESPCMD * cmd, ESPCMD * end)
{
    ESP * esp = (ESP *) obj;
    if(esp->expectResult) return cmd;
    esp->tx.log(cmd->cmd);
    esp->expectResult = cmd->result;
    return cmd + 1;
}

char * ESP::parseRX(void * obj, char * text, char * end)
{
    ESP * esp = (ESP *) obj;
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
                }
                else if(!strcmp(val, "CLOSED")) 
                {
                    esp->ready &= esp->connects &= ~mask;
                    if(!esp->ready) out.output.set(0);
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
                    //esp->tx.log("AT+CIPSEND=0,50\r\n");
                    //esp->tx.log("AT+CIPSEND=0,5\r\n");
                    if(!(esp->ready & mask))
                    {
                        esp->ready |= mask;
                        out.output.set(esp->send, esp);
                        out.pull();
                    }
                    
                }
            }
            break;
        case 9: // Приглашение к отправке данных
            {
                if(c == ' ')
                {
                    esp->busy = false;
                    out.pull();
                    //out.output.set(esp->tx.input, esp);
                    //esp->tx.source = &out;
                    //out.log("+++");
                    //out.pull();
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
    if(esp->busy || !esp->ready) return data;
    int len = esp->sendSize;
    if(!len)
    {
        len = out.length();
        int m = esp->ready;
        int id = 0;
        while(m && !(m & 1)) { m >>= 1; id++;}
        //if(len > 5) len = 5;
        
        char buf[20];
        int l = sprintf(buf, "AT+CIPSEND=%d,%d\r\n", id, len);
        esp->sendSize = len;
        esp->busy = true;
        esp->tx.push(buf, buf + l);
        return data;
    }
    
    if(end > data + len) end = data + len;
    const char * result = esp->tx.push(data, end);
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
    outDisable(); // Надо дождаться, когда придёт ответ
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
}

ESP esp;
UART &uart = esp;


void initESP()
{   
    esp.espInit(0/*aps*/, "AT+CWSAP=\"Scorbot\",\"aaa\",1,0\r\n");
}
