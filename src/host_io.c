#include <stdint.h>
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "usb_regs.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include <string.h>
#include "host_io.h"

Circular out;

void Circular::log(const char * text)
{
    int size = strlen(text), f;
    char * s = src, * d, * nd;
    do
    { // Попытка выделить место в буфере для записи
        d = (char *)__LDREX((unsigned long *)&newDst);
        f = d - dst; // Если значения равны - значит мы первые
        int empty = (s - d - 1) & (sizeof(data) - 1);
        if(!empty) 
        { // Некуда писать
            __CLREX();
            pop();
            return; 
        } 
        if(size > empty) size = empty;
        nd = d + size;
        if(nd > data + sizeof(data)) nd -= sizeof(data);
    } while(__STREX((unsigned long)nd, (unsigned long *)&newDst));
    int left = data + sizeof(data) - d; // Сколько до конца отрезка
    if(left >= size) // Производим запись
        memcpy(d, text, size);
    else
    {
        memcpy(d, text, left);
        memcpy(data, text + left, size - left);
    }
    if(f) return;
    do 
    { // Попытка обновить dst
        nd = (char *)__LDREX((unsigned long *)&newDst);
        dst = nd;
    } while(__STREX((unsigned long)nd, (unsigned long *)&newDst));
    pop();
}

void Circular::pop()
{ // Отправка из кольцевого буфера
    if(!receiver) return;
    static char work = 0;
    if(work) return;
    work = 1;
    
    char * s = src, * d = dst;
    int size = (d - s) & (sizeof(data) - 1); // Сколько всего надо отправить
    if(!size)
    {
        //outState = 0;
        work = 0;
        return;
    }
    int left = data + sizeof(data) - s;
    if(size > left) size = left;
    s = receiver(receiverObj, s, size);
    if(s >= data + sizeof(data)) s = data;
    src = s;
    //outState = 1;
    work = 0;
}

extern volatile uint32_t packet_sent;

void EP1_IN_Callback (void)
{   
    out.pop();
    
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

////// Тестирование на устойчивость к прерываниям /////////////////////////////////////////

class CircularTest: private Circular
{
private:
    friend void testSysTick_Handler();
    char testBuf[64], * tb;
    int volatile testSendCount;
    int volatile irq;
    void * lastStack;
    char * volatile _src, * volatile _dst, * volatile _newOutDst;
public:    
    CircularTest(): tb(testBuf), testSendCount(0), irq(0), lastStack(0) {};
    const char * run();
    static char * testReceiver(void * obj, char * start, int size);
};

CircularTest ctest;

void testSysTick_Handler(void)
{
    if(ctest.testSendCount > 0) ctest.testSendCount--;
    else SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    ctest.log("(IRQ)");
    ctest.irq++;
    ctest._newOutDst = ctest.newDst;
    ctest._dst = ctest.dst;
    ctest._src = ctest.src;
    void * t[10], ** tt = t;
    ctest.lastStack = tt[18];
}


char * CircularTest::testReceiver(void * obj, char * start, int size)
{
    memcpy(ctest.tb, start, size);
    ctest.tb += size;
    *ctest.tb = 0;
    return start + size;
}

//#define __LDREX(a) *(a)
//#define __STREX(b, a) ((*(a) = b) & 0)   

const char * CircularTest::run()
{
    // Разгоняем SysTick до максимума
    //SYSCFG->CFGR1 |= SYSCFG_CFGR1_MEM_MODE;
    //*(void * *)0x3C = (void *)IRQTest;
    const char * error = 0;
    int abc, def;
    int X = 768;
    for(X = 0; X < 10000; X++)
    //for(X = 768; X < 769; X++)
    {
        testSendCount = X >> 11;
        irq = 0;
        SysTick->LOAD  = 1 + (X & 0x7FF);
        NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Systick Interrupt */
        SysTick->VAL   = 0;
        SysTick->CTRL  = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

        log("(ABC)");
        log("(DEF)");
        
        tb = testBuf;
        receiver = &testReceiver;
        pop();
        while(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk);
        pop();
        pop();
        abc = 1; def = 1;
        for(char * x = testBuf; *x; x++)
        {
            if(*x == '(')
            {
                char * t = x + 1;
                while(*x && *x != ')') x++;
                if(*x != ')') { error = "no ')'"; break;}
                char bbb[32];
                if(x - t > sizeof(bbb) - 1) { error = "too long"; break;}
                memcpy(bbb, t, x - t);
                bbb[x - t] = 0;
                if(!strcmp("ABC", bbb)) abc--;
                else if(!strcmp("DEF", bbb)) def--;
                else if(!strcmp("IRQ", bbb)) irq--;
                else {error = "unknown string"; break;}
            }
        }
        if(error) break;
        if(abc || def || irq) {error = "non zero"; break;}
    }
    return error;
}

const char * testOut()
{
    return ctest.run();
}
