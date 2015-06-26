/**
********************************************************************************
��������� ���������� ������������� SCORBOT-ER III

���������� ����� 4 �����. 1 ���� -- ������/������, 2 ���� -- ����� ������ (��������),
3 ���� -- ���� �������� (������ �������� � ��������), 4 ���� -- ����������� ����� (���� 13).

3. ��������� ������ � ��������� � ����������� �� ������ � ���������.
4. ��������� ���� � ������������ �� #SW.
5. ������ ������� � ������������ �� �������, �� ������� ������ � �������� �� ����������.

********************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "parser.h"
#include <stm32f30x_tim.h>
//#include <misc.h>
#include "mx_gpio.h"
#include "host_io.h"
#include "wireless.h"
#include "controller.h"
#include "kinematics.h"
#include "test.h"
#include <stdio.h>

uint8_t noZero = 0x3F;

	uint16_t che;
	uint8_t div;

/* Extern variables ----------------------------------------------------------*/
extern __IO uint8_t Receive_Buffer[4];//[64]
extern __IO  uint32_t Receive_length ;
extern __IO  uint32_t length ;
uint8_t Send_Buffer[4];//[64]
uint32_t packet_sent=1;
uint32_t packet_receive=1;

/* Private function prototypes -----------------------------------------------*/
void init_gpio(void);
void init_timer(void);
char zero(char noZero);
void platform(uint8_t x);
//void EncInit(void);
void delay_d(void);
//void TIM3_IRQHandler(void);
void Enc_prg(void);//���������� ������� #6
/* Private functions ---------------------------------------------------------*/

int main(void)
{
    //char enOut = 0;
    Set_System();
    //Set_USBClock();
    //USB_Interrupts_Config();
    //USB_Init();
    mx_pinout_config();
#ifdef _TEST_
    Test::run();
    while(1);
#endif
    
    esp.uartInit(USART1, DMA1, DMA1_Channel5, DMA1_Channel4);
    initESP();
    
    out.log("Scorbot firmware 0.1 "__DATE__);
    while(1)
    {
		if(noZero)
		{
			zero(noZero); 
            noZero = 0;
		}
        ctlLoop();
					
        if (bDeviceState == CONFIGURED)
        {
            CDC_Receive_DATA();
            if(Receive_length)
            {
                //if(!enOut) { enOut = 1; enableOut();}
                
                char buf[16];
                for(int x = 0; x < Receive_length; x++) buf[x] = Receive_Buffer[x];
                const char * e = parse(buf, buf + Receive_length);
                if(e)
                    out.log(e);
                Receive_length = 0;
            }
        }						
    }
}

#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
