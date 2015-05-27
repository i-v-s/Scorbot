/*
  ******************************************************************************
  * File Name          : mx_gpio.c
  * Date               : 29/09/2013 02:25:48
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  *
  * COPYRIGHT 2013 STMicroelectronics
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mx_gpio.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

void mx_pinout_config(void) {
	/* Private typedef ---------------------------------------------------------*/
	GPIO_InitTypeDef GPIO_InitStruct;


	/** TIM1 GPIO Configuration	
		 PE9	 ------> TIM1_CH1
		 PE11	 ------> TIM1_CH2
	*/
/*

	//Enable or disable the AHB peripheral clock 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_2);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_2);


	// TIM2 GPIO Configuration	
		// PA0	 ------> TIM2_CH1_ETR
		// PA1	 ------> TIM2_CH2
	


	//Enable or disable the AHB peripheral clock 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_1);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_1);


	// TIM3 GPIO Configuration	
		// PE2	 ------> TIM3_CH1
		// PE3	 ------> TIM3_CH2
	


	//Enable or disable the AHB peripheral clock 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; //NOPULL
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; //NOPULL
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource2, GPIO_AF_2);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource3, GPIO_AF_2);


	// TIM4 GPIO Configuration	
	// PD12	 ------> TIM4_CH1
		// PD13	 ------> TIM4_CH2
	


	//Enable or disable the AHB peripheral clock 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_2);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_2);


	// TIM8 GPIO Configuration	
	//	 PC6	 ------> TIM8_CH1
	//	 PC7	 ------> TIM8_CH2
	


	//Enable or disable the AHB peripheral clock 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);


	//!!! GPIO_Init is commented because some parameters are missing
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_4);

	//Configure GPIO pin alternate function 
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_4);
*/

	/** Configure pins as GPIO
		 PE4	 ------> GPIO_Output
		 PE8	 ------> GPIO_Output ок LD4
		 PE10	 ------> GPIO_Output
		 PE12	 ------> GPIO_Output
		 PE13	 ------> GPIO_Output
		 PE14	 ------> GPIO_Output ок LD8
		 PE15	 ------> GPIO_Output ок LD6
		 PD8	 ------> GPIO_Input
		 PD9	 ------> GPIO_Input
		 PD10	 ------> GPIO_Input
		 PD11	 ------> GPIO_Input
		 PD14	 ------> GPIO_Input
		 PA13	 ------> GPIO_Output
		 PF6	 ------> GPIO_Output
		 PA14	 ------> GPIO_Output
		 PA15	 ------> GPIO_Output
		 PC10	 ------> GPIO_Output
		 PC11	 ------> GPIO_Output
		 PC12	 ------> GPIO_Output
		 PD0	 ------> GPIO_Output
		 PD1	 ------> GPIO_Output
		 PD2	 ------> GPIO_Output
		 PD3	 ------> GPIO_Output
		 PD4	 ------> GPIO_Output
		 PD5	 ------> GPIO_Output
		 PD6	 ------> GPIO_Output
		 PD7	 ------> GPIO_Output
		 PB3	 ------> GPIO_Output
		 PB4	 ------> GPIO_Output
		 PB5	 ------> GPIO_Output
		 PE0	 ------> GPIO_Output
     PC9	 ------> GPIO_Output
		 
		 энкодер для оси #6
		 PF9   ------> GPIO_Input
		 PF10  ------> GPIO_Input
		 
		 энкодеры:
		 PE2	 ------> TIM3_CH1
		 PE3	 ------> TIM3_CH2
		 
		 PC6	 ------> TIM8_CH1
		 PC7	 ------> TIM8_CH2
		 
		 PD12	 ------> TIM4_CH1
		 PD13	 ------> TIM4_CH2
		 
		 PA0	 ------> TIM2_CH1_ETR
		 PA1	 ------> TIM2_CH2
		 
		 PE9	 ------> TIM1_CH1
		 PE11	 ------> TIM1_CH2
	*/


	/*Enable or disable the AHB peripheral clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE|RCC_AHBPeriph_GPIOD|RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOF|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOB, ENABLE);

	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_8|GPIO_Pin_10|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15|GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOF, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
