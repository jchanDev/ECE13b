#include "SPI.h"
#include "SysTimer.h"

void SPI1_GPIO_Init(void) {
	// Enable the GPIO Clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// Set PA4, PB3, PB4, and PB5 to Alternative Functions, and configure their AFR to SPI1
	// PA4 - CS
	GPIOA->MODER |= GPIO_MODER_MODE4_1;
	GPIOA->MODER &= ~GPIO_MODER_MODE4_0;
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL4_3;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL4_2;
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL4_1;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL4_0;
	
	// PB3 - SCL
	GPIOB->MODER |= GPIO_MODER_MODE3_1;
	GPIOB->MODER &= ~GPIO_MODER_MODE3_0;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL3_3;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL3_2;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL3_1;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL3_0;
	
	// PB4 - SDO
	GPIOB->MODER |= GPIO_MODER_MODE4_1;
	GPIOB->MODER &= ~GPIO_MODER_MODE4_0;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL4_3;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL4_2;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL4_1;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL4_0;
	
	// PB5 - SDA
	GPIOB->MODER |= GPIO_MODER_MODE5_1;
	GPIOB->MODER &= ~GPIO_MODER_MODE5_0;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL5_3;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL5_2;
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL5_1;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL5_0;
	// Set GPIO Pins to: Very High Output speed, Output Type Push-Pull, and No Pull-Up/Down
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED4;
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED3;
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED4;
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED5;
	
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT4;
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT3;
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT4;
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT5;
	
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD3;
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD4;
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD5;
}


void SPI1_Init(void){
	// Enable SPI clock and Reset SPI
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
	// Disable SPI
	SPI1->CR1 &= ~SPI_CR1_SPE;
	// Configure for Full Duplex Communication
	SPI1->CR1 &= ~SPI_CR1_RXONLY;
	// Configure for 2-line Unidirectional Data Mode
	SPI1->CR1 &= ~SPI_CR1_BIDIMODE;
	// Disable Output in Bidirectional Mode
	SPI1->CR1 &= ~SPI_CR1_BIDIOE;
	// Set Frame Format: MSB First, 16-bit, Motorola Mode
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;
	SPI1->CR2 |= SPI_CR2_DS;
	SPI1->CR2 &= ~SPI_CR2_FRF;
	// Configure Clock. Read DataSheet for required value
	// CPOL = 1, CPHA = 1
	// Max fclk = 5MHz
	SPI1->CR1 |= SPI_CR1_CPOL;
	SPI1->CR1 |= SPI_CR1_CPHA;
	// Set Baud Rate Prescaler to 16
	SPI1->CR1 &= ~SPI_CR1_BR_2;
	SPI1->CR1 |= SPI_CR1_BR_1;
	SPI1->CR1 |= SPI_CR1_BR_0;
	// Disable Hardware CRC Calculation
	SPI1->CR1 &= ~SPI_CR1_CRCEN;
	// Set as Master
	SPI1->CR1 |= SPI_CR1_MSTR;
	// Disable Software Slave Management
	SPI1->CR1 &= ~SPI_CR1_SSM;
	// Enable NSS Pulse Management
	SPI1->CR2 |= SPI_CR2_NSSP;
	// Enable Output
	SPI1->CR2 |= SPI_CR2_SSOE;
	// Set FIFO Reception Threshold to 1/2
	SPI1->CR2 &= ~SPI_CR2_FRXTH;
	// Enable SPI
	SPI1->CR1 |= SPI_CR1_SPE;
}

uint16_t SPI_Transfer_Data(uint16_t write_data){
	// TODO: Do we have to set / unset flags manually?
	// Wait for TXE (Transmit buffer empty)
	// Wait for flag to be SET
	while (!(SPI1->SR & SPI_SR_TXE));
	// Write data
	SPI1->DR = write_data;
	// Wait for not busy
	// Wait for flag to be UNSET
	while (SPI1->SR & SPI_SR_BSY);
	// Wait for RXNE (Receive buffer not empty)
	// Wait for flag to be SET
	while (!(SPI1->SR & SPI_SR_RXNE));
	// Read data
	uint16_t receive_data = (uint16_t) SPI1->DR;
	return receive_data;
}