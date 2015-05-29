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

// константы для счета энкодеров
//#define CORRECT1         (5)  /*<! константа для коррекции данных энкодера 1 (платформа) */
//#define CORRECT2         (5)  /*<! константа для коррекции данных энкодера 2 */
//#define CORRECT3         (5)  /*<! константа для коррекции данных энкодера 3 */
//#define CORRECT4         (5)  /*<! константа для коррекции данных энкодера 4 */
//#define CORRECT5         (5)  /*<! константа для коррекции данных энкодера 5 */
//#define CORRECT6         (2)  /*<! константа для коррекции данных энкодера 6 (клешня) */

//variables
int c=0;//для счета
uint16_t rel;//для счета
int relativ; //очередная переменная вникуда
//uint16_t cnt_3;//счет энкодера #3
uint8_t monit = 0;//анализ номера энкодера
//uint8_t num_enc;// удалить вместе с GetRelative function
uint8_t nozero = 1;

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
void zero(void);
void platform(uint8_t x);
void axis2(uint8_t x);
void axis3(uint8_t x);
void axis4(uint8_t x);
void axis5(uint8_t x);
void claw(uint8_t x);
void EncPlatform(void);
void Enc2(void);
void Enc3(void);
void Enc4(void);
void Enc5(void);
void EncClaw(void);
//void EncInit(void);
void delay_d(void);
//void TIM3_IRQHandler(void);
void Enc_prg(void);//аппаратный энкодер #6
/* Private functions ---------------------------------------------------------*/


int main(void)
{
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
    mx_pinout_config();//gpio init
   
    while(1)
    {
		if(nozero)
		{
			zero(); 
			nozero = 0;
		}
        ctlLoop();
			
				
    if (bDeviceState == CONFIGURED)
    {
        CDC_Receive_DATA();
        if(Receive_length)
        {
            char buf[16];
            for(int x = 0; x < Receive_length; x++) buf[x] = Receive_Buffer[x];
            const char * e = parse(buf, buf + Receive_length);
            //if(Receive_Buffer[0] == 'H')
            if(e)
                sendText(e);
            sendAll();
            Receive_length = 0;
        }
    }			
			
    }
}
 

/*-------------------функции управления осями-------------------------*/
//OK 2 -- по часовой, 1 -- против часовой, 0 -- стоп
void platform(uint8_t x){
	
		switch (x)
			{
			case 0:
					GPIOC->BRR = GPIO_Pin_10; //enabled
                  	GPIOA->BRR = GPIO_Pin_15; //move
                  	GPIOA->BRR = GPIO_Pin_4; //GPIO_Pin_14;
					break;
			
			case 1:
					GPIOC->BSRR = GPIO_Pin_10; //enabled
                  	GPIOA->BSRR = GPIO_Pin_15; //move
                  	GPIOA->BRR = GPIO_Pin_4; //GPIO_Pin_14;
					break;
					 
			case 2:
					GPIOC->BSRR = GPIO_Pin_10; //enabled
                  	GPIOA->BSRR = GPIO_Pin_4; //move
                  	GPIOA->BRR = GPIO_Pin_15;
					break;
			}			
}

//2 -- вперед, 1 -- назад, 0 -- стоп
void axis2(uint8_t x){
	switch (x)
	{
		case 2:
				GPIOB->BSRR = GPIO_Pin_4; //enabled
                GPIOD->BSRR = GPIO_Pin_7; //move
                GPIOB->BRR = GPIO_Pin_3;
				break;
				  	
		case 1:  
				GPIOB->BSRR = GPIO_Pin_4; //enabled
                GPIOB->BSRR = GPIO_Pin_3; //move
                GPIOD->BRR = GPIO_Pin_7;
				break;
					  
		case 0:  
				GPIOB->BRR = GPIO_Pin_4; //enabled
                GPIOB->BRR = GPIO_Pin_3; //move
                GPIOD->BRR = GPIO_Pin_7;
				break;		
	}
}

void axis3(uint8_t x){
	
	switch (x)
	{
		case 2:
									GPIOD->BSRR = GPIO_Pin_0; //enabled
                  GPIOC->BRR = GPIO_Pin_11; //move
                  GPIOC->BSRR = GPIO_Pin_12;
									break;
				  	
		case 1:  		
									GPIOD->BSRR = GPIO_Pin_0; //enabled
                  GPIOC->BSRR = GPIO_Pin_11; //move
                  GPIOC->BRR = GPIO_Pin_12;
									break;
				
		case 0:
									GPIOD->BRR = GPIO_Pin_0; //enabled
                  GPIOC->BRR = GPIO_Pin_11; //move
                  GPIOC->BRR = GPIO_Pin_12;
									break;
	}
}

void axis4(uint8_t x){
	
	switch (x)
	{
		case 2:
				GPIOD->BSRR = GPIO_Pin_3; //enabled
                GPIOD->BSRR = GPIO_Pin_1; //move
                GPIOD->BRR = GPIO_Pin_2;
                GPIOD->BSRR = GPIO_Pin_6; //enabled
                GPIOD->BSRR = GPIO_Pin_4; //move
                GPIOD->BRR = GPIO_Pin_5;
				break;
		
		case 1: 
				GPIOD->BSRR = GPIO_Pin_3; //enabled
                GPIOD->BSRR = GPIO_Pin_2; //move
                GPIOD->BRR = GPIO_Pin_1;
                GPIOD->BSRR = GPIO_Pin_6; //enabled
                GPIOD->BSRR = GPIO_Pin_5; //move
                GPIOD->BRR = GPIO_Pin_4;
				break;
		
		case 0:  
				GPIOD->BRR = GPIO_Pin_3; //enabled
                GPIOD->BRR = GPIO_Pin_2; //move
                GPIOD->BRR = GPIO_Pin_1;
                GPIOD->BRR = GPIO_Pin_6; //enabled
                GPIOD->BRR = GPIO_Pin_5; //move
                GPIOD->BRR = GPIO_Pin_4;
				break;
	}
}

//2 -- по часовой, 1 -- против часовой, 0 -- стоп
void axis5(uint8_t x){
	switch (x)
	{
		case 2:
				GPIOD->BSRR = GPIO_Pin_6; //enabled
                GPIOD->BSRR = GPIO_Pin_4; //move
                GPIOD->BRR = GPIO_Pin_5;
                GPIOD->BSRR = GPIO_Pin_3; //enabled
                GPIOD->BSRR = GPIO_Pin_2; //move
                GPIOD->BRR = GPIO_Pin_1;
				break;
		
		case 1:
				GPIOD->BSRR = GPIO_Pin_6; //enabled
                GPIOD->BSRR = GPIO_Pin_5; //move
                GPIOD->BRR = GPIO_Pin_4;
                GPIOD->BSRR = GPIO_Pin_3; //enabled
                GPIOD->BSRR = GPIO_Pin_1; //move
                GPIOD->BRR = GPIO_Pin_2;
				break;
		
		case 0:  
				GPIOD->BRR = GPIO_Pin_6; //enabled
                GPIOD->BRR = GPIO_Pin_5; //move
                GPIOD->BRR = GPIO_Pin_4;
                GPIOD->BRR = GPIO_Pin_3; //enabled
                GPIOD->BRR = GPIO_Pin_1; //move
                GPIOD->BRR = GPIO_Pin_2;
				break;
	}
}

//2 -- клешня закрывается, 1 -- клешня открывается, 0 -- клешня стоп
void claw(uint8_t x){
	
	switch (x)
	{
		case 2:
				GPIOC->BSRR = GPIO_Pin_9; //enabled
                GPIOA->BRR = GPIO_Pin_5;//GPIO_Pin_13; //move
                GPIOF->BSRR = GPIO_Pin_6;
				break;
		
		case 1:  
				GPIOC->BSRR = GPIO_Pin_9; //enabled
                GPIOA->BSRR = GPIO_Pin_5;//GPIO_Pin_13; //move
                GPIOF->BRR = GPIO_Pin_6;
				break;
		
		case 0:  
				GPIOC->BRR = GPIO_Pin_9; //enabled
                GPIOA->BRR = GPIO_Pin_5;//GPIO_Pin_13; //move
                GPIOF->BRR = GPIO_Pin_6;
				break;
	}
}
void EncPlatform(void)
{ //обработка энкодера #1 (platform)

	uint8_t New;
	
	// OK для энкодера #1
	uint16_t t3 = GPIO_ReadInputData(GPIOA);
	New = (t3>>6) & 0x03;//состояние энкодера
    
    switch(EncState[0])
	{
	case 2:
		{
		if(New == 3) encData[0]++;
		if(New == 0) encData[0]--; 
		break;
		}

	case 0:
		{
		if(New == 2) encData[0]++;
		if(New == 1) encData[0]--;
		break;
		}
	case 1:
		{
		if(New == 0) encData[0]++;
		if(New == 3) encData[0]--;
		break;
		}
	case 3:
		{
		if(New == 1) encData[0]++;
		if(New == 2) encData[0]--; 
		break;
		}
	}
	
	EncState[0] = New;		// запись нового значения
				// предыдущего состояния
}
void Enc2(void){ //обработка энкодера #2 
	
	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOA);
	New = t3 & 0x03;//состояние энкодера
	
								switch(EncState[1])
									{
									case 2:
										{
										if(New == 3) encData[1]++;
										if(New == 0) encData[1]--; 
										break;
										}

									case 0:
										{
										if(New == 2) encData[1]++;
										if(New == 1) encData[1]--;
										break;
										}
									case 1:
										{
										if(New == 0) encData[1]++;
										if(New == 3) encData[1]--;
										break;
										}
									case 3:
										{
										if(New == 1) encData[1]++;
										if(New == 2) encData[1]--; 
										break;
										}
									}
	
	EncState[1] = New;		// запись нового значения
				// предыдущего состояния
}

void Enc3(void)
{ //обработка энкодера #3 
	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOA);
	New = (t3>>4) & 0x03;//состояние энкодера
 
		switch(EncState[2])
			{
			case 2:
				{
				if(New == 3) encData[2]++;
				if(New == 0) encData[2]--; 
				break;
				}

			case 0:
				{
				if(New == 2) encData[2]++;
				if(New == 1) encData[2]--;
				break;
				}
			case 1:
				{
				if(New == 0) encData[2]++;
				if(New == 3) encData[2]--;
				break;
				}
			case 3:
				{
				if(New == 1) encData[2]++;
				if(New == 2) encData[2]--; 
				break;
				}
			}
	
	EncState[2] = New;		// запись нового значения
				// предыдущего состояния
}
void Enc4(void){ //обработка энкодера #4

	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOD);
	New = (t3>>12) & 0x03;//состояние энкодера
	
	switch(EncState[3])
		{
		case 2:
			{
			if(New == 3) encData[3]++;
			if(New == 0) encData[3]--; 
			break;
			}

		case 0:
			{
			if(New == 2) encData[3]++;
			if(New == 1) encData[3]--;
			break;
			}
		case 1:
			{
			if(New == 0) encData[3]++;
			if(New == 3) encData[3]--;
			break;
			}
		case 3:
			{
			if(New == 1) encData[3]++;
			if(New == 2) encData[3]--; 
			break;
			}
		}
	
	EncState[3] = New;		// запись нового значения
				// предыдущего состояния
}
void Enc5(void){ //обработка энкодера #5

	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOC);
	New = (t3>>6) & 0x03;//состояние энкодера
	
	switch(EncState[4])
		{
		case 2:
			{
			if(New == 3) encData[4]++;
			if(New == 0) encData[4]--; 
			break;
			}

		case 0:
			{
			if(New == 2) encData[4]++;
			if(New == 1) encData[4]--;
			break;
			}
		case 1:
			{
			if(New == 0) encData[4]++;
			if(New == 3) encData[4]--;
			break;
			}
		case 3:
			{
			if(New == 1) encData[4]++;
			if(New == 2) encData[4]--; 
			break;
			}
		}
	
	EncState[4] = New;		// запись нового значения
				// предыдущего состояния
}
void EncClaw(void){ //обработка энкодера #6 (claw)

	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOF);
	New = (t3>>9) & 0x03;//состояние энкодера
 
					switch(EncState[5])
						{
						case 2:
							{
							if(New == 3) encData[5]++;
							if(New == 0) encData[5]--; 
							break;
							}

						case 0:
							{
							if(New == 2) encData[5]++;
							if(New == 1) encData[5]--;
							break;
							}
						case 1:
							{
							if(New == 0) encData[5]++;
							if(New == 3) encData[5]--;
							break;
							}
						case 3:
							{
							if(New == 1) encData[5]++;
							if(New == 2) encData[5]--; 
							break;
							}
						}
	
	EncState[5] = New;		// запись нового значения
				// предыдущего состояния
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
