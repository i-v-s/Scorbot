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
#include "controller.h"

// ��������� ��� ����� ���������
//#define CORRECT1         (5)  /*<! ��������� ��� ��������� ������ �������� 1 (���������) */
//#define CORRECT2         (5)  /*<! ��������� ��� ��������� ������ �������� 2 */
//#define CORRECT3         (5)  /*<! ��������� ��� ��������� ������ �������� 3 */
//#define CORRECT4         (5)  /*<! ��������� ��� ��������� ������ �������� 4 */
//#define CORRECT5         (5)  /*<! ��������� ��� ��������� ������ �������� 5 */
//#define CORRECT6         (2)  /*<! ��������� ��� ��������� ������ �������� 6 (������) */

//variables
int c=0;//��� �����
uint16_t rel;//��� �����
int relativ; //��������� ���������� �������
//uint16_t cnt_3;//���� �������� #3
uint8_t monit = 0;//������ ������ ��������
//uint8_t num_enc;// ������� ������ � GetRelative function
uint8_t nozero = 1;

//enc_prg
	uint8_t EncState[6]; //���������� ��������� ��������
	uint16_t encData[6] = {1, 1, 1, 1, 1, 1};	//������� ������� ��������
	
    uint8_t rab[6] = {0};
    uint16_t e[6] = {0};
	
	uint8_t mail = 0;
	uint8_t fll = 1; // � ���..

	uint16_t cnt_base = 0;
	
//	uint8_t EData;
	uint8_t flag = 0;
	uint8_t error = 0;//���� ������
//	uint8_t flg1 = 0;//��� ���� ����
	
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
void Enc_prg(void);//���������� ������� #6
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
 

/*-------------------������� ���������� �����-------------------------*/
//OK 2 -- �� �������, 1 -- ������ �������, 0 -- ����
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

//2 -- ������, 1 -- �����, 0 -- ����
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

//2 -- �� �������, 1 -- ������ �������, 0 -- ����
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

//2 -- ������ �����������, 1 -- ������ �����������, 0 -- ������ ����
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
{ //��������� �������� #1 (platform)

	uint8_t New;
	
	// OK ��� �������� #1
	uint16_t t3 = GPIO_ReadInputData(GPIOA);
	New = (t3>>6) & 0x03;//��������� ��������
    
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
	
	EncState[0] = New;		// ������ ������ ��������
				// ����������� ���������
}
void Enc2(void){ //��������� �������� #2 
	
	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOA);
	New = t3 & 0x03;//��������� ��������
	
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
	
	EncState[1] = New;		// ������ ������ ��������
				// ����������� ���������
}

void Enc3(void)
{ //��������� �������� #3 
	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOA);
	New = (t3>>4) & 0x03;//��������� ��������
 
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
	
	EncState[2] = New;		// ������ ������ ��������
				// ����������� ���������
}
void Enc4(void){ //��������� �������� #4

	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOD);
	New = (t3>>12) & 0x03;//��������� ��������
	
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
	
	EncState[3] = New;		// ������ ������ ��������
				// ����������� ���������
}
void Enc5(void){ //��������� �������� #5

	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOC);
	New = (t3>>6) & 0x03;//��������� ��������
	
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
	
	EncState[4] = New;		// ������ ������ ��������
				// ����������� ���������
}
void EncClaw(void){ //��������� �������� #6 (claw)

	uint8_t New;
	uint16_t t3 = GPIO_ReadInputData(GPIOF);
	New = (t3>>9) & 0x03;//��������� ��������
 
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
	
	EncState[5] = New;		// ������ ������ ��������
				// ����������� ���������
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
