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
/* посмотреть относительное перемещение, где num_enc от 1 до 5 (для оси "claw"/#6 обработка энкодера не организована)
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
				// для хранения предыдущего значения
				CNT_last1 = 0;
					
				// текущее значение
				CNT_now = TIM1->CNT;
				
				// посчитаем перемещение
				CNT_diff = (int16_t)(CNT_now - CNT_last1);
					
				// сохраним текущее положение как предыдущее
				CNT_last1 = CNT_now;
					
				// вернем перемещение
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

// посмотреть положение энкодера
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
void timer_CheckEncoder(void)//функция для проверки энкодеров
	{
	int16_t diff = enc_GetRelativeMove(3);
	int16_t pos = enc_GetPosition(3);
    
	// нули на светодиоды
		GPIOE->BRR = GPIO_Pin_8;//led2
		GPIOE->BRR = GPIO_Pin_15;//led1
		GPIOE->BRR = GPIO_Pin_14;//led3
    
	// если начальная позиция отлична от нуля -- зажигаем led1
	if(pos > 0) {GPIOE->BSRR = GPIO_Pin_15; GPIOE->BRR = GPIO_Pin_8; GPIOE->BRR = GPIO_Pin_14;}
    
	// если относительное перемещение больше нуля, то зажигаем led2
	if(diff > 0) {GPIOE->BSRR = GPIO_Pin_8; GPIOE->BRR = GPIO_Pin_15; GPIOE->BRR = GPIO_Pin_14;}

	// если перемещение было произведено в обратную сторону -- зажигаем LED3
	if (diff < 0) {GPIOE->BSRR = GPIO_Pin_14; GPIOE->BRR = GPIO_Pin_8; GPIOE->BRR = GPIO_Pin_15;}
}
*/

int main(void)
{
    /*char t[] = "ptp a20 save 70 ptp B 34  ;";
    const char * er = parse(t, t + sizeof(t));
    
    if(er) printf(er);*/
    
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
    mx_pinout_config();//gpio init

    while (1)
    {
		/*if (nozero)
		{
			zero(); 
			zero();
			zero();
			nozero = 0;
		}*/

		//считаем энкодеры
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
				/*отправляем данные о положении энкодера
		if (Receive_Buffer[0] == 1)
		{
			monit = Receive_Buffer[1];
			if (monit > 10)  
			{
				monit = monit/10;
			}//если в обратную сторону, то делим на 10 (44/4=4 -- энкодер #4)
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
			if (error) //Обработка ошибки!!!
			{
				platform(0);
				rab[0] = 0;
				error = 0;
			}
		}

		if (rab[1])
		{
			temp8 = ((encData[1] >> 5) & 0xFF);
			//if ((temp8 > 205) || (e[1] > 205))//уточнить!!
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
			if (error) //Обработка ошибки!!!
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
			if (error) //Обработка ошибки!!!
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
			if (error) //Обработка ошибки!!!
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
			if (error) //Обработка ошибки!!!
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
				claw(2);//закрываем
			}
			if (e[5] < temp8)
			{
				claw(1);//открываем
			}
			if (e[5] == temp8)
			{
				claw(0);
				rab[5] = 0;
			}
			if (error) //Обработка ошибки!!!
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
		/*if (Receive_Buffer[0]==0) // установка нулей, установление центаральных положений счетчиков
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
      
// ****** запись -- номер мотора 					
								
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
                    e[0] = Receive_Buffer[2];//запоминаем угол 
                }
                break;
                
            case 2:
                if (!rab[1])
                {													
                    rab[1] = 1;
                    e[1] = Receive_Buffer[2];//запоминаем угол 
                } 
                break;
            case 3:
                if (!rab[2])
                {													
                    rab[2] = 1;
                    e[2] = Receive_Buffer[2];//запоминаем угол 
                }
                break;
            case 4:
                if (!rab[3])
                {													
                    rab[3] = 1;
                    e[3] = Receive_Buffer[2];//запоминаем угол 
                }
                //axis4(2);
                break;
                
            case 5:
                if (!rab[4])
                {													
                    rab[4] = 1;
                    e[4] = Receive_Buffer[2];//запоминаем угол 
                }
                //axis5(2);
                break;
            case 6: 	 //otkrivaetsa
                if (!rab[5])
                {													
                    rab[5] = 1;
                    e[5] = Receive_Buffer[2];//запоминаем угол 
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
 


//функция установки нулей
void zero (void) //pfgecrfnm 2-3 раза
{
		uint8_t p = 0;
		uint16_t cnt_dick = 0;
		uint8_t dick = 1; //для нулей
		uint8_t cl = 0;
	//	uint8_t p1 = 0;
		uint8_t cn1 = 0;
		
	//	platform(2);//двигаем тему) 
	//	axis2(1);
	//	axis3(2);
	//	axis4(2);
	//	axis5(2);
	//	claw(1);
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
	encData[1] = 0;

	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_14))
	{
		axis5(2);
	}
	axis5(0);
	encData[4] = 0;

	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_8))
	{
		axis4(2);
		if (!GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_8)){
			cn1 = 10;
			while (cn1>0){cn1--;}
			axis4(0);}
	}
	axis4(0);
	encData[3] = 0;

	/*
	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_8))
	{
		axis4(2);
	}
	axis4(0);
	encData[3] = 0;
*/
	
	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_9))
	{
		axis3(2);
	}
	axis3(0);
	encData[2] = ((0x28<<6) & 0xFFFF); //40

	while (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_11)) // кнопка платформы
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
	encData[0] = ((0x55<<6) & 0xFFFF); // 85 сдвигаем влево на 6! или 120??!!
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


/*-------------------функции управления осями-------------------------*/
//OK 2 -- по часовой, 1 -- против часовой, 0 -- стоп
void platform(uint8_t x){
	
		switch (x)
			{
			case 0:
					GPIOC->BRR = GPIO_Pin_10; //enabled
                  	GPIOA->BRR = GPIO_Pin_15; //move
                  	GPIOA->BRR = GPIO_Pin_8; //GPIO_Pin_14;
					break;
			
			case 1:
					GPIOC->BSRR = GPIO_Pin_10; //enabled
                  	GPIOA->BSRR = GPIO_Pin_15; //move
                  	GPIOA->BRR = GPIO_Pin_8; //GPIO_Pin_14;
					break;
					 
			case 2:
					GPIOC->BSRR = GPIO_Pin_10; //enabled
                  	GPIOA->BSRR = GPIO_Pin_8; //move
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
                GPIOA->BRR = GPIO_Pin_9;//GPIO_Pin_13; //move
                GPIOF->BSRR = GPIO_Pin_6;
				break;
		
		case 1:  
				GPIOC->BSRR = GPIO_Pin_9; //enabled
                GPIOA->BSRR = GPIO_Pin_9;//GPIO_Pin_13; //move
                GPIOF->BRR = GPIO_Pin_6;
				break;
		
		case 0:  
				GPIOC->BRR = GPIO_Pin_9; //enabled
                GPIOA->BRR = GPIO_Pin_9;//GPIO_Pin_13; //move
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
