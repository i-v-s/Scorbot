/**
********************************************************************************
��������� ���������� ������������� SCORBOT-ER III

���������� ����� 4 �����. 1 ���� -- ������/������, 2 ���� -- ����� ������ (��������),
3 ���� -- ���� �������� (������ �������� � ��������), 4 ���� -- ����������� ����� (���� 13).

###################################################
�������:
- ������� 3 -- �������� ���������� ��������.
- ������ �������.

###################################################

1. ���� �� ��������� 1 � 8.
�� 2. ���� �� �������� #6 (����������� ���������).
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
#include <stm32f30x_tim.h>
//#include <misc.h>
#include "mx_gpio.h"

//variables
int c=0;//��� �����
uint16_t rel;//��� �����
int relativ; //��������� ���������� �������
uint16_t cnt_3;//���� �������� #3
uint16_t monit;//������ ������ ��������
uint8_t num_enc;

//enc_prg
	uint8_t EncState = 0; //���������� ��������� ��������
	uint16_t EncData = 128;	//������� ������� ��������

//hard_enc
typedef enum { FORWARD, BACKWARD } Direction;

volatile uint8_t capture_is_first = 1, capture_is_ready = 0;
volatile Direction captured_direction = FORWARD;

//uint8_t flag_deb=0;
//uint8_t flag_deb1=0;
int cnt_deb=128;
int cnt_deb1=0;

long Chislo;
long Chisl;
long Chis;

/* Extern variables ----------------------------------------------------------*/
extern __IO uint8_t Receive_Buffer[4];//[64]
extern __IO  uint32_t Receive_length ;
extern __IO  uint32_t length ;
uint8_t Send_Buffer[4];//[64]
uint32_t packet_sent=1;
uint32_t packet_receive=1;

Direction direction;
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
//void EncInit(void);
void delay_d(void);
//void TIM3_IRQHandler(void);
void Enc_prg(void);//���������� ������� #6
/* Private functions ---------------------------------------------------------*/
// ���������� ������������� �����������, ��� num_enc �� 1 �� 5 (��� ��� "claw"/#6 ��������� �������� �� ������������)
int16_t enc_GetRelativeMove(uint8_t num_enc){
	
	static uint16_t CNT_last1;
	static uint16_t CNT_last2;
	static uint16_t CNT_last3;
	static uint16_t CNT_last4;
	static uint16_t CNT_last5;
	uint16_t CNT_now;
	int16_t  CNT_diff;
	
	switch (num_enc)
		{
		case 1:
				// ��� �������� ����������� ��������
				CNT_last1 = 0;
					
				// ������� ��������
				CNT_now = TIM1->CNT;
				
				// ��������� �����������
				CNT_diff = (int16_t)(CNT_now - CNT_last1);
					
				// �������� ������� ��������� ��� ����������
				CNT_last1 = CNT_now;
					
				// ������ �����������
				//return CNT_diff;
				break;
		
		case 2:
				CNT_last2 = 0;
				CNT_now = TIM2->CNT;
				CNT_diff = (int16_t)(CNT_now - CNT_last2);
				CNT_last2 = CNT_now;
				//return CNT_diff;
				break;
		
		case 3:
				CNT_last3 = 0;
				CNT_now = TIM3->CNT;
				CNT_diff = (int16_t)(CNT_now - CNT_last3);
				CNT_last3 = CNT_now;
				//return CNT_diff;
				break;
		
		case 4:
				CNT_last4 = 0;
				CNT_now = TIM4->CNT;
				CNT_diff = (int16_t)(CNT_now - CNT_last4);
				CNT_last4 = CNT_now;
				//return CNT_diff;
				break;
		
		case 5:
				CNT_last5 = 0;
				CNT_now = TIM8->CNT;
				CNT_diff = (int16_t)(CNT_now - CNT_last5);
				CNT_last5 = CNT_now;
				//return CNT_diff;
				break;
		
		default: 
				CNT_diff=0; 
				//return CNT_diff;
				break;
	}
		return CNT_diff;
} 

// ���������� ��������� ��������
int16_t enc_GetPosition(uint8_t num_enc){
	int16_t ret;
	switch (num_enc)
	{
		case 1: ret = TIM1->CNT; break;//platform
		case 2: ret = TIM2->CNT; break;//#2
		case 3: ret = TIM3->CNT; break;//#3
		case 4: ret = TIM4->CNT; break;//#4
		case 5: ret = TIM8->CNT; break;//#5
		default: ret = 0; break;
	}	
	return ret;
}
void timer_CheckEncoder(void)//������� ��� �������� ���������
	{
	int16_t diff = enc_GetRelativeMove(3);
	int16_t pos = enc_GetPosition(3);
    
	// ���� �� ����������
		GPIOE->BRR = GPIO_Pin_8;//led2
		GPIOE->BRR = GPIO_Pin_15;//led1
		GPIOE->BRR = GPIO_Pin_14;//led3
    
	// ���� ��������� ������� ������� �� ���� -- �������� led1
	if(pos > 0) {GPIOE->BSRR = GPIO_Pin_15; GPIOE->BRR = GPIO_Pin_8; GPIOE->BRR = GPIO_Pin_14;}
    
	// ���� ������������� ����������� ������ ����, �� �������� led2
	if(diff > 0) {GPIOE->BSRR = GPIO_Pin_8; GPIOE->BRR = GPIO_Pin_15; GPIOE->BRR = GPIO_Pin_14;}

	// ���� ����������� ���� ����������� � �������� ������� -- �������� LED3
	if (diff < 0) {GPIOE->BSRR = GPIO_Pin_14; GPIOE->BRR = GPIO_Pin_8; GPIOE->BRR = GPIO_Pin_15;}
}

int main(void)
{
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  mx_pinout_config();//gpio init
//	EncInit();//apparat_encoders init
 	
  while (1)
  {
		Enc_prg();//��������� ������� #6
		/*
		if (capture_is_ready)
    {
      NVIC_DisableIRQ(TIM3_IRQn);
      capture_is_ready = 0;
      direction = captured_direction;
      NVIC_EnableIRQ(TIM3_IRQn);
			
			switch(direction)
				{
				case FORWARD:
											cnt_deb++;
											break;
				
				case BACKWARD:	
											cnt_deb--;
											break;
				} 
			
    }*/
		
		//((GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_9))
				
		//timer_CheckEncoder();
				
    if (bDeviceState == CONFIGURED)
    {
      CDC_Receive_DATA();
			
			if (Receive_Buffer[0]==0) // ��������� �����, ������������ ������������ ��������� ���������
            {
							//---DEBUG-------
									platform(0);
									axis2(0);
									axis3(0);
									axis4(0);
									axis5(0);
									claw(0);
							
							// �������� �������� ���� ���������
									TIM1->CNT = 0;
									TIM2->CNT = 0;
									TIM3->CNT = 0;
									TIM4->CNT = 0;
									TIM8->CNT = 0;
							//---------------
									Send_Buffer[0] = 0;//�������� ����� �������� ������
									Send_Buffer[1] = 0;
									Send_Buffer[2] = 0;
									Send_Buffer[3] = 0;
							
              if(Receive_Buffer[1]==1)
                {
                  //zero();
                }
            }//end #0
      
//****** ������ -- ����� ������ 					
								
      if (Receive_Buffer[0] == 1) 
            {
              switch (Receive_Buffer[1])
							{
								case 0: 
													 platform(0);
													 axis2(0);
													 axis3(0);
													 axis4(0);
													 axis5(0);
													 claw(0);
													 break;
								
								case 1:
													 platform(2);
													
													 //if (Receive_Buffer[3] != enc_GetRelativeMove){}
													 break;
								
								case 11:
													 platform(1);
													 break;
								
								case 2:
													 axis2(2);
													 break;
								
								case 22:
													 axis2(1);
													 break;
								
								case 3:
													 axis3(2);
													 break;
								
								case 33:
													 axis3(1);
													 break;
								
								case 4:
													 axis4(2);
													 break;
								
								case 44:
													 axis4(1);
													 break;
								
								case 5:
													 axis5(2);
													 break;
								
								case 55:
													 axis5(1);
													 break;
								
								case 6: 	 //otkrivaetsa
													 claw(1);
													 break;
								
								case 66:	 //zakrivaetsa
													 claw(2);
													 break;
							}//end switch
						}//end if
											
      /*Check to see if we have data yet */
      if (Receive_length  != 0)
      {
				/*-----------------protocol-------------------*/
				//���������� ������ � ��������� ��������
				//monit = Receive_Buffer[1];
				//if (monit > 6) {monit = monit/10;}//���� � �������� �������, �� ����� �� 10 (44/4=4 -- ������� #4)
				//������!!!
				//c = enc_GetPosition(num_enc);//enc_GetPosition(monit);
				Send_Buffer[0] = EncData;
				Send_Buffer[1] = EncState;
				//Send_Buffer[0] = c>>8;
				//Send_Buffer[1] = c;
				//���������� ������ �� ������������� �����������
				//rel = (enc_GetRelativeMove(monit))/10;
				//relativ = rel>>8;
				//Send_Buffer[1] = cnt_deb;
				//Send_Buffer[2] = (TIM3->CR1 & TIM_CR1_DIR);
				//Send_Buffer[0] = TIM3->CNT;
				//Send_Buffer[1] = (uint8_t)rel;
				/*-------------------------------------------*/
        if (packet_sent == 1)
          CDC_Send_DATA (Send_Buffer, Receive_length);
        Receive_length = 0;
      }
					
			
    }//end if (bDeviceState == CONFIGURED)
  }//end while
} 

/*-------------------������� ���������� �����-------------------------*/
//OK 2 -- �� �������, 1 -- ������ �������, 0 -- ����
void platform(uint8_t x){
		switch (x)
			{
			case 0:
									GPIOC->BRR = GPIO_Pin_10; //enabled
                  GPIOA->BRR = GPIO_Pin_15; //move
                  GPIOA->BRR = GPIO_Pin_14;
									break;
			
			case 1:
									GPIOC->BSRR = GPIO_Pin_10; //enabled
                  GPIOA->BSRR = GPIO_Pin_15; //move
                  GPIOA->BRR = GPIO_Pin_14;
									break;
					 
			case 2:
									GPIOC->BSRR = GPIO_Pin_10; //enabled
                  GPIOA->BSRR = GPIO_Pin_14; //move
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
                  GPIOD->BRR = GPIO_Pin_0; //enabled
                  GPIOC->BRR = GPIO_Pin_1; //move
                  GPIOC->BRR = GPIO_Pin_2;
									break;
	}
}

//2 -- ������ �����������, 1 -- ������ �����������, 0 -- ������ ����
void claw(uint8_t x){
	
	switch (x)
	{
		case 2:
									GPIOC->BSRR = GPIO_Pin_9; //enabled
                  GPIOA->BRR = GPIO_Pin_13; //move
                  GPIOF->BSRR = GPIO_Pin_6;
									break;
		
		case 1:  
									GPIOC->BSRR = GPIO_Pin_9; //enabled
                  GPIOA->BSRR = GPIO_Pin_13; //move
                  GPIOF->BRR = GPIO_Pin_6;
									break;
		
		case 0:  
									GPIOC->BRR = GPIO_Pin_9; //enabled
                  GPIOA->BRR = GPIO_Pin_13; //move
                  GPIOF->BRR = GPIO_Pin_6;
									break;
	}
}

/*----------------����� ������� �� ���������� �����-------------------*/
/*void TIM3_IRQHandler(void){
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    if (!capture_is_first)
      capture_is_ready = 1;

    capture_is_first = 0;
     // � ���� TIM_CR1_DIR �������� TIM3_CR1 ��������
     //  ����������� �������� ��������, ���������� ��� �������� 
    captured_direction = (TIM3->CR1 & TIM_CR1_DIR ? FORWARD : BACKWARD);
  }
}

void EncInit(void){
	
	TIM_TimeBaseInitTypeDef timer_base;
	
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM1, ENABLE); 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM8, ENABLE);

  
  TIM_TimeBaseStructInit(&timer_base);
  timer_base.TIM_Period = 0xFFFFFFFF;//4;
	timer_base.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_base.TIM_CounterMode = TIM_CounterMode_Up;//���� �� ������ � ����� TIM_CounterMode_Down | 
 // TIM_TimeBaseInit(TIM1, &timer_base);
  TIM_TimeBaseInit(TIM2, &timer_base);
  TIM_TimeBaseInit(TIM3, &timer_base);
  TIM_TimeBaseInit(TIM4, &timer_base);
 // TIM_TimeBaseInit(TIM8, &timer_base);


  TIM_EncoderInterfaceConfig(TIM1, TIM_EncoderMode_TI12,
  TIM_ICPolarity_BothEdge, TIM_ICPolarity_BothEdge);
  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM1, ENABLE);
  
 // NVIC_EnableIRQ(TIM1_IRQn); 
  //----------------------------------
  TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_BothEdge, TIM_ICPolarity_BothEdge);
 // TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM2, ENABLE);

//  NVIC_EnableIRQ(TIM2_IRQn);
  //----------------------------------
  TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//TIM_EncoderMode_TI12
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM3, ENABLE);

  NVIC_EnableIRQ(TIM3_IRQn);
  //----------------------------------
  TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_BothEdge, TIM_ICPolarity_BothEdge);
//  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM4, ENABLE);

// NVIC_EnableIRQ(TIM4_IRQn);
  //----------------------------------
  TIM_EncoderInterfaceConfig(TIM8, TIM_EncoderMode_TI12,
  TIM_ICPolarity_BothEdge, TIM_ICPolarity_BothEdge);
  TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM8, ENABLE);

 // NVIC_EnableIRQ(TIM8_IRQn);
}//end EncInit(); */
void delay_d(void){ //�������� 0,5 �
		Chisl = 10;      
       while (Chisl>0)              
       {
               Chisl--;                
       }
}

void Enc_prg(void){ //��������� �������� #6 (claw)

	uint8_t New;
	//uint16_t t3 = GPIO_ReadInputData(GPIOF);
	//New = (t3>>8) & 0x06;//��������� ��������
	//��� �������� 3
	//uint16_t t3 = GPIO_ReadInputData(GPIOE);
	//New = t3 & 0x03;//��������� ��������
	//��� #4
	uint16_t t3 = GPIO_ReadInputData(GPIOD);
	New = (t3>>8) & 0x30;//��������� ��������
 
switch(EncState)
	{
	case 2:
		{
		if(New == 3) EncData++;
		if(New == 0) EncData--; 
		break;
		}

	case 0:
		{
		if(New == 2) EncData++;
		if(New == 1) EncData--;
		break;
		}
	case 1:
		{
		if(New == 0) EncData++;
		if(New == 3) EncData--;
		break;
		}
	case 3:
		{
		if(New == 1) EncData++;
		if(New == 2) EncData--; 
		break;
		}
	}
	
	EncState = New;		// ������ ������ ��������
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
