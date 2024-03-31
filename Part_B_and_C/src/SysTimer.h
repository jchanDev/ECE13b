/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */

#ifndef __STM32L476G_NUCLEO_SYSTICK_H
#define __STM32L476G_NUCLEO_SYSTICK_H

#include "stm32l476xx.h"

void SysTick_Init(void);
void SysTick_Handler(void);
void delay(uint32_t ms);
void delay2(uint32_t ms, int initOrNot);
void setTimerFlag(void);
void clearTimerFlag(void);
void delaySignal(int input);

#endif
