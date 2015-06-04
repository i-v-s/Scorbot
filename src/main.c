/**
********************************************************************************
программа управления манипулятором SCORBOT-ER III

Засылается пакет 4 байта. 1 байт -- чтение/запись, 2 байт -- номер мотора (энкодера),
3 байт -- угол поворота (чтение значения с энкодера), 4 байт -- контрольная сумма (пока 13).

3. Обработка данных и поведение в зависимости от данных с энкодеров.
4. Установка нуля с зависимостью от #SW.
5. Защита моторов с зависимостью от времени, за которое данные с энкодера не изменились.

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
#include "controller.h"

uint8_t noZero = 0x3F;

//enc_prg
	uint8_t EncState[6]; //предыдущее состояние энкодера
	uint16_t encData[6] = {1, 1, 1, 1, 1, 1};	//счетный регистр энкодера
	
    uint8_t rab[6] = {0};
    uint16_t e[6] = {0};
	
	uint8_t mail = 0;
	uint8_t fll = 1; // и это..

	uint16_t cnt_base = 0;
	
//	uint8_t EData;
	uint8_t flag = 0;
	uint8_t error = 0;//флаг ошибки
//	uint8_t flg1 = 0;//еще один флаг
	
	uint8_t data1_one = 0;
	uint8_t data2_one = 0;
	uint8_t data3_one = 0;
	uint8_t data4_one = 0;
	uint8_t data5_one = 0;
	uint8_t data6_one = 0;
	
	uint8_t temp8;
	
	uint8_t m1 = 0;
	uint8_t m2 = 0;
	uint8_t m3 = 0;
	uint8_t m4 = 0;
	uint8_t m5 = 0;
	uint8_t m6 = 0;

//uint8_t flag_deb=0;
//uint8_t flag_deb1=0;
int cnt_deb=128;
int cnt_deb1=0;

//long Chislo;
//long Chisl;
//long Chis;

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
void Enc_prg(void);//аппаратный энкодер #6
/* Private functions ---------------------------------------------------------*/


int main(void)
{
    char enOut = 0;
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
    mx_pinout_config();
    sendText("Scorbot firmware 0.1 ");
   
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
                if(!enOut) { enOut = 1; enableOut();}
                
                char buf[16];
                for(int x = 0; x < Receive_length; x++) buf[x] = Receive_Buffer[x];
                const char * e = parse(buf, buf + Receive_length);
                //if(Receive_Buffer[0] == 'H')
                if(e)
                    sendText(e);
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
