/*
 * ECE 153B
 *
 * Name(s): Ishaan Joshi, Julia Chan
 * Section: Tues 7PM
 * Project
 */
 
#include "DMA.h"
#include "UART.h"
#include "SysTimer.h"

void DMA_Init_UARTx(DMA_Channel_TypeDef * tx, USART_TypeDef * uart) {
	//TODO
	
	// TODO: Is this the right channel?
	//Enable the clock for DMA
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	//Wait 20us for DMA to finish setting up
	delay(20);
	//disable the channel
	tx->CCR &= ~DMA_CCR_EN;
	//Disable Memory-to-memory mode
	tx->CCR &= ~DMA_CCR_MEM2MEM;
	//Set channel priority to high
	tx->CCR |= DMA_CCR_PL_1;
	tx->CCR &= ~DMA_CCR_PL_0;
	//Set peripheral size to 8-bit
	tx->CCR &= ~DMA_CCR_PSIZE_1;
	tx->CCR &= ~DMA_CCR_PSIZE_0;
	//Set memory size to 8-bit
	tx->CCR &= ~DMA_CCR_MSIZE_1;
	tx->CCR &= ~DMA_CCR_MSIZE_0;
	//Disable peripheral increment mode
	tx->CCR &= ~DMA_CCR_PINC;
	//Enable memory increment mode
	tx->CCR |= DMA_CCR_MINC;
	//Disable circular mode
	tx->CCR &= ~DMA_CCR_CIRC;
	//Set data transfer direction to Memory-to-Peripheral
	tx->CCR |= DMA_CCR_DIR;
	//Set the data source to data buffer provided in CRC.h
//	tx->CMAR = (uint32_t)active; // set the CMAR later
	//Set the data destination to the data register of the UART block
	tx->CPAR = (uint32_t)&(uart->TDR); 
	//Disable half transfer interrupt
	tx->CCR &= ~DMA_CCR_HTIE;
	//Disable transfer error interrupt
	tx->CCR &= ~DMA_CCR_TEIE;
	//Enable transfer complete interrupt
	tx->CCR |= DMA_CCR_TCIE;
	// Configure DMA1 Channel Select to UART1 TX
	// CHANGE BASED ON WHAT DMA / UART IS BEING USED
	uint32_t channelSelectMask;
	
	if ((DMA_Channel_TypeDef*)tx == DMA1_Channel4) {
		//Set interrupt priority to 0 in NVIC
		NVIC_SetPriority(DMA1_Channel4_IRQn, 1);
		//Enable interrupt in NVIC
		NVIC_EnableIRQ(DMA1_Channel4_IRQn);
		channelSelectMask = 0x00002000U;
	}
	else if ((DMA_Channel_TypeDef*)tx == DMA1_Channel7) {
		//Set interrupt priority to 0 in NVIC
		NVIC_SetPriority(DMA1_Channel7_IRQn, 0);
		//Enable interrupt in NVIC
		NVIC_EnableIRQ(DMA1_Channel7_IRQn);
		channelSelectMask = 0x02000000U;
	}
	DMA1_CSELR->CSELR = channelSelectMask;
}


void DMA1_Channel7_IRQHandler(void) {
	// Channel 7: USART2 TX (Termite)
	//Clear NVIC interrupt flag
	NVIC_ClearPendingIRQ(DMA1_Channel7_IRQn);
	//Check Transfer Complete interrupt flag. If it occurs, clear the flag and mark computation as completed by calling computationComplete.
	if(DMA1->ISR & DMA_ISR_TCIF7){
		DMA1->IFCR |= DMA_IFCR_CTCIF7;
		completeTermite();
		// TODO: Clear active buffer?
		DMA1_Channel7->CCR &= ~DMA_CCR_EN;
		on_complete_transfer();
	}
	if(DMA1->ISR & DMA_ISR_GIF7){
		//Clear global DMA interrupt flag.
		DMA1->IFCR |= DMA_IFCR_CGIF7;
	}
}
void DMA1_Channel4_IRQHandler(void) {
	// Channel 4: USART1 TX (Bluetooth)
	// TODO: Check if flags are correct
	//Clear NVIC interrupt flag
	NVIC_ClearPendingIRQ(DMA1_Channel4_IRQn);
	//Check Transfer Complete interrupt flag. If it occurs, clear the flag and mark computation as completed by calling computationComplete.
	if(DMA1->ISR & DMA_ISR_TCIF4){
		DMA1->IFCR |= DMA_IFCR_CTCIF4;
		completeBluetooth();
		// TODO: Clear active buffer?
		DMA1_Channel4->CCR &= ~DMA_CCR_EN;
		on_complete_transfer();
	}
	if(DMA1->ISR & DMA_ISR_GIF4){
		//Clear global DMA interrupt flag.
		DMA1->IFCR |= DMA_IFCR_CGIF4;
	}
}