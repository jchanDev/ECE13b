/*
 * ECE 153B
 *
 * Name(s): Ishaan Joshi, Julia Chan
 * Section:
 * Project
 */

#include "stm32l476xx.h"
#include "SysClock.h"
#include "SysTimer.h"
#include "LED.h"
#include "DMA.h"
#include "UART.h"
#include "motor.h"
#include "SPI.h"
#include "I2C.h"
#include "accelerometer.h"
#include <stdio.h>

static char buffer[IO_SIZE];
static uint32_t BluetoothReceived = 0;	
static uint32_t TermiteSent = 0;	
static uint32_t lessFromConsole = 0;	
static uint32_t moreFromConsole = 0;	
static uint32_t lessFromTemp = 0;	
static uint32_t moreFromTemp = 0;
static int delayFlag = 0;
static int currentlyRunning;
static int maxTemp = 0;
static int prevTemp = 0;
static int currentStage = 0;
static int timerFlag = 0;
static int pauseFlag = 0;

//void UART_onInput(char* inputs, uint32_t size) {
//	//TODO
//}

void completeBluetooth(void) {
	BluetoothReceived = 1;
}

void completeTermite(void) {
	TermiteSent = 1;
}

void setTimerFlag(void) {
	timerFlag = 1;
}

void clearTimerFlag(void) {
	timerFlag = 0;
}

void consoleSignal(int type) {
	if (type == 1) {
		lessFromConsole = 1;
	}
	else if (type == 2) {
		moreFromConsole = 1;
	}
	else {
		lessFromConsole = 0;
		moreFromConsole = 0;
	}
}

void delaySignal(int input) {
	// 1 if not allowed to run
	// 2 if allowed to run
	// 0 if delay2 not called
	delayFlag = input;
}

void pauseSignal(int input) {
	// 1 if paused
	// 0 if not paused
	pauseFlag = input;
}

void checkTemp(int temp) {
	char buffer[100];
	if (temp != prevTemp) {
		sprintf(buffer, "Temperature changed! Temp = %d\n", temp);
		UART_print(buffer);
	}
	if (temp - maxTemp < 0) {
		if (maxTemp > 30 && temp == 28) {
			moreFromTemp = 1;
			maxTemp = 0;
		}
		else if (maxTemp == 28 && temp == 26) {
			lessFromTemp = 1;
			maxTemp = 0;
		}
		else {
			lessFromTemp = 0;
			moreFromTemp = 0;
		}
	}
	if (temp > maxTemp) {
		maxTemp = temp;
	}
	prevTemp = temp;
}

int main(void) {
	// Switch System Clock = 80 MHz
	System_Clock_Init(); 
	Motor_Init();
	SysTick_Init();
	UART1_GPIO_Init();
	UART1_Init();
	USART_Init(USART1);
	DMA_Init_UARTx(DMA1_Channel4, USART1);
//	UART2_GPIO_Init();
//	UART2_Init();	
//	USART_Init(USART2);
//	DMA_Init_UARTx(DMA1_Channel7, USART2);
	 
	LED_Init();	
	SPI1_GPIO_Init();
	SPI1_Init();
	initAcc();
	I2C_GPIO_Init();
	I2C_Initialization();

	
	
	sprintf(buffer, "Termite Program Starts.\r\n");
	UART_print(buffer);
//	sprintf(buffer, "Program Again.\r\n");
//	UART_print(buffer);
//	delay(50);
//	transfer_data('L');
//	transfer_data('\n');
//	transfer_data('X');
//	transfer_data('\n');

// For I2C Communication:
	uint8_t SecondaryAddress = 0b1001000 << 1;
	uint8_t temperature = 0;
	uint8_t Data_Send = 0b00000000;
	
	// Testing serial input with new delay
	LED_On();
//	transfer_data('L');
//	transfer_data('\n');
	LED_Off();
	
	while(1) {
		//TODO
		LED_Toggle();
		double x, y, z;
		readValues(&x, &y, &z);
//		sprintf(buffer, "Acceleration hello: %.2f, %.2f, %.2f\r\n", x, y, z);
//		UART_print(buffer);
		// Send command to sensor to read temp
		I2C_SendData(I2C1, SecondaryAddress, &Data_Send, 1);
		// Get measurement in variable Data_Receive
		I2C_ReceiveData(I2C1, SecondaryAddress, &temperature, 1);
		checkTemp(temperature);
		
		uint32_t delayValue = 1000;
		int routineReturnVal = 0;
		delay(50);
		
//		sprintf(buffer, "Hello hello.\r\n");
//		UART_print(buffer);
		
//		if (readTime() == 3000) {
//			endTimer();
//			timerFlag = 0;
//		}
//		
//		if (timerFlag) {
//			continue;
//		}
		// Print measurement via UART
//		sprintf(buffer, "Temperature: %d\n", temperature);
//		UART_print(buffer);
		

//		sprintf(buffer, "%d %d %d %d \n", lessFromConsole, moreFromConsole, lessFromTemp, moreFromTemp);
//		UART_print(buffer);
		if (!pauseFlag) {
			if ((lessFromConsole && !moreFromConsole && !lessFromTemp && !moreFromTemp && !currentlyRunning)) {
				sprintf(buffer, "You wanted less candy? :o\r\n");
				UART_print(buffer);
				currentlyRunning = 1;
				currentStage = 1;
				lessFromConsole = 0;
			}
			else if ((!lessFromConsole && moreFromConsole && !lessFromTemp && !moreFromTemp) && !currentlyRunning) {
				sprintf(buffer, "You demanded more candy!\r\n");
				UART_print(buffer);
				currentlyRunning = 2;
				currentStage = 1;
				lessFromConsole = 0;
			}
			else if ((!lessFromConsole && !moreFromConsole && lessFromTemp && !moreFromTemp) && !currentlyRunning) {
				sprintf(buffer, "Your cold hands got you less candy!\r\n");
				UART_print(buffer);
				currentlyRunning = 1;
				currentStage = 1;
				lessFromConsole = 0;
			}
			else if ((!lessFromConsole && !moreFromConsole && !lessFromTemp && moreFromTemp) && !currentlyRunning) {
				sprintf(buffer, "Your hot hands got you more candy!\r\n");
				UART_print(buffer);
				currentlyRunning = 2;
				currentStage = 1;
				lessFromConsole = 0;
			}
			else if (currentlyRunning) {
				
			}
			else if (!lessFromConsole && !moreFromConsole && !lessFromTemp && !moreFromTemp) {
				
			}
			else {
				sprintf(buffer, "Invalid Input.\r\n");
				UART_print(buffer);
				lessFromConsole = 0;
				moreFromConsole = 0;
				lessFromTemp = 0;
				moreFromTemp = 0;
			}
			
			if (currentlyRunning == 1 && (delayFlag == 2 || delayFlag == 0)) {
				routineReturnVal = lessCandyRoutine(currentStage, x, y);
				if (routineReturnVal) {
					// Switch to next stage
					delay2(delayValue, 1);
					currentStage++;
				}
				if (currentStage == 5) {
					// Routine has completed
					currentlyRunning = 0;
					currentStage = 0;
					sprintf(buffer, "Candy dispensed!\r\n");
					UART_print(buffer);
				}
				lessFromConsole = 0;
				moreFromConsole = 0;
				lessFromTemp = 0;
				moreFromTemp = 0;
			}
			else if (currentlyRunning == 2  && (delayFlag == 2 || delayFlag == 0)) {
				routineReturnVal = moreCandyRoutine(currentStage, x, y);
				if (routineReturnVal) {
					// Switch to next stage
					delay2(delayValue, 1);
					currentStage++;
				}
				if (currentStage == 5) {
					// Routine has completed
					currentlyRunning = 0;
					currentStage = 0;
					sprintf(buffer, "Candy dispensed!\r\n");
					UART_print(buffer);
				}
				lessFromConsole = 0;
				moreFromConsole = 0;
				lessFromTemp = 0;
				moreFromTemp = 0;
			}
			else {
				delay2(delayValue, 0);
			}
		}
		else {
			if ((lessFromConsole && !moreFromConsole && !lessFromTemp && !moreFromTemp && !currentlyRunning)) {
				sprintf(buffer, "Continuing less candy!\r\n");
				UART_print(buffer);
				currentlyRunning = 1;
				currentStage = currentStage;
				lessFromConsole = 0;
			}
			else {
				setDire(0);
			}
		}
//		sprintf(buffer, "i am here.\r\n");
//		UART_print(buffer);
//		delay(1000);
		
		
		// Some delay
		//for (int i = 0; i < 5000; ++i);
		
	}
}

