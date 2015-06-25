#include <stdint.h>
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "usb_regs.h"
#include "usb_pwr.h"
#include "host_io.h"

Out out;

extern "C" {

size_t __write(int Handle, const unsigned char *Buf, size_t Bufsize)
{
    //stdout
    return out.push((const char *) Buf, (const char *) (Buf + Bufsize)) - (const char *)Buf;
}

}

extern volatile uint32_t packet_sent;

void EP1_IN_Callback (void)
{   
    out.pull();
    
    //outState = 1;
}

char * usbReceiver(void * obj, char * data, int size)
{
    if(size > VIRTUAL_COM_PORT_DATA_SIZE) size = VIRTUAL_COM_PORT_DATA_SIZE;
    UserToPMABufferCopy((unsigned char *)data, ENDP1_TXADDR, size);
    SetEPTxCount(ENDP1, size);
    SetEPTxValid(ENDP1);
    return data + size;
}


/*void onConfigured()
{
    if(!outState)
    { 
        char * s = out.src;
        if(s != out.dst)
            EP1_IN_Callback();
    }
}

void enableOut()
{
    outState = 0;
    char * s = out.src;
    if(s != out.dst)
        EP1_IN_Callback();    
}*/



/*void sendTextOld(const char * text)
{
    int l = strlen(text);
    while(l)
    { // Запись в кольцевой буфер
        char * s = out.src, * d = out.dst;
        int empty = (s - d - 1) & (sizeof(out.data) - 1); // Сколько всего места в буфере осталось
        if(!empty) break; // Некуда писать
        int left = out.data + sizeof(out.data) - d; // Сколько до конца отрезка
        if(left > empty) left = empty;
        if(left > l) left = l;
        memcpy(d, text, left);
        text += left;
        l -= left;
        d += left;
        if(d >= out.data + sizeof(out.data)) d = out.data;
        out.dst = d;
    }
    if(!outState && bDeviceState == CONFIGURED) // Пнуть отправку, если не работает
        EP1_IN_Callback();
}*/

