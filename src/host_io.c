#include <stdint.h>
#include "usb_desc.h"
#include "hw_config.h"
#include <string.h>

unsigned char data[VIRTUAL_COM_PORT_DATA_SIZE - 1], * dp = data;
extern volatile uint32_t packet_sent;


void sendAll()
{
    while(!packet_sent);
    CDC_Send_DATA(data, dp - data);
    dp = data;
}

void sendText(const char * text)
{
    int l = strlen(text);
    while(l-- && dp < data + sizeof(data)) *(dp++) = *(text++);
    while(l > 0)
    {
        sendAll();
        while(l-- && dp < data + sizeof(data)) *(dp++) = *(text++);
    }
}
