/*
 * main.c
 *
 *  Created on: Dec 20, 2018
 *      Author: nk
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <spi.h>

#define F_CPU 16000000UL

#define FOSC 16000000 /**< Clock speed for UBRR calculation. refer page 179 of 328p datasheet. */
#define BAUD 9600 /**< Baud Rate in bps. refer page 179 of 328p datasheet. */
#define MYUBRR FOSC/16/BAUD-1 /**< UBRR = (F_CPU/(16*Baud))-1 for asynch USART page 179 328p datasheet. Baud rate 9600bps, assuming	16MHz clock UBRR0 becomes 0x0067*/
#define SPI_CS PORTB4 /**< SPI chip select*/

int result;
/**
 * @brief Initialize USART for 8 bit data transmit no parity and 1 stop bit.
 *
 *@details This is a code snippet from datasheet page 182
 *
 * @param ubrr The UBRR value calculated in macro MYUBRR
 * @see MYUBRR
 */
void USART_init(unsigned int ubrr) {

	UCSR0C = (0 << USBS0) | (3 << UCSZ00); /// Step 1. Set UCSR0C in Asynchronous mode, no parity, 1 stop bit, 8 data bits
	UCSR0A = 0b00000000; /// Step 2. Set UCSR0A in Normal speed, disable multi-proc

	UBRR0H = (unsigned char) (ubrr >> 8); /// Step 3. Load ubrr into UBRR0H and UBRR0L
	UBRR0L = (unsigned char) ubrr;

	UCSR0B = 0b00011000; /// Step 4. Enable Tx Rx and disable interrupt in UCSR0B
}

/**
 * @brief Send 8bit data.
 *
 *@details This is a code snippet from datasheet page 184
 *
 * @param data The 8 bit data to be sent
 */

int USART_send(char c, FILE *stream) {

	while (!(UCSR0A & (1 << UDRE0))) /// Step 1.  Wait until UDRE0 flag is high. Busy Waitinig
	{
		;
	}

	UDR0 = c; /// Step 2. Write char to UDR0 for transmission
}

/**
 * @brief Receive 8bit sata.
 *
 *@details This is a code snippet from datasheet page 187
 *
 * @return Returns received data from UDR0
 */
int USART_receive(FILE *stream) {

	while (!(UCSR0A & (1 << RXC0)))
		/// Step 1. Wait for Receive Complete Flag is high. Busy waiting
		;

	return UDR0; /// Step 2. Get and return received data from buffer
}

void SPI_init() {

}

int main() {
	DDRB = 0x00; /// setting PORTB as output

	USART_init(MYUBRR);
	SPI_CS = 1;
	SPI_init();
	stdout = fdevopen(USART_send, NULL);
	stdin = fdevopen(NULL, USART_receive);

	while (1) {
		int8_t value, command;
		SPI_CS = 0;
		printf("Enter a command for pot: ")
		scanf("%d", &command);
		switch (command) {
		case 0:  /// No command will be executed
			break;
		case 1: /// Write Data
			printf("Enter a value for pot: ")
			scanf("%d", &value);
			WriteSPI(command); /// writing the command to the pot
			WriteSPI(value); /// writing the value to the pot
			SPI_CS = 1;
			break;
		case 2: /// shutdown
			SPI_CS = 0;
			break;
		case 3: /// none
			break;
		default:
			break;

		}

	}

}

