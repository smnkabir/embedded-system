#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <inttypes.h> // to print uint32_t

#define FOSC 16000000 /**< Clock speed for UBRR calculation. refer page 179 of 328p datasheet. */
#define BAUD 57600 /**< Baud Rate in bps. refer page 179 of 328p datasheet. */
#define MYUBRR FOSC/16/BAUD-1 /**< UBRR = (F_CPU/(16*Baud))-1 for asynch USART page 179 328p datasheet.*/

volatile uint32_t n; /**< n to count number of times TCNT1 overflowed */
volatile uint32_t elapse_time = 0;/**< Stores cumulative revolution completion time */
volatile uint32_t estimated_time = 0;/**<Stores time get from ADC (3 POT) */
volatile uint8_t flag = 0; /**<To control the blink */
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

	UCSR0B = 0b00001000; /// Step 4. Enable Tx Rx and disable interrupt in UCSR0B
}

/**
 * @brief Send 8bit data.
 *
 *@details This is a code snippet from datasheet page 184
 *
 * @param c The 8 bit data to be sent
 * @param FILE *stream to receive
 */

int USART_send(char c, FILE *stream) {

	while (!(UCSR0A & (1 << UDRE0))) /// Step 1.  Wait until UDRE0 flag is high. Busy Waitinig
	{
		;
	}

	UDR0 = c; /// Step 2. Write char to UDR0 for transmission
	return 0;
}

/**
 * @brief Receive 8bit data.
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

/**
 * @brief Interrupt service routine for Timer1 Overflow vector.
 *
 *@details It increases the value of n by one to number of time TIMER1 overflowed between two INT0 interrupts
 *
 */
ISR(TIMER1_OVF_vect) {

	n++;
}

void timer1_init() {

	TCCR1A = 0b00000000; ///Step 1.  normal mode
	TCCR1B = 0b00000011; ///Step 2. 1:64 prescaler, internal clock
	TIMSK1 = 0b00000001; ///Step 3. enable Timer 1 overflow interrupt
}

/**
 * @brief Interrupt service routine for INT0 vector.
 *
 *@details INT0 interrupt
 *
 */
ISR(INT0_vect) {
	if (flag == 0) { /// Exceuted only once for very first external interrupt and initites timer timer counter
		TCNT1 = 0;
		n = 0;
		flag = 1; /// Make the flag 1 so that it never enters again
	}

	elapse_time = n * 262144 + (uint32_t) (TCNT1 * 4); /// Caclulate the elapsed time  (Prescaler * 2^16)/(Fosc) = 64 * 2^16 / 16MHz = 262144 uS. TCNT1 counter updates after = Prescaler/Fosc = 64/16MHz = 4uS

	if (elapse_time == estimated_time) {
		flag = 0;
		elapse_time = 0;
		TCNT1 = 0; /// reset counter to calculate next delay
		n = 0;
	}
}

void init_ADC() {
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

uint16_t read_ADC(uint8_t ch) {
	// select the corresponding channel 0~7
	// ANDing with ’7′ will always keep the value
	// of ‘ch’ between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8) | ch; // clears the bottom 3 bits before ORing

	// start single convertion
	// write ’1′ to ADSC
	ADCSRA |= (1 << ADSC);

	// wait for conversion to complete
	// ADSC becomes ’0′ again
	// till then, run loop continuously
	while (ADCSRA & (1 << ADSC))
		;

	return (ADC);
}

int main() {
	DDRD |= (1 << PD2); /// Configure PORTD 2 as output
	PORTD |= (1 << PD2);
	USART_init(MYUBRR); /// Initialize USART
	stdout = fdevopen(USART_send, NULL); /// setup printf()

	timer1_init(); /// Initiate Timer1

	sei();
	/// Enable global interrupt
	while (1) {
		if (flag == 0) {

			PORTD ^= (1 << PD2);

			uint16_t first_pot = read_ADC(0); /// read the value for minutes
			uint16_t second_pot = read_ADC(0); /// read the value for seconds
			uint16_t third_pot = read_ADC(0); /// read the value for milliseconds
			estimated_time = ((uint32_t) first_pot * 60 * 1000000)
					+ ((uint32_t) second_pot * 1000000)
					+ ((uint32_t) third_pot * 1000); /// estimation of ADC pot value all together

		}

	}
}

