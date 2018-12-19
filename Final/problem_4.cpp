/*
 * main.c
 *
 *  Created on: Dec 17, 2018
 *      Author: nk
 */
#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stdio.h>

#define F_CPU 16000000UL

#define FOSC 16000000 /**< Clock speed for UBRR calculation. refer page 179 of 328p datasheet. */
#define BAUD 9600 /**< Baud Rate in bps. refer page 179 of 328p datasheet. */
#define MYUBRR FOSC/16/BAUD-1 /**< UBRR = (F_CPU/(16*Baud))-1 for asynch USART page 179 328p datasheet. Baud rate 9600bps, assuming	16MHz clock UBRR0 becomes 0x0067*/
uint16_t result;
/**
 * @brief Initialize USART for 8 bit data transmit no parity and 1 stop bit.
 *
 *@details This is a code snippet from datasheet page 182
 *
 * @param ubrr The UBRR value calculated in macro MYUBRR
 * @see MYUBRR
*/
void USART_init(unsigned int ubrr)
{

	UCSR0C = (0<<USBS0)|(3<<UCSZ00); /// Step 1. Set UCSR0C in Asynchronous mode, no parity, 1 stop bit, 8 data bits
	UCSR0A = 0b00000000;/// Step 2. Set UCSR0A in Normal speed, disable multi-proc

	UBRR0H = (unsigned char)(ubrr>>8);/// Step 3. Load ubrr into UBRR0H and UBRR0L
	UBRR0L = (unsigned char)ubrr;


	UCSR0B = 0b00011000;/// Step 4. Enable Tx Rx and disable interrupt in UCSR0B
}

/**
 * @brief Send 8bit data.
 *
 *@details This is a code snippet from datasheet page 184
 *
 * @param data The 8 bit data to be sent
*/

int USART_send(char c, FILE *stream)
{

	while ( !( UCSR0A & (1<<UDRE0)) )/// Step 1.  Wait until UDRE0 flag is high. Busy Waitinig
	{;}

	UDR0 = c; /// Step 2. Write char to UDR0 for transmission
}

/**
 * @brief Receive 8bit sata.
 *
 *@details This is a code snippet from datasheet page 187
 *
 * @return Returns received data from UDR0
*/
int USART_receive( FILE *stream )
{

while ( !(UCSR0A & (1<<RXC0)) )/// Step 1. Wait for Receive Complete Flag is high. Busy waiting
;

return (UDR0);/// Step 2. Get and return received data from buffer
}

void init_PWM()
{
	TCCR1A|= (1<<COM1A1);
	TCCR1A &= ~(1<<COM1A0); ///COM1A1:COM1A0 1:0 Clear OC1A on Compare Match
	////WGM 13->10 = 1000 Mode 8 phase correct PWM top ICR1
	TCCR1A &= ~(1<<WGM10)|~(1<<WGM11);
	TCCR1B &=~(1<<WGM12);
	TCCR1B |=(1<<WGM13);
	ICR1 = 80000; /// TOP 100 Hz or, 1 pulse = 100ms

	DDRB |= (1<<PB1); ///make OC1A output

}



void init_ADC()
{
    /// ADMUX section 23.9.1 page 262
    ///BIT 7 and BIT 6 – AVCC with external capacitor at AREF pin REFS0 =0 and REFS1= 1
    /// BIT 5 – ADC Right Adjust Result ADLAR = 0
    /// BIT 3:0 –MUX3:0 0b0000 for channel 0
    ADMUX = 0b01000000;
    //same as previous line
    //  ADMUX = (_BV(REFS0))| (ADMUX & ~_BV(REFS1))|(ADMUX & ~_BV(ADLAR))|(ADMUX & ~_BV(MUX3))|(ADMUX & ~_BV(MUX2))|(ADMUX & ~_BV(MUX1))|(ADMUX & ~_BV(MUX0));

    /// ADSCRA ADC Control and Status Register A Section 23.9.2 page 263 -264
    ///Bit 7 – ADEN: ADC Enable =1 enable ADC
    ///Bit 6 – ADSC: ADC Start Conversion =0 do not start conversion
    ///Bit 5 – ADATE: ADC Auto Trigger Enable = 0 Disable trigger
    ///Bit 4 – ADIF: ADC Interrupt Flag = 0
    ///Bit 3 – ADIE: ADC Interrupt Enable = 0 Disable ADC interrupt
    ///Bits 2:0 – ADPS2:0: ADC Prescaler Select Bits 010 division factor = 4
    ADCSRA = 0b10000010;

}

int Map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t read_ADC(uint8_t ch)
{
	// select the corresponding channel 0~7
	  // ANDing with ’7′ will always keep the value
	  // of ‘ch’ between 0 and 7
	  ch &= 0b00000111;  // AND operation with 7
	  ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing

	  // start single convertion
	  // write ’1′ to ADSC
	  ADCSRA |= (1<<ADSC);

	  // wait for conversion to complete
	  // ADSC becomes ’0′ again
	  // till then, run loop continuously
	  while(ADCSRA & (1<<ADSC));

	  return (ADC);
}

int main ()
{
	init_PWM();
	init_ADC();
	int map_res;
	USART_init(MYUBRR);
	// Initialise the standard IO handlers
	stdout = fdevopen(USART_send, NULL);
	stdin = fdevopen(NULL, USART_receive);
	OCR1A = 20000;// OCR1A = x ms * 1000  2ms = 2*1000 =2000
	TCCR1B |= (1<<CS11); // CS12 CS11 CS10 = 010 for 1:8 pre-scaler and start timer
	while(1)
	{
		result = read_ADC(0);
		map_res = Map(result,0,1023, 704,2000);//MG996R
		OCR1A = map_res;
		printf("ADC value %" PRIu16 " and map value %d \r\n",result, map_res);
		_delay_ms(100);
	}
}




