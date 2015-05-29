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
#include "controller.h"

void m0f() // Вперёд
{
    GPIOA->BSRR = GPIO_Pin_4; //move
    GPIOA->BRR = GPIO_Pin_15;
    GPIOC->BSRR = GPIO_Pin_10; //enabled
}

void m0r() // Назад
{
    GPIOA->BSRR = GPIO_Pin_15; //move
    GPIOA->BRR = GPIO_Pin_4; //GPIO_Pin_14;    
    GPIOC->BSRR = GPIO_Pin_10; //enabled
}

void m0s() // Стоп
{
    GPIOC->BRR = GPIO_Pin_10; //enabled
    GPIOA->BRR = GPIO_Pin_15 | GPIO_Pin_4; //move
}

void m1f()
{
    GPIOB->BSRR = GPIO_Pin_4; //enabled
    GPIOB->BSRR = GPIO_Pin_3; //move
    GPIOD->BRR = GPIO_Pin_7;
}

void m1r()
{
    GPIOB->BSRR = GPIO_Pin_4; //enabled
    GPIOD->BSRR = GPIO_Pin_7; //move
    GPIOB->BRR = GPIO_Pin_3;
}

void m1s()
{
    GPIOB->BRR = GPIO_Pin_4 | GPIO_Pin_3;
    GPIOD->BRR = GPIO_Pin_7;
}

void m2f()
{
	GPIOD->BSRR = GPIO_Pin_0; //enabled
    GPIOC->BRR = GPIO_Pin_11; //move
    GPIOC->BSRR = GPIO_Pin_12;
}

void m2r()
{
    GPIOD->BSRR = GPIO_Pin_0; //enabled
    GPIOC->BSRR = GPIO_Pin_11; //move
    GPIOC->BRR = GPIO_Pin_12;
}

void m2s()
{
    GPIOD->BRR = GPIO_Pin_0; //enabled
    GPIOC->BRR = GPIO_Pin_11; //move
    GPIOC->BRR = GPIO_Pin_12;
}

void m3f()
{
    GPIOD->BSRR = GPIO_Pin_3; //enabled
    GPIOD->BSRR = GPIO_Pin_1; //move
    GPIOD->BRR = GPIO_Pin_2;
}

void m3r()
{
    GPIOD->BSRR = GPIO_Pin_2 | GPIO_Pin_3; //enabled
    GPIOD->BRR = GPIO_Pin_1;
}

void m3s()
{
    GPIOD->BRR = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3; //enabled
}


void m4f()
{
    GPIOD->BSRR = GPIO_Pin_6; //enabled
    GPIOD->BSRR = GPIO_Pin_4; //move
    GPIOD->BRR = GPIO_Pin_5;
}
		
void m4r()
{
    GPIOD->BSRR = GPIO_Pin_5 | GPIO_Pin_6; //enabled
    GPIOD->BRR = GPIO_Pin_4;
}

void m4s()
{
    GPIOD->BRR = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6; //enabled
}

void m5f()
{
    GPIOC->BSRR = GPIO_Pin_9; //enabled
    GPIOA->BRR = GPIO_Pin_5;//GPIO_Pin_13; //move
    GPIOF->BSRR = GPIO_Pin_6;
}

void m5r()
{
    GPIOC->BSRR = GPIO_Pin_9; //enabled
    GPIOA->BSRR = GPIO_Pin_5;//GPIO_Pin_13; //move
    GPIOF->BRR = GPIO_Pin_6;
}

void m5s()
{
    GPIOC->BRR = GPIO_Pin_9; //enabled
    GPIOA->BRR = GPIO_Pin_5;//GPIO_Pin_13; //move
    GPIOF->BRR = GPIO_Pin_6;
}

int a1sw()
{
    return GPIOD->IDR & GPIO_Pin_10;
}

int a2sw()
{
    return GPIOD->IDR & GPIO_Pin_9;
}


void mx_pinout_config(void) {
	/* Private typedef ---------------------------------------------------------*/
	GPIO_InitTypeDef GPIO_InitStruct;


	/*Enable or disable the AHB peripheral clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE|RCC_AHBPeriph_GPIOD|RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOF|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOB, ENABLE);

	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15|GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin */
	
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
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_4);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_4);
	
    // Энкодер оси 1 TIM3, AF2
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6| GPIO_Pin_7 | GPIO_Pin_0|GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;    
	GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_6);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_6);
    
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_2);
    
    //TIM3->SMCR 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8 | RCC_APB2Periph_TIM1, ENABLE);
    initEncoder(motors + 0, TIM3);
    initMotor(motors + 0, m0f, m0r, m0s);
    
    initEncoder(motors + 1, TIM2);
    initMotor(motors + 1, m1f, m1r, m1s);
    axes[1].getSwitch = a1sw;
    
    initEncoder(motors + 2, TIM1);
    initMotor(motors + 2, m2f, m2r, m2s);
    axes[2].getSwitch = a2sw;
    
    initEncoder(motors + 3, TIM4);
    initMotor(motors + 3, m3f, m3r, m3s);

    initEncoder(motors + 4, TIM8);
    initMotor(motors + 4, m4f, m4r, m4s);

    initMotor(motors + 5, m5f, m5r, m5s);
    
    
	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_15;
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
