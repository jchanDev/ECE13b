/*
 * ECE 153B
 *
 * Name(s): Ishaan Joshi, Julia Chan
 * Section: Tues 7PM
 * Project
 */


#include "UART.h"
#include "DMA.h"
#include "SysTimer.h"
#include "motor.h"
#include <stdio.h>

// CHANGE DMA_CHANNEL FROM 7 <-> 4 FOR TERMITE <-> BLUETOOTH
static volatile DMA_Channel_TypeDef* tx = DMA1_Channel4;
static volatile char inputs[IO_SIZE];
static volatile uint8_t data_t_0[IO_SIZE];
static volatile uint8_t data_t_1[IO_SIZE];
static volatile uint8_t input_size = 0;
static volatile int8_t pending_size = 0;
static volatile uint8_t* active = data_t_0;
static volatile uint8_t* pending = data_t_1;
static volatile uint8_t* temp;

#define SEL_0 1
#define BUF_0_EMPTY 2
#define BUF_1_EMPTY 4
#define BUF_0_PENDING 8
#define BUF_1_PENDING 16

void transfer_data(char ch);
void on_complete_transfer(void);

void UART1_Init(void) {
	// Enable USART1 Clock
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	// Set system clock as USART1 source
	RCC->CCIPR &= ~RCC_CCIPR_USART1SEL_1;
	RCC->CCIPR |= RCC_CCIPR_USART1SEL_0;
}

void UART2_Init(void) {
	// Enable USART2 Clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
	// Set system clock as USART2 source
	RCC->CCIPR &= ~RCC_CCIPR_USART2SEL_1;
	RCC->CCIPR |= RCC_CCIPR_USART2SEL_0;
}

void UART1_GPIO_Init(void) {
	// PB6: Transmitter
	// PB7: Receiver

	// Turn on GPIO Port B Clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

	// Set Pins PB6 and PB7 to Alternative Function
	GPIOB->MODER |= GPIO_MODER_MODE6_1;
	GPIOB->MODER &= ~GPIO_MODER_MODE6_0;
	GPIOB->MODER |= GPIO_MODER_MODE7_1;
	GPIOB->MODER &= ~GPIO_MODER_MODE7_0;
	
	// Setting PB6, PB7 to Alternative Function Mode 7 (USART1) Both TX and RX
	// AFR[0] and AFRL because 6 and 7 fall in low-order range
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL6_3;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL6_2;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL6_1;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL6_0;
	
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL7_3;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL7_2;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL7_1;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL7_0;
	
	// Very high speed
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED6;
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED7;
	
	// Push-pull
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT6;
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT7;
	
	// Pull-Up
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD6_1;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD6_0;
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD7_1;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD7_0;
}

void UART2_GPIO_Init(void) {
	// PA2: Transmitter
	// PA3: Receiver
	// Turn on GPIO Port A Clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// Set Pins PA2 and PA3 to Alternative Function
	GPIOA->MODER |= GPIO_MODER_MODE2_1;
	GPIOA->MODER &= ~GPIO_MODER_MODE2_0;
	GPIOA->MODER |= GPIO_MODER_MODE3_1;
	GPIOA->MODER &= ~GPIO_MODER_MODE3_0;
	
	// Setting PA2, PA3 to Alternative Function Mode 7 (USART2) Both TX and RX
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL2_3;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_2;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_1;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_0;
	
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL3_3;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3_2;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3_1;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3_0;
	
	// Very high speed
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2;
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED3;
	
	// Push-pull
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT2;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT3;
	
	// Pull-Up
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD2_1;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD2_0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD3_1;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;
	
}

void USART_Init(USART_TypeDef* USARTx) {
	// TODO: Disable USART 1 and 2 like this?
	USARTx->CR1 &= ~USART_CR1_UE;
	
	// Word Length: 8 bits
	USARTx->CR1 &= ~USART_CR1_M1;
	
	// Oversampling mode: 16
	USARTx->CR1 &= ~USART_CR1_OVER8;
	
	// # Stop Bits: 1
	USARTx->CR2 &= ~USART_CR2_STOP;
	
	// Baud Rate: 9600
	// Divide fclk (80MHz) by 9600 to get 1667
	// Check if register should be different
	USARTx->BRR = 8333;
	
	// Enable Transmitter and Receiver
	USARTx->CR1 |= USART_CR1_TE;
	USARTx->CR1 |= USART_CR1_RE;
	
	// Enable DMA Transmission and Receiving
	USARTx->CR3 |= USART_CR3_DMAT;
	USARTx->CR3 |= USART_CR3_DMAR;
	
	// Set USART Interrupts
	// Transmission Complete Interrupt Enable:
//	USARTx->CR1 |= USART_CR1_TCIE;
	USARTx->CR1 |= USART_CR1_RXNEIE;
	
	if ((USART_TypeDef*)USARTx == USART1) {
		//Set interrupt priority to 4 in NVIC
		NVIC_SetPriority(USART1_IRQn, 4);
		//Enable interrupt in NVIC
		NVIC_EnableIRQ(USART1_IRQn);
	}
	else if ((USART_TypeDef*)USARTx == USART2) {
		//Set interrupt priority to 3 in NVIC
		NVIC_SetPriority(USART2_IRQn, 3);
		//Enable interrupt in NVIC
		NVIC_EnableIRQ(USART2_IRQn);
	}
	
	// TODO: Enable USART 1 and 2 like this?
	USARTx->CR1 |= USART_CR1_UE;
}


/**
 * This function accepts a string that should be sent through UART
*/
void UART_print(char* data) {
	// add 1 char to array until NULL reached
	// int8_t data_size = 0; // variable to count how big input array is
	
	// if the data_t0 array is the one that is currently pending, append to it
	if (pending == data_t_0) {
		while (data[pending_size]) { // check if current char exists
			data_t_0[pending_size] = data[pending_size];
			pending_size++;
		}
	}
	// if the data_t1 array is the one that is currently pending, append to it
	else if (pending == data_t_1) {
		while (data[pending_size]) { // check if current char exists
			data_t_1[pending_size] = data[pending_size];
			pending_size++;
		}
	}
	//Transfer char array to buffer
	// do this in if statement in our implementation
	//Check DMA status. If DMA is ready, send data
	// check if DMA ready
	if (!(tx->CCR & DMA_CCR_EN)) {
		//  switch buffers
		temp = pending;
		pending = active;
		active = temp;
		// 	send data
		tx->CMAR = (uint32_t)active; 
		tx->CNDTR = pending_size;
		tx->CCR |= DMA_CCR_EN;
		// 	set pending_size to 0 so interrupt handler doesn't run print again
		pending_size = 0;
		return;
	}
	
	//If DMA is not ready, put the data aside
	// idk pending_size does this anyway just kinda chill
}

void UART_onInput(char* inputsFunction, uint32_t size) {
	char* lessStr1 = "L";
	char* lessStr2 = "less";
	char* moreStr1 = "M";
	char* moreStr2 = "more";
	char* pauseStr1 = "P";
	char* pauseStr2 = "p";
	char* dataPrintLess = "Dispensing less candy\n";
	char* dataPrintMore = "Dispensing more candy\n";
	char* dataPrintPause = "Pausing Motor\n";
	char* invalidInput = "Invalid Input\n";
	int marker = 0;
	// Compare first character separately so initial marker can be set
	char buffer[IO_SIZE];
	delay(50);
//	sprintf(buffer, "Size: %d\n", size);
//	UART_print(buffer);
	if ((inputsFunction[0] == lessStr1[0] || inputsFunction[0] == lessStr2[0])) {
		// input is the left-rotating input 
		marker = 1;
	}
	else if ((inputsFunction[0] == moreStr1[0] || inputsFunction[0] == moreStr2[0])) {
		// input is the right-rotating input 
		marker = 2;
	}
	else if ((inputsFunction[0] == pauseStr1[0] || inputsFunction[0] == pauseStr2[0])) {
		// input is the right-rotating input 
		marker = 3;
	}
	else {
		marker = 0;
	}
	// Compare rest of string to see if it matches an input
	// Size - 2 because we don't want to take into account the extra \n in the buffer
	for (uint32_t i = 1; i < size - 2; i++) {
//		delay(100);
//		sprintf(buffer, "Character %d: %c\n", i, inputsFunction[i]);
//		UART_print(buffer);
		if ((inputsFunction[i] == lessStr1[i] || inputsFunction[i] == lessStr2[i]) && marker != 2 && marker != 0 && marker != 3) {
			// input is the left-rotating input 
			marker = 1;
		}
		else if ((inputsFunction[i] == moreStr1[i] || inputsFunction[i] == moreStr2[i]) && marker != 1 && marker != 0 && marker != 3) {
			// input is the right-rotating input 
			marker = 2;
		}
		else if ((inputsFunction[i] == pauseStr1[i] || inputsFunction[i] == pauseStr2[i]) && marker != 1 && marker != 0 && marker != 2) {
			// input is the right-rotating input 
			marker = 3;
		}
		else {
			marker = 0;
			break;
		}
	}
	if (marker == 1) {
		// TODO: start "less candy" routine
		// send message to UART to print to Termite
//		delay(50);
		//UART_print(dataPrintLess);
		//setDire(1); // comment this out later
		consoleSignal(1);
		pauseSignal(0);
	}
	else if (marker == 2) {
		// TODO: rotate motor right
		// send message to UART to print to Termite
		//delay(50);
		//UART_print(dataPrintMore);
		//setDire(2); // comment this out later
		consoleSignal(2);
	}
	else if (marker == 3) {
		// TODO: pause motor (for testing)
		// send message to UART to print to Termite
		//delay(50);
		UART_print(dataPrintPause);
//		setDire(0); // comment this out later
		pauseSignal(1);
	}
	else {
//		delay(50);
		UART_print(invalidInput);
//		setDire(0); // comment this out later
		consoleSignal(3);
	}
}	

/**
 * This function should be invoked when a character is accepted through UART
*/
void transfer_data(char ch) {
	
	// Append character to input buffer.
	inputs[input_size] = ch;
	input_size++;
//	char buffer[10];
//	delay(50);
//	sprintf(buffer, "Character: %c\n", ch);
//	UART_print(buffer);
	// If the character is end-of-line, invoke UART_onInput
	if (ch == 0x0A) {
		UART_onInput(inputs, input_size);
		// reset input array for future use
		for (int i = 0; i < input_size; i++) {
			inputs[i] = 0;
		}
		// reset input_size for future use
		input_size = 0;
	}
}

/**
 * This function should be invoked when DMA transaction is completed
*/
void on_complete_transfer(void) {
	// If there are pending data to send, switch active and pending buffer, and send data
	if (pending_size != 0) {
		temp = pending;
		pending = active;
		active = temp;
		// 	send data
		tx->CMAR = (uint32_t)active; 
		tx->CNDTR = pending_size;
		tx->CCR |= DMA_CCR_EN;
		// 	set pending_size to 0 so interrupt handler doesn't run print again
 		pending_size = 0;
	}
	tx->CCR &= ~DMA_CCR_EN;
}

void USART1_IRQHandler(void){
	// "Bluetooth handler"
	// send char that was received from phone to transfer_data 
	// or complete transfer if pending data exists
	NVIC_ClearPendingIRQ(USART1_IRQn);
	if (USART1->ISR & USART_ISR_TC) {
		// 	reset flag
		USART1->ICR |= USART_ICR_TCCF;
		// 	disable DMA
		//DMA1_Channel7->CCR &= ~DMA_CCR_EN;
		// 	call on_complete_transfer
		//on_complete_transfer();
	}
	else if (USART1->ISR & USART_ISR_RXNE) {
		// 	reset flag
		//USART2->RQR |= USART_RQR_RXFRQ;
		completeTermite();
		//	call transfer_data
		transfer_data((char)(USART1->RDR & 0xFF));
	}
}

void USART2_IRQHandler(void){
	// "Termite handler"
	// send char that was received from phone to transfer_data 
	// or complete transfer if pending data exists
	// if flag triggered (USART2 TC)
	NVIC_ClearPendingIRQ(USART2_IRQn);
	
	if (USART2->ISR & USART_ISR_TC) {
		// 	reset flag
		USART2->ICR |= USART_ICR_TCCF;
		// 	disable DMA
		//DMA1_Channel7->CCR &= ~DMA_CCR_EN;
		// 	call on_complete_transfer
		//on_complete_transfer();
	}
	else if (USART2->ISR & USART_ISR_RXNE) {
		// 	reset flag
		//USART2->RQR |= USART_RQR_RXFRQ;
		completeTermite();
		//	call transfer_data
		transfer_data((char)(USART2->RDR & 0xFF));
	}
}
