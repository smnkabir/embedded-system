/*
 * main.c
 *
 *  Created on: Nov 27, 2018
 *      Author: nk
 */
#include <avr/io.h>
#include<util/delay.h>
#include<stdbool.h>
#include<stdio.h>

int8_t stage = 1; // 0 means calling stage ,1 means music stage;

void volume_up() {
	printf("volume_up");
	PORTD |= (1 << PD1);
	_delay_ms(2000);
	PORTD &= ~(1 << PD1);
}
void volume_down() {
	printf("volume_down");
	PORTD |= (1 << PD2);
	_delay_ms(2000);
	PORTD &= ~(1 << PD2);
}
void answer_call() {
	printf("answer_call");
	PORTD |= (1 << PD0);
	_delay_ms(2000);
	PORTD &= ~(1 << PD0);
}
void reject_call() {
	printf("reject_call");
	PORTD |= (1 << PD0);
	_delay_ms(2000);
	PORTD &= ~(1 << PD0);
}
void next_song() {
	printf("next_song");
	PORTD |= (1 << PD4);
	_delay_ms(2000);
	PORTD &= ~(1 << PD4);
}
void previous_song() {
	printf("previos_song");
	PORTD |= (1 << PD3);
	_delay_ms(2000);
	PORTD &= ~(1 << PD3);
}
void play_song() {
	printf("play_song");
	PORTD |= (1 << PD0);
	_delay_ms(2000);
	PORTD &= ~(1 << PD0);
}
void pause_song() {
	printf("pause_song");
	PORTD |= (1 << PD0);
	_delay_ms(2000);
	PORTD &= ~(1 << PD0);

}

void call() {
	while (1) {
		if (stage)
			break;
		if (bit_is_set(PINB, PIN0)) {
			while (bit_is_set(PINB, PIN0))
				;
			_delay_ms(500);
			if (bit_is_set(PINB, PIN0)) {
				while (bit_is_set(PINB, PIN0))
					;
				reject_call();
			}

			else
				answer_call();
		}
	}
}
void music() {
	bool playing = true;
	while (1) {
		if (!stage)
			break;
		if (bit_is_set(PINB, PB0)) {
			while (bit_is_set(PINB, PB0))
				;
			if (playing) {
				pause_song();
				playing = false;
			} else {
				play_song();
			}
		}

		if (bit_is_set(PINB, PB1))
			volume_up();

		if (bit_is_set(PINB, PB2))
			volume_down();

		if (bit_is_set(PINB, PB3)) {
			while (bit_is_set(PINB, PIN3))
				;
			previous_song();
		}

		if (bit_is_set(PINB, PIN4)) {
			while (bit_is_set(PINB, PB4))
				;
			next_song();
		}

	}
}

int main() {
	DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4)); // SET PB 0-4 AS INPUT
	DDRD = ((1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4));
	while (1) {
		switch (stage) {
		case 0:
			call();
			break;
		case 1:
			music();
			break;
		default:
			break;
		}
	}
	return 0;
}

