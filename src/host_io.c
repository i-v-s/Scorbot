#include <stdint.h>
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "usb_regs.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include <string.h>

char out[128];
char * volatile outSrc = out, * volatile outDst = out;
char outState = 1;

// Пишем в outDst
// Читаем из outSrc

extern volatile uint32_t packet_sent;

// VIRTUAL_COM_PORT_DATA_SIZE - 1
/*void sendAll()
{
    while(!packet_sent);
    CDC_Send_DATA(data, dp - data);
    dp = data;
}*/

volatile char outwrk = 0;

void EP1_IN_Callback (void)
{ // Отправка из кольцевого буфера
    if(outwrk) return;
    outwrk = 1;
    
    char * s = outSrc, * d = outDst;
    int size = (d - s) & (sizeof(out) - 1); // Сколько всего надо отправить
    if(!size)
    {
        outState = 0;
        outwrk = 0;
        return;
    }
    if(size > VIRTUAL_COM_PORT_DATA_SIZE) size = VIRTUAL_COM_PORT_DATA_SIZE;
    if(size > out + sizeof(out) - s) size = out + sizeof(out) - s;
    
    UserToPMABufferCopy((unsigned char *)s, ENDP1_TXADDR, size);
    SetEPTxCount(ENDP1, size);
    SetEPTxValid(ENDP1);
    s += size;
    if(s >= out + sizeof(out)) s = out;
    outSrc = s;
    outState = 1;
    outwrk = 0;
}

void onConfigured()
{
    if(!outState)
    { 
        char * s = outSrc;
        if(s != outDst)
            EP1_IN_Callback();
    }
}

void enableOut()
{
    outState = 0;
    char * s = outSrc;
    if(s != outDst)
        EP1_IN_Callback();    
}

void sendText(const char * text)
{
    int l = strlen(text);
    while(l)
    { // Запись в кольцевой буфер
        char * s = outSrc, * d = outDst;
        int empty = (s - d - 1) & (sizeof(out) - 1); // Сколько всего места в буфере осталось
        if(!empty) break; // Некуда писать
        int left = out + sizeof(out) - d; // Сколько до конца отрезка
        if(left > empty) left = empty;
        if(left > l) left = l;
        memcpy(d, text, left);
        text += left;
        l -= left;
        d += left;
        if(d >= out + sizeof(out)) d = out;
        outDst = d;
    }
    if(!outState && bDeviceState == CONFIGURED) // Пнуть отправку, если не работает
        EP1_IN_Callback();
}
