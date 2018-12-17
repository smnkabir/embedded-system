/*
 * main.c
 *
 *  Created on: Dec 15, 2018
 *      Author: nk
 */

#include<avr/io.h>
#include<util/delay.h>
#define F_CPU 16000000UL

int i = 5;
void b_wave() {
	OCR1B = 0;
	for (i = 0; i < 255; i++) {
		OCR1B = 200 + i;
		_delay_ms(10);
	}
	for (i = 0; i < 255; i++) {
		OCR1B = 767 + i;
		_delay_ms(10);
	}

	for (i = 0; i < 255; i++) {
		OCR1B = 1023 - i;
		_delay_ms(10);
	}
	for (i = 0; i < 255; i++) {
		OCR1B = 453 - i;
		_delay_ms(10);
	}
}

void d_wave() {
	OCR1A = 200;
	for (i = 0; i < 100; i++) {
		OCR1A -= 1;
		_delay_ms(10);
	}
	for (i = 0; i < 100; i++) {
		OCR1A += 0;
		_delay_ms(10);
	}
	OCR1A = 200;
	for (i = 0; i < 100; i++) {
		OCR1A += 1;
		_delay_ms(10);
	}
	OCR1A = 400;
	for (i = 0; i < 200; i++) {
		OCR1A += 1;
		_delay_ms(10);
	}
	OCR1A = 700;
	for (i = 0; i < 100; i++) {
		OCR1A += 1;
		_delay_ms(10);
	}
	OCR1A = 900;
	for (i = 0; i < 123; i++) {
		OCR1A += 1;
		_delay_ms(10);
	}
	for (i = 0; i < 123; i++) {
		OCR1A -= i;
		_delay_ms(10);
	}
	OCR1A = 800;
	for (i = 0; i < 100; i++) {
		OCR1A -= 1;
		_delay_ms(10);
	}
	for (i = 0; i < 100; i++) {
		OCR1A += 1;
		_delay_ms(10);
	}
	OCR1A = 900;
	for (i = 0; i < 123; i++) {
		OCR1A += 1;
		_delay_ms(10);
	}
	for (i = 0; i < 124; i++) {
		OCR1A -= i;
		_delay_ms(10);
	}
	OCR1A = 800;
	for (i = 0; i < 100; i++) {
		OCR1A -= 1;
		_delay_ms(10);
	}
	OCR1A = 600;
	for (i = 0; i < 200; i++) {
		OCR1A -= 1;
		_delay_ms(10);
	}
	OCR1A = 300;
	for (i = 0; i < 100; i++) {
		OCR1A -= 1;
		_delay_ms(10);
	}
	OCR1A = 100;
	for (i = 0; i < 100; i++) {
		OCR1A += 0;
		_delay_ms(10);
	}
	for (i = 0; i < 100; i++) {
		OCR1A += 1;
		_delay_ms(10);
	}

}
int main(void) {
	DDRB |= (1 << PB2) | (1 << PB1); ///OCR1A and OCR1B as output

	///Setup 10bit Fast PWM Mode 7
	/// WGM13=0, WGM12=1, WGM11=1, WGM10= 1 is for fastPWM in 10bit mode.
	///Here TOP= 0x03FF or 1023
	/// For more information 2313 datasheet table 46 pg 106
	OCR1B = 0;
	TCCR1B |= (0 << WGM13) | (1 << WGM12);
	TCCR1A |= (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0)
			| (1 << WGM11) | (1 << WGM10);

	///COM1A1:COM1A0 =10 COM1B1:COM1B0 =10 and for Clear OC1A/OC1B on Compare Match, set OC1A/OC1B at TOP
	///For more information 2313 datasheet table 44 pg 104
	/// PWM_Frequency= F_osc / (PreScaler*(1+TOP))= 20MHz / (1*(1023+1))=  19531.25 = 19.5 KHz
	/// Resolution= log(TOP+1)/log(2)= 10 bit

	TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10); //CS12->10 = 001 is for 1:1 timer pre-scaler. Start timer

	while (1) {
		b_wave();
		d_wave();
	}
}

