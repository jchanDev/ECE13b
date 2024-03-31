/*
 * ECE 153B
 *
 * Name(s): Ishaan Joshi, Julia Chan
 * Section:	 Tues 7PM
 * Project
 */

#include "stm32l476xx.h"
#include "motor.h"
#include "UART.h"
#include "accelerometer.h"
#include "SysTimer.h"
#include <stdio.h>

static const uint32_t MASK = 0x0360;
static const uint32_t HalfStep[8] = {0x0220,0x0020,0x0120,0x0100,0x0140,0x0040,0x0240,0x0200};

static volatile int8_t dire = 0;
static volatile int8_t currStepCounter = 0;
void Motor_Init(void) {	
	// Enable GPIO port C Clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	// Enable pins 5,6,8,9 of port C to output
	GPIOC->MODER &= ~GPIO_MODER_MODE5_1;
	GPIOC->MODER |= GPIO_MODER_MODE5_0;
	GPIOC->MODER &= ~GPIO_MODER_MODE6_1;
	GPIOC->MODER |= GPIO_MODER_MODE6_0;
	GPIOC->MODER &= ~GPIO_MODER_MODE8_1;
	GPIOC->MODER |= GPIO_MODER_MODE8_0;
	GPIOC->MODER &= ~GPIO_MODER_MODE9_1;
	GPIOC->MODER |= GPIO_MODER_MODE9_0;
	// Set OSPEEDR so output speed of the pins to fast
	// Chapter 9.4 in the STM32 reference manual
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED5;
//	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED5_0;
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED6;
//	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_0;
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED8;
//	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED8_0;
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED9;
//	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED9_0;
	// Set otyper for pins to push pull
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT5;
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT6;
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT8;
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT9;
	//Set output type of the pins to no pull-up, no pull-down
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD5;
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD6;
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD8;
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD9;
}

void rotate(void) {
	if(currStepCounter == 8){
		currStepCounter = 0;
	}
	else if(currStepCounter == -1){
		currStepCounter = 7;
	}
	GPIOC->ODR = HalfStep[currStepCounter];
//	printf("currStepCounter = %d\n",currStepCounter);
//	printf("Array Value = %d\n",HalfStep[currStepCounter]);
	
	if(dire == 1){
		// Counterclockwise
		currStepCounter++;
	}
	else if (dire == 2) { 
		// Clockwise
		currStepCounter--;
	}
	else if (dire == 0 || dire == 3) {
		// Pause
		currStepCounter = currStepCounter;
	}
}

void setDire(int8_t direction) {
	dire = direction;
}

// Returns 1 if current acc vector is in intended range
// Returns 0 otherwise
int turnLeft(double endX, double endY, double currentX, double currentY) {
	// Find range of values that motor should spin to
	// I'm using a constant +- 0.05 g's from where it *should* be
	double endXBeginRange = endX - 0.1;
	double endXEndRange = endX + 0.1;
	double endYBeginRange = endY - 0.1;
	double endYEndRange = endY + 0.1;
	delay(50);

	int xInRange = (currentX > endXBeginRange) && (currentX < endXEndRange);
	int yInRange = (currentY > endYBeginRange) && (currentY < endYEndRange);
	if (xInRange && yInRange) {
		// Once accelerometer has reached valid position, pause
		setDire(0);
	}
	else {
		// Set direction to left
		setDire(1);
	}
	return xInRange && yInRange;
}

int turnRight(double endX, double endY, double currentX, double currentY) {
	// I'm using a constant +- 0.05 g's from where it *should* be
	double endXBeginRange = endX - 0.1;
	double endXEndRange = endX + 0.1;
	double endYBeginRange = endY - 0.1;
	double endYEndRange = endY + 0.1;
	int xInRange = (currentX > endXBeginRange) && (currentX < endXEndRange);
	int yInRange = (currentY > endYBeginRange) && (currentY < endYEndRange);
	if (xInRange && yInRange) {
		// Once accelerometer has reached valid position, pause
		setDire(0);
	}
	else {
		// Set direction to right
		setDire(2);
	}
	return xInRange && yInRange;
}
	

// Function to turn motor to dispense less candy
// Returns 0 if not ready to switch stage
// Returns 1 if ready to switch stage
// Returns -1 on bad stage input
int lessCandyRoutine(int stage, double currentX, double currentY) {
	if (stage == 1) {
		// Getting to Base State 0: Ready to Begin (Machine should be here but just make sure)
		// ax = 0g, ay = -1g
		return turnLeft(0, -1, currentX, currentY);
	}
	else if (stage == 2) {
		// Getting to State 1: Picking up Candy
		// ax = 1g, ay = 0g
		return turnLeft(1, 0, currentX, currentY);
	}
		
	else if (stage == 3) {
		// Getting to State 2: Dropping Candy
		// ax = -0.7g, ay = -0.7g
		return turnRight(-0.7, -0.7, currentX, currentY);
		// Long Delay to let human pick up candy
		//	startTimer(); TODO: put in main
	}
	
	else if (stage == 4) {
		// Returning to Base State 0
		// ax = 0g, ay = -1g
		return turnLeft(0, -1, currentX, currentY);
	}
	else {
		return -1; // error
	}
}

// Function to turn motor to dispense more candy
// Returns 0 if not ready to switch stage
// Returns 1 if ready to switch stage
// Returns -1 on bad stage input
int moreCandyRoutine(int stage, double currentX, double currentY) {
	if (stage == 1) {
		// Getting to Base State 0: Ready to Begin (Machine should be here but just make sure)
		// ax = 0g, ay = -1g
		return turnLeft(0, -1, currentX, currentY);
	}
	else if (stage == 2) {
		// Getting to State 1: Picking up Candy
		// ax = -1g, ay = 0g
		return turnRight(-1, 0, currentX, currentY);
	}
		
	else if (stage == 3) {
		// Getting to State 2: Dropping Candy
		// ax = 0.55g, ay = 0.83g
		return turnRight(0.55, 0.83, currentX, currentY);
		// Long Delay to let human pick up candy
		//	startTimer(); TODO: put in main
	}
	
	else if (stage == 4) {
		// Returning to Base State 0
		// ax = 0g, ay = -1g
		return turnLeft(0, -1, currentX, currentY);
	}
	else {
		return -1; // error
	}
}
