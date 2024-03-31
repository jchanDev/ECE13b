/*
 * ECE 153B
 *
 * Name(s): Ishaan Joshi, Julia Chan
 * Section: Tues 7PM
 * Lab: Project
 */

#include "LED.h"

void LED_Init(void) {
	// Enable GPIO Clocks
	// [TODO]
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	
	// Initialize Green LED
	// [TODO]
		GPIOA->MODER &= ~GPIO_MODER_MODE5_1;
		GPIOA->MODER |= GPIO_MODER_MODE5_0;
		GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;
		GPIOA->PUPDR &= ~ GPIO_PUPDR_PUPD5;
}

void LED_Off(void) {
	// [TODO]
	GPIOA->ODR &= ~GPIO_ODR_OD5;
}

void LED_On(void) {
	// [TODO]
	GPIOA->ODR |= GPIO_ODR_OD5;
}

void LED_Toggle(void) {
	// [TODO]
	GPIOA->ODR ^= GPIO_ODR_OD5;
}