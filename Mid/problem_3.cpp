/*
 * main.c
 *
 *  Created on: Nov 29, 2018
 *      Author: nk
 */
#include<avr/io.h>
#include<util/delay.h>

uint8_t LED[3][4] =
		{ { (1 << 0), (1 << 1), (1 << 2), (1 << 3) }
		, { (1 << 4), (1 << 5), (1 << 6), (1 << 7) }
		, { (1 << 0), (1 << 1), (1 << 2), (1 << 3) }
		}; /**< LED matrix */

int main() {
	DDRD |= 0XFF; /// SET PORTD AS OUTPUT
	PORTD = 0X00;
	DDRB |= 0X0F; /// SET PORTB 0-3 AS OUTPUT
	PORTB = 0X00;
	DDRC &= 0X00; /// PORTC AS INPUT 0 = CENTER, 1 = LEFT, 2 = RIGHT, 3 = UP, 4 = DOWN

	int8_t i = 0, j = 0;
	PORTB = LED[0][0]; /// INITIALIZE

	while (1) {
		if (bit_is_set(PORTC, PC1)) {    /// for left button
			while (bit_is_set(PORTC, PC1))
				;
			j = (j - 1 + 4) % 4;
			if (i < 2)
				PORTD = LED[i][j];
			else
				PORTB = LED[i][j];
		}

		if (bit_is_set(PORTC, PC2)) {    /// for right button
			while (bit_is_set(PORTC, PC2))
				;
			j = (j + 1) % 4;
			if (i < 2)
				PORTD = LED[i][j];
			else
				PORTB = LED[i][j];
		}

		if (bit_is_set(PORTC, PC3)) {    /// for up button
			while (bit_is_set(PORTC, PC3))
				;
			i = (i - 1 + 3) % 3;
			if (i < 2)
				PORTD = LED[i][j];
			else
				PORTB = LED[i][j];
		}

		if (bit_is_set(PORTC, PC4)) {    /// for down button
			while (bit_is_set(PORTC, PC4))
				;
			i = (i + 1) % 3;
			if (i < 2)
				PORTD = LED[i][j];
			else
				PORTB = LED[i][j];
		}
	}

}


