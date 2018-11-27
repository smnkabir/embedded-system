/*
 * main.c
 *
 *  Created on: Nov 27, 2018
 *      Author: nk
 */

#include<avr/io.h>
#include<avr/delay.h>
uint8_t stage = 0; // 0 = off ,1 = on

void power_on() {
	PORTD = 0x00;
	PORTD |= (1 << PB1); // SET PORTD1 AS THE SYMBOLE OF POWER ON
}
void power_off() {
	PORTD = 0x00;
	PORTD |= (1 << PB0); // SET PORTD0 AS THE SYMBOLE OF POWER OFF
}
void pearing() {
	PORTD = 0x00;
	PORTD |= (1 << PB2); // SET PORTD2 AS THE SYMBOLE OF PEARING
}

int8_t is_4s() {
	if (bit_is_set(PINB, PB2)) {
		_delay_ms(400);
		if (bit_is_set(PINB, PB2)) {
			_delay_ms(400);
			if (bit_is_set(PINB, PB2)) {
				_delay_ms(400);
				if (bit_is_set(PINB, PB2)) {
					_delay_ms(400);
					if (bit_is_set(PINB, PB2)) {
						_delay_ms(400);
						if (bit_is_set(PINB, PB2)) {
							_delay_ms(400);
							if (bit_is_set(PINB, PB2)) {
								_delay_ms(400);
								if (bit_is_set(PINB, PB2)) {
									_delay_ms(400);
									if (bit_is_set(PINB, PB2)) {
										_delay_ms(400);
										if (bit_is_set(PINB, PB2)) {
											_delay_ms(400);
											if (bit_is_set(PINB, PB2)) {
												return 1;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

int main() {
	DDRB &= ~(1 << PB2); // SET PORTB2 AS INPUT
	DDRD |= (1 << PB0) | (1 << PB1) | (1 << PB2); // SET PORTD 0,1,2 AS OUTPUT

	while (1) {
		switch (stage) {
		case 0:
			if (is_4s()) {
				power_on();
				stage = 1;
			}
			break;
		case 1:
			while (1) {
				if (is_4s()) {
					power_off();
					stage = 0;
					break;
				}
				if (bit_is_set(PINB, PB2)) {
					_delay_ms(1000);
					if (bit_is_set(PINB, PB2)) {
						pearing();
					}
				}
			}
			break;
		default:
			break;
		}
	}
}

