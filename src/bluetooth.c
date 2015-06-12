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

char buff[64];

void initUART(USART_TypeDef * USART, DMA_TypeDef * DMA, DMA_Channel_TypeDef * DMAC)
{   
    DMAC->CNDTR = sizeof(buff);
    DMAC->CMAR = (uint32_t) buff;
    DMAC->CPAR = (uint32_t) &USART->RDR;
    DMAC->CCR = DMA_CCR_PL_1 | /*DMA_CCR_TEIE |*/ DMA_CCR_MINC | /*DMA_CCR_PSIZE_0 |*/ DMA_CCR_EN | DMA_CCR_CIRC;
    
    //USART->BRR = 0x0753; // 38400
    //USART->BRR = 0x1D4C; //  9600
    //USART->BRR = 0x04E2; // 57600
    USART->BRR = 0x271;  // 115200
    USART->CR1 = USART_CR1_UE;
    USART->CR2 = 0;
    USART->CR3 = USART_CR3_DMAR;
    USART->CR1 |= USART_CR1_TE | USART_CR1_RE;
    char buf[20];
    //while(1)
    //{
        sendToUART(USART, "AT\r\n");//"AT+RST\n");//AT+CWMODE=?\r\n");
        //char * t = buf;
        //for(int x = 10000000; x--;)
            //if(USART->ISR & USART_ISR_RXNE) *(t++) = USART->RDR;
    //}
    //*t = 0;
    //sendText(buf);
    while(1);
    //| USART_CR1_RE | USART_CR1_TE;

    
    
}