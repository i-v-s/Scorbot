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
uint8_t nozero = 0;

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
/* ���������� ������������� �����������, ��� num_enc �� 1 �� 5 (��� ��� "claw"/#6 ��������� �������� �� ������������)
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
				brea                                                                                                                                                                                                                                                                                                                                 k;
		
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
*/

int main(void)
{
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
    mx_pinout_config();//gpio init
     
    for(int x = 10000000; x; x--)
    {
        ctlLoop();
    }
    while(1);
    while(1)
    {
		if(nozero)
		{
			zero(); 
			//zero();
			//zero();
			nozero = 0;
		}

		//������� ��������
        EncPlatform();
        Enc2();
        Enc3();
        Enc4();
        Enc5();
        EncClaw();

        if (((encData[0] >> 6) & 0xFF) > 250) {encData[0] += (1<<6);}
        if (((encData[1] >> 5) & 0xFF) > 250) {encData[1] += (1<<5);}
        if (((encData[2] >> 6) & 0xFF) > 250) {encData[2] += (1<<6);}
        if (((encData[3] >> 4) & 0xFF) > 250) {encData[3] += (1<<4);}
        if (((encData[4] >> 5) & 0xFF) > 250) {encData[4] += (1<<5);}
        if (((encData[5] >> 2) & 0xFF) > 250) {encData[5] += (1<<2);}
				/*���������� ������ � ��������� ��������
		if (Receive_Buffer[0] == 1)
		{
			monit = Receive_Buffer[1];
			if (monit > 10)  
			{
				monit = monit/10;
			}//���� � �������� �������, �� ����� �� 10 (44/4=4 -- ������� #4)
		}	*/			
		if (rab[0])
		{
			temp8 = ((encData[0] >> 6) & 0xFF);
			//if ((temp8 > 205) || (e[0] > 205))
			//	{
			//		error = 1;
			//	}
			if(e[0] > temp8)
			{
				platform (1);
			}
			if (e[0] < temp8)
			{
				platform(2);
			}
			if (e[0] == temp8)
			{
				platform(0);
				rab[0] = 0;
			}
			if (error) //��������� ������!!!
			{
				platform(0);
				rab[0] = 0;
				error = 0;
			}
		}

		if (rab[1])
		{
			temp8 = ((encData[1] >> 5) & 0xFF);
			//if ((temp8 > 205) || (e[1] > 205))//��������!!
			//	{
			//		error = 1;
			//	}
			if(e[1] > temp8)
			{
				axis2(2);
			}
			if (e[1] < temp8)
			{
				axis2(1);
			}
			if (e[1] == temp8)
			{
				axis2(0);
				rab[1] = 0;
			}
			if (error) //��������� ������!!!
			{
				axis2(0);
				rab[1] = 0;
				error = 0;
			}
		}

		if (rab[2])
		{
			temp8 = ((encData[2]>>6) & 0xFF);
			//if ((temp8 > 100) || (e[2] > 100))
			//	{
			//		error = 1;
			//	}
			if(e[2] > temp8)
			{
				axis3(1);
			}
			if (e[2] < temp8)
			{
				axis3(2);
			}
			if (e[2] == temp8)
			{
				axis3(0);
				rab[2] = 0;
			}
			if (error) //��������� ������!!!
			{
				axis3(0);
				rab[2] = 0;
				error = 0;
			}
		}

		if (rab[3])
		{
			temp8 = ((encData[3]>>4) & 0xFF);
			//if ((temp8 > 95) || (e[3] > 95))
			//	{
			//		error = 1;
			//	}
			if(e[3] > temp8)
			{
				axis4(1);
			}
			if (e[3] < temp8)
			{
				axis4(2);
			}
			if (e[3] == temp8)
			{
				axis4(0);
				rab[3] = 0;
			}
			if (error) //��������� ������!!!
			{
				axis4(0);
				rab[3] = 0;
				error = 0;
			}
		}

		if (rab[4])
		{
			temp8 = ((encData[4]>>5) & 0xFF);
			//if ((temp8 > 100) || (e[4] > 100))
			//	{
			//		error = 1;
			//	}
			if(e[4] > temp8)
			{
				axis5(1);
			}
			if (e[4] < temp8)
			{
				axis5(2);
			}
			if (e[4] == temp8)
			{
				axis5(0);
				rab[4] = 0;
			}
			if (error) //��������� ������!!!
			{
				axis5(0);
				rab[4] = 0;
				error = 0;
			}
		}

		if (rab[5])
		{
			temp8 = ((encData[5]>>2) & 0xFF);
			//if ((temp8 > 200) || (e[5] > 200))
			//	{
			//		error = 1;
			//	}
			if(e[5] > temp8)
			{
				claw(2);//���������
			}
			if (e[5] < temp8)
			{
				claw(1);//���������
			}
			if (e[5] == temp8)
			{
				claw(0);
				rab[5] = 0;
			}
			if (error) //��������� ������!!!
			{
				claw(0);
				rab[5] = 0;
				error = 0;
			}
		}
									
		switch (flag)
				{
				case 0: 
								flag = 1;
								data1_one = ((encData[0]>>6) & 0xFF);
								data2_one = ((encData[1]>>5) & 0xFF);
								data3_one = ((encData[2]>>6) & 0xFF);
								data4_one = ((encData[3]>>4) & 0xFF);
								data5_one = ((encData[4]>>5) & 0xFF);
								data6_one = ((encData[5]>>2) & 0xFF);
								break;
				case 1:
								if (cnt_base > 50000)
									{
										cnt_base = 0;
										flag = 0;
										if (rab[0])
										{
											temp8 = ((encData[0]>>6) & 0xFF);
											if (e[0] != temp8)
											{
											if (data1_one == ((encData[0]>>6) & 0xFF))
												{
													platform(0);
													error = 1;
													//flag = 0;
													rab[0] = 0;
												}
											}
										}

										if (rab[1])
										{
											temp8 = ((encData[1]>>5) & 0xFF);
											if (e[1] != temp8)
											{
											if (data2_one == ((encData[1]>>5) & 0xFF))
												{
													axis2(0);
													error = 1;
													//flag = 0;
													rab[1] = 0;
												}
											}
										}

										if (rab[2])
										{
											temp8 = ((encData[2]>>6) & 0xFF);
											if (e[2] != temp8)
											{
											if (data3_one == ((encData[2]>>6) & 0xFF))
												{
													axis3(0);
													error = 1;
													//flag = 0;
													rab[2] = 0;
												}
											}
										}

										if (rab[3])
										{
											temp8 = ((encData[3]>>4) & 0xFF);
											if (e[3] != temp8)
											{
											if (data4_one == ((encData[3]>>4) & 0xFF))
												{
													axis4(0);
													error = 1;
													//flag = 0;
													rab[3] = 0;
												}
											}
										}

										if (rab[4])
										{
											temp8 = ((encData[4]>>5) & 0xFF);
											if (e[4] != temp8)
											{
											if (data5_one == ((encData[4]>>5) & 0xFF))
												{
													axis5(0);
													error = 1;
													//flag = 0;
													rab[4] = 0;
												}
											}
										}

										if (rab[5])
										{
											temp8 = ((encData[5]>>2) & 0xFF);
											if (e[5] != temp8)
											{
											if (data6_one == ((encData[5]>>2) & 0xFF))
												{
													claw(0);
													error = 1;
													//flag = 0;
													rab[5] = 0;
												}
											}
										}


									}//end if cnt_base
								else
									{
										cnt_base++;
									}
					}//end switch
			
				
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
		/*if (Receive_Buffer[0]==0) // ��������� �����, ������������ ������������ ��������� ���������
        {
            switch (Receive_Buffer[1])
			{
            case 1: 
                Send_Buffer[0] = ((encData[0] >> 6) & 0xFF);
                Send_Buffer[1] = 180;
                mail = 1;
                break;
            case 2: 
                Send_Buffer[0] = ((encData[1] >> 5) & 0xFF);
            Send_Buffer[1] = 160;
            mail = 1;
            break;
            case 3: Send_Buffer[0] = ((encData[2] >> 6) & 0xFF);
            Send_Buffer[1] = 110;
            mail = 1;
            break;
            case 4: Send_Buffer[0] = ((encData[3] >> 4) & 0xFF);
            Send_Buffer[1] = 80;
            mail = 1;
            break;
            case 5: Send_Buffer[0] = ((encData[4] >> 5) & 0xFF);
            Send_Buffer[1] = 250;
            mail = 1;
            break;
            case 6: Send_Buffer[0] = ((encData[5] >> 2) & 0xFF);
            Send_Buffer[1] = 200;
            mail = 1;
            break;
            case 7: //error = 0;
              platform(0);
              axis2(0);
              axis3(0);
              axis4(0);
              axis5(0);
              claw(0);
              Send_Buffer[0] = 0;
              Send_Buffer[1] = 0;
              //Send_Buffer[2] = 0;
              //Send_Buffer[3] = 0;
              
            default:;
					}

            }
    }//end #0
      
// ****** ������ -- ����� ������ 					
								
    if (Receive_Buffer[0] == 1) 
    {
        switch (Receive_Buffer[1])
		{
			case 0: 
                rab[0] = 0;
                rab[1] = 0;
                rab[2] = 0;
                rab[3] = 0;
                rab[4] = 0;
                rab[5] = 0;
                platform(0);
                axis2(0);
                axis3(0);
                axis4(0);
                axis5(0);
                claw(0);
                break;					
			case 1:
                if (!rab[0])
                {													
                    rab[0] = 1;
                    // platform(2);
                    e[0] = Receive_Buffer[2];//���������� ���� 
                }
                break;
                
            case 2:
                if (!rab[1])
                {													
                    rab[1] = 1;
                    e[1] = Receive_Buffer[2];//���������� ���� 
                } 
                break;
            case 3:
                if (!rab[2])
                {													
                    rab[2] = 1;
                    e[2] = Receive_Buffer[2];//���������� ���� 
                }
                break;
            case 4:
                if (!rab[3])
                {													
                    rab[3] = 1;
                    e[3] = Receive_Buffer[2];//���������� ���� 
                }
                //axis4(2);
                break;
                
            case 5:
                if (!rab[4])
                {													
                    rab[4] = 1;
                    e[4] = Receive_Buffer[2];//���������� ���� 
                }
                //axis5(2);
                break;
            case 6: 	 //otkrivaetsa
                if (!rab[5])
                {													
                    rab[5] = 1;
                    e[5] = Receive_Buffer[2];//���������� ���� 
                }
                //claw(1);
                break;
            default:;
        }//end switch
    }//end if
											
      / *Check to see if we have data yet * /
      / *if (mail && (Receive_length  != 0))
      {
		if (packet_sent == 1)
          CDC_Send_DATA (Send_Buffer, 2); //Receive_length);
        Receive_length = 0;
        mail = 0;
      }*/
    }			
			
    }
}
 


//������� ��������� �����
void zero (void) //pfgecrfnm 2-3 ����
{
		uint8_t p = 0;
		uint16_t cnt_dick = 0;
		uint8_t dick = 1; //��� �����
		uint8_t cl = 0;
	//	uint8_t p1 = 0;
		uint8_t cn1 = 0;
		
	//	platform(2);//������� ����) 
	//	axis2(1);
	//	axis3(2);
	//	axis4(2);
	//	axis5(2);
	//	claw(1);
    sendText("Axis 2 to zero. ");
	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_10))
	{
		axis2(1);

		if (!GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_9))
		{	
			axis2(0);
			encData[2] = 0;
			while (encData[2] != ((0x28<<6) & 0xFFFF))
				{
					Enc3();
					axis3(1);
				}
			axis3(0);
		}
	}
	axis2(0);
    
    sendText("Axis 2&3 ok. ");
        
	encData[1] = 0;

    sendText("Axis 5 to zero. ");
	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_14))
	{
		axis5(2);
	}
	axis5(0);
    
    sendText("Axis 5 ok. ");
    
	encData[4] = 0;

    sendText("Axis 4 to zero. ");
	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_8))
	{
		axis4(2);
		if (!GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_8)){
			cn1 = 10;
			while (cn1>0){cn1--;}
			axis4(0);}
	}
	axis4(0);
    sendText("Axis 4 ok. ");
	encData[3] = 0;
	
    sendText("Axis 3 to zero.");
	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_9))
	{
		axis3(2);
	}
	axis3(0);
    sendText("Axis 3 ok. ");
	encData[2] = ((0x28<<6) & 0xFFFF); //40

    sendText("Axis 1 to zero. ");
	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_11)) // ������ ���������
	{
		EncPlatform();

		if (!p)
		{
			platform(2);
		}

		switch (dick)
					{
					case 0: 
									fll = ((encData[0]>>6) & 0xFF);
									dick = 1;
									break;
					case 1:
									if (cnt_dick > 50000)
										{
											cnt_dick = 0;
											dick = 0;
											
											if (fll == ((encData[0]>>6) & 0xFF))
											{
												platform(1);
												p = 1;
											}
										}
									else
										{
											cnt_dick++;
										}
					}
	}	
	platform(0);
    sendText("Axis 1 to zero. ");
    
	encData[0] = ((0x55<<6) & 0xFFFF); // 85 �������� ����� �� 6! ��� 120??!!
	dick = 0;
	cnt_dick = 0;

	while (!cl)
	{
		EncClaw();
		claw(1);

		switch (dick)
					{
					case 0: 
									fll = ((encData[5]>>2) & 0xFF);
									dick = 1;
									break;
					case 1:
									if (cnt_dick > 50000)
										{
											cnt_dick = 0;
											dick = 0;
											
											if (fll == ((encData[5]>>2) & 0xFF))
											{
												cl = 1;
											}
										}
									else
										{
											cnt_dick++;
										}
					}
	}	
	claw(0);
	encData[5] = 0;

} // end zero(0);


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
