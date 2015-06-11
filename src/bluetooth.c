#include "mx_gpio.h"


void sendToUART(USART_TypeDef * USART, const char * text)
{
    while(*text)
    {
        while(!(USART->ISR & USART_ISR_TXE));
        USART->TDR = *(text++);       
    }
    //
    
}

void initBluetooth(USART_TypeDef * USART)
{   
    //USART->BRR = 0x0753; // 38400
    USART->BRR = 0x1D4C; //  9600
    //USART->BRR = 0x04E2; // 57600
    //USART->BRR = 0x271;  // 115200
    USART->CR1 = USART_CR1_UE;
    USART->CR2 = 0;
    USART->CR3 = 0;
    USART->CR1 |= USART_CR1_TE | USART_CR1_RE;
    char buf[20];
    while(1)
    {
        sendToUART(USART, "AT\r\n");//"AT+RST\n");//AT+CWMODE=?\r\n");
        char * t = buf;
        for(int x = 10000000; x--;)
            if(USART->ISR & USART_ISR_RXNE) *(t++) = USART->RDR;
    }
    //*t = 0;
    //sendText(buf);
    while(1);
    //| USART_CR1_RE | USART_CR1_TE;

    
    
}