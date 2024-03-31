/*
 * ECE 153B
 *
 * Name(s): Ishaan Joshi, Julia Chan
 * Section:
 * Project
 */

#include "SysTimer.h"
#include "motor.h"

static uint32_t volatile step;
static uint32_t volatile step2;
static uint32_t volatile timeCounter;

void SysTick_Init(void) {
	// SysTick Control & Status Register
	SysTick->CTRL = 0; // Disable SysTick IRQ and SysTick Counter
	
  SysTick->VAL = 0;
	SysTick->LOAD = 79999;  // 1ms period
	
	// Enables SysTick exception request
	// 1 = counting down to zero asserts the SysTick exception request
	// 0 = counting down to zero does not assert the SysTick exception request
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	
	// Select clock source
	// If CLKSOURCE = 0, the external clock is used. The frequency of SysTick clock is the frequency of the AHB clock divided by 8.
	// If CLKSOURCE = 1, the processor clock is used.
	// TODO
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
	
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	
	// Configure and Enable SysTick interrupt in NVIC
	NVIC_EnableIRQ(SysTick_IRQn);
	NVIC_SetPriority(SysTick_IRQn, 0); // Set Priority to 0
}

void SysTick_Handler(void) {
	++step;
	++step2;
	++timeCounter;
	//rotate();
	if (timeCounter == 3){
		timeCounter = 0;
		rotate();
	}
}

// in milliseconds
void delay(uint32_t T) {
//	step = 0;
//	SysTick->VAL = 0;
//	SysTick->LOAD = 79999;  // 1ms period?

//	// Enable SysTick
//	// SysTick is not enabled before this because we are using it for 2 different functions at different times
//	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

//	// Wait for counter to reach desired value
//	while (step <= T);
//	step=0;

//	// Disable SysTick for future use
//	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	
	// One-function SysTick version of delay
	step = 0;
	while (step <= T);
	step = 0;
}

void delay2(uint32_t T, int initOrNot) {
	if (initOrNot) {
		// initializing timer
		step2 = 0;
		delaySignal(1);
		return;
	}
	if (step2 >= T){
		step2 = 0;
		// condition met
		delaySignal(2);
	}
	else {
		// condition not met yet
		delaySignal(1);
	}
}