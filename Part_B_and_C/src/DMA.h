/*
 * ECE 153B
 *
 * Name(s): Ishaan Joshi, Julia Chan
 * Section:
 * Project
 */

#ifndef __STM32L476G_DISCOVERY_DMA_H
#define __STM32L476G_DISCOVERY_DMA_H

#include "stm32l476xx.h"

void DMA_Init_UARTx(DMA_Channel_TypeDef * tx, USART_TypeDef * uart);

void DMA1_Channel4_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);

void completeBluetooth(void);
void completeTermite(void);

#endif /* __STM32L476G_DISCOVERY_DMA_H */