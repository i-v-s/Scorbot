#include "mx_gpio.h"
#include "host_io.h"
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "wireless.h"
//#include <vector>

//////////////////////////////////// Класс для работы с UART ///////////////////////////////////////

UART::UART(): txSrc(txData), txDst(txData), nextOutSrc(txData), txStop(0)
{
    rxPos = rxData;    
}

void UART::uartInit(USART_TypeDef * USART, DMA_TypeDef * DMA, DMA_Channel_TypeDef * DMA_RX, DMA_Channel_TypeDef * DMA_TX)
{
    this->USART = USART;
    this->DMA = DMA;
    this->DMA_RX = DMA_RX;
    this->DMA_TX = DMA_TX;
    DMA_RX->CNDTR = sizeof(rxData);
    DMA_RX->CMAR = (uint32_t) rxData;
    DMA_RX->CPAR = (uint32_t) &USART->RDR;
    DMA_RX->CCR = DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_PSIZE_0 | DMA_CCR_EN | DMA_CCR_CIRC
        | DMA_CCR_TEIE | DMA_CCR_HTIE | DMA_CCR_TCIE;

    DMA_TX->CNDTR = 0;
    DMA_TX->CMAR = (uint32_t) txData;
    DMA_TX->CPAR = (uint32_t) &USART->TDR;
    DMA_TX->CCR = DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_PSIZE_0 | DMA_CCR_DIR
        | DMA_CCR_TEIE | DMA_CCR_TCIE;
    
    NVIC->ISER[0] = (1 << 14) | (1 << 15);
    NVIC->ISER[1] = (1 << 5);
    
    //USART->BRR = 0x0753; // 38400
    //USART->BRR = 0x1D4C; //  9600
    //USART->BRR = 0x04E2; // 57600
    USART->BRR = 0x271;  // 115200
    
    USART->RTOR = 20;
    USART->CR1 = USART_CR1_UE | USART_CR1_RTOIE;
    USART->CR2 = USART_CR2_RTOEN;
    USART->CR3 = USART_CR3_DMAR | USART_CR3_DMAT;
    USART->CR1 |= USART_CR1_TE | USART_CR1_RE;    
}

void UART::txComplete()
{
    DMA_TX->CCR &= ~DMA_CCR_EN; // Выключить канал
    //if(!1) return; // Если отправка запрещена
    
    char * s = nextOutSrc, * d = txStop ? txStop : txDst;
    txSrc = s;
    int size = (d - s) & (sizeof(txData) - 1); // Сколько всего надо отправить
    if(!size)
    {
        //outState = 0;
        //outwrk = 0;
        
        return;
    }
    if(size > txData + sizeof(txData) - s) size = txData + sizeof(txData) - s;
    
    /*char bf[100], *bp = bf, *tp = s;
    *(bp++) = '[';
    for(int x = size; x--; ) *(bp++) = *(tp++);
    *(bp++) = ']';
    *(bp++) = 0;
    txData.log(bf);*/
    
    DMA_TX->CMAR = (uint32_t)s;
    DMA_TX->CNDTR = size;
    DMA_TX->CCR |= DMA_CCR_EN; // Включить отправку
    s += size;
    if(s >= txData + sizeof(txData)) s = txData;
    nextOutSrc = s;
}

void UART::rxNotEmpty()
{
    static bool work = false;
    if(work) return;
    work = true;
    char * p = rxData + sizeof(rxData) - DMA_RX->CNDTR;
    if(p > rxPos)
    {
        onRX(rxPos, p);
        rxPos = p;
    }
    else if(p < rxPos)
    {
        onRX(rxPos, rxData + sizeof(rxData));
        onRX(rxData, p);
        rxPos = p;    
    }
    work = false;
}

int UART::uartSend(const char * text, int size)
{
    while(size)
    { // Запись в кольцевой буфер
        char * s = txSrc, * d = txDst;
        int empty = (s - d - 1) & (sizeof(txData) - 1); // Сколько всего места в буфере осталось
        if(!empty) break; // Некуда писать
        int left = txData + sizeof(txData) - d; // Сколько до конца отрезка
        if(left > empty) left = empty;
        if(left > size) left = size;
        memcpy(d, text, left);
        text += left;
        size -= left;
        d += left;
        if(d >= txData + sizeof(txData)) d = txData;
        txDst = d;
    }
    if(!(DMA_TX->CCR & DMA_CCR_EN)) txComplete();
    return size;
}

/////////////////////////////////// Классы для работы с ESP8266 ////////////////////////////////////

/*void * ESP::parseDef(ESP * th, char * &t, char * e) 
{ 
    while(t < e) if(*(t++) == '\n') return (void *)&parseLine;
    return (void *)&parseDef;
}

void * ESP::parseLine(ESP * th, char * &t, char * e)
{
    char c = *t;
    if(c >= 'A' && c <= 'Z') {dst = val; return (void *)parseRes;}
    if(c == '+') {t++; dst = var; return (void *)parseVar;}
    return (void *)&parseLine;
}

void * ESP::parseVar(ESP * th, char * &t, char * e) 
{ 
    while(t < e)
    {
        char c = *t++;        
        if(c == ':') return (void *)&parseValue;
        if(c >= 'A' && c <= 'Z') *(dst++) = c;
    }
    return (void *)&parseDef;
}*/


void ESP::onRX(const char * text, const char * end)
{
    while(text < end)
    {
        char c = *(text++);
        switch(state)
        {
        case 0:
            if(c == '\n') { if(dst > var) ; state = 1; dst = val; continue;}
            break;
        case 1: 
            if(c >= 'A' && c <= 'Z') *(dst++) = c;
            else if(c >= '0' && c <= '9') { state = 4; client = c - '0'; break;}
            else if(c == '+') { state = 2; dst = var;}
            else if(c == '\n' || c == '\r') 
            { 
                if(dst > val) 
                {
                    *dst = 0;
                    
                    if(!strcmp(val, "OK"))
                    {
                        if(outList && *outList)
                            uartSend(*(outList++));
                        else
                            if(onDone) onDone(true);
                    }
                    else if(!strcmp(val, "ERROR"))
                    {
                        //char buf[64];
                        out.log("ESP error:");
                        if(outList) out.log(outList[-1]);
                        if(onDone) onDone(false);
                    }
                }; 
                dst = val;
                continue;
            }
            break;
        case 2:
            if(c >= 'A' && c <= 'Z') *(dst++) = c;
            else if(c == ',') 
            { 
                *dst = 0; 
                if(!strcmp(var, "IPD")) { client = 0; state = 6;}
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
                if(!strcmp(var, "CWLAP")) onCWLAP(val);
                
            }
            break;
        case 4:
            if(c >= '0' && c <= '9') client = client * 10 + (c - '0');
            else if(c == ','){ dst = val; state = 5;}
            break;
        case 5:
            if(c >= 'A' && c <= 'Z') *(dst++) = c;
            else if(c == '\n' || c == '\r')
            {
                *dst = 0;
                state = 0;
                if(!strcmp(val, "CONNECT")) 
                {
                    if(onConnect) onConnect(client);
                    parseOn = false; 
                } 
                else if(!strcmp(val, "CLOSED"))
                {
                    if(onDisconnect) onDisconnect(client);
                    
                    // 
                }
            }
            break;
        case 6:
            if(c >= '0' && c <= '9') client = client * 10 + (c - '0');
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
                const char * e = text + size;
                if(e > end) e = end;
                if(parseOn)
                {                    
                    const char * msg = parse(text, e);
                    if(msg) out.log(msg);
                }
                size -= e - text;
                text = e;
                if(!size) {state = 0; parseOn = true;}
            }
            break;
        }
        
    }
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

void ESP::onTX()
{
    
}

ESP::ESP(): dst(var), state(0), outList(0), aps(0), sendSize(0)
{
}

void ESP::echo(bool e, void (* done)(bool Ok))
{
    onDone = done;
    while(uartSend(e ? "ATE1\r\n" : "ATE0\r\n"));
    outDisable(); // Надо дождаться, когда придёт ответ
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
    while(uartSend(buf));
    outDisable();    
    onDone = done;
}

void ESP::exec(const char **list)
{
    if(!list) {outList = 0; return;}
    while(uartSend(*(list++)));
    outList = list;
}

const char * initEsp[10];

void ESP::espInit(const AP * list, const char * ap)
{
    const char * * t = initEsp;
    *(t++) = "ATE0\r\n";
    aps = list;
    if(list)
    {
        *(t++) = "AT+CWMODE=1\r\n";
        *(t++) = "AT+CWLAP\r\n";
    }
    else
    {
        *(t++) = "AT+CWMODE=2\r\n";
        *(t++) = ap;
    }
    *(t++) = "AT+CIPMUX=1\r\n";
    *(t++) = "AT+CIPSERVER=1,8080\r\n";
    *(t++) = "AT+CIFSR\r\n";
    *t = 0;
    exec(initEsp);
}

char * ESP::send(void * obj, char * data, int size)
{
    ESP * esp = (ESP *)obj;
    int len = esp->sendSize;
    if(!len)
    {
        len = out.length();
        char buf[20];
        int l = sprintf(buf, "AT+CIPSEND=%d,%d", 0, len);
        while(esp->uartSend(buf, l));
        esp->sendSize = len;
    }
    if(size > len) size = len;
    size -= esp->uartSend(data, size);
    esp->sendSize = len - size;
    return data + size;
}


////////////////////////////////////////////////////////////////////////////////////////////////////

ESP esp;
UART &uart = esp;

/////////////////////// Обработка прерываний для UART //////////////////////////////////////////////


#ifdef __cplusplus
 extern "C" {
#endif

void DMA1_Channel4_IRQHandler() // Весь буфер отправлен
{
    int isr = DMA1->ISR;
    if(int t = isr & DMA_ISR_TCIF4)
    {
        DMA1->IFCR = t;
        //txData.log("DMA:");
        uart.txComplete();
    }
}

void DMA1_Channel5_IRQHandler() // При приёме заполнено полбуфера или весь буфер
{
    int isr = DMA1->ISR;
    if(int t = isr & (DMA_ISR_HTIF5 | DMA_ISR_TCIF5))
    {
        uart.rxNotEmpty();
        DMA1->IFCR = t;
    }
}

void USART1_IRQHandler() // Прошло достаточное время с последнего приёма
{
    int isr = USART1->ISR;
    if(int t = isr & USART_ISR_RTOF)
    {
        uart.rxNotEmpty();
        USART1->ICR = t;
    }
}

#ifdef __cplusplus
}
#endif


void espConnect(int id)
{
    out.setReceiver(esp.send, &esp);
}

void initESP()
{   
    //esp.exec(initEsp);
    //AP aps[] = {{"water", "888waterparol"}, {0}};
    esp.onConnect = espConnect;
    esp.espInit(0/*aps*/, "AT+CWSAP=\"Scorbot\",\"aaa\",1,0\r\n");
}


