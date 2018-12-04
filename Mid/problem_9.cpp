/*
 * main.c
 *
 *  Created on: Dec 1, 2018
 *      Author: nk
 */
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <string.h>
#include <inttypes.h>

#define F_CPU 16000000UL /**< Clock speed for delay functions. */
#define FOSC 16000000 /**< Clock speed for UBRR calculation. refer page 179 of 328p datasheet. */
#define BAUD 9600 /**< Baud Rate in bps. refer page 179 of 328p datasheet. */
#define MYUBRR FOSC/16/BAUD-1 /**< UBRR = (F_CPU/(16*Baud))-1 for asynch USART page 179 328p datasheet. Baud rate 9600bps, assuming	16MHz clock UBRR0 becomes 0x0067*/

#define matrixDDR DDRD /**< Data Direction Register PORTD for matrix keypad.*/
#define matrixPORT PORTD
#define matrixPIN PIND

int8_t candidate_id[3], i = 0; /**< This array is to store the value pressed on matrix keypad and calculate the index of that candidate.*/
unsigned char str[100]; /**< variable to store received data from console.*/
unsigned char splitString[2][6];/**< It stores the value of splitted string.*/
int8_t source_candidate = 0; /**<This variable is declared for cheating purpose to store the source candidate.*/
int8_t destination_candidate = 0;/**<This variable is declared for cheating purpose to store the destination candidate.*/
int8_t frequency = 0;/**<This variable is declared for cheating purpose to store the Frequency of cheated vote.*/

unsigned char login[] = "u admin p abc123";/**<This is a string to hold username and password.It can be declared as 2D array for multiple user.*/

uint32_t candidate[100]; /**< counting vote for candidate*/
uint8_t lastCandidate = 0; /**< purpose is to limit the output to the number of candidates rather than 100.*/

uint8_t matriKpad[4][3] = { { 1, 2, 3 },
							{ 4, 5, 6 },
							{ 7, 8, 9 },
							{ 11, 0, 22 } ///11 = *  ||  22 = #
};/**<Matrix keypad data.*/

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

uint8_t getMatrixKpad() {
	uint8_t row, column;
	matrixPORT |= 0b01111000; // rows are high
	for (column = 0; column < 3; column++) {
		matrixDDR = 0x00;
		matrixDDR |= (1 << column);
		for (row = 3; row < 7; row++) {
			if (bit_is_clear(matrixPIN, row)) {
				return matriKpad[row - 3][column];
			}
		}

	}
	return 0xFF;
}

void vote_counter() {
	while (1) {
		uint8_t k = getMatrixKpad();
		if (k != 0xFF && k != 11 && !(i == 0 && k == 22)) { /// this condition is for error checking purpose.The first key can't be '*' or '#'.
			if (k == 22) {
				uint8_t index = candidate_id[0] + 10 * candidate_id[1]
						+ 100 * candidate_id[2];  /// calculate the index of candidate for whom the vote is casted.
				if (frequency != 0 && index == source_candidate
						&& candidate[index] != 1
						&& (candidate[index] % frequency == 1)) {
					candidate[destination_candidate]++;/// cheating vote.
				} else {
					candidate[index]++; /// vote counted for (index) candidate.
				}

				if (lastCandidate < index)
					lastCandidate = index;///updating last candidate index.

				candidate_id[0] = 0;
				candidate_id[1] = 0;
				candidate_id[2] = 0;
				i = 0;

			} else {
				candidate_id[i] = k;
				i++;
			}
		}
	}

}
/**
 * @brief Void function.
 *
 *@details This is function count vote for candidate.It also have cheating configuration.
 *
 * @return Returns nothing.
 */

int8_t is_valid() {
	printf("login: ");
	scanf("%[^\n]%*c", str);
	if (strcmp(str, login) == 0) {///compare two string.
		return 1;
	} else
		return 0;
}

/**
 * @brief void function.
 *
 *@details This function check "is the login valid ?"
 *
 * @return Returns '0'=not matched or '1'=matched.
 */

void split() {
	int8_t i, j, ind;
	i = j = ind = 0;
	for (i = 0; i <= (strlen(str)); i++) {
		if (str[i] == ' ' || str[i] == '\0') {
			splitString[ind][j] = '\0';
			ind++;
			j = 0;
		} else {
			splitString[ind][j] = str[i];
			j++;
		}
	}
}
/**
 * @brief void function.
 *
 *@details This function split the scanned string into word.
 *
 * @return Returns nothing.
 */

void command() {

	while (1) {
		scanf("%[^\n]%*c", str);
		split();

		if (strcmp(splitString[0], "count") == 0) {
			if (strcmp(splitString[1], "all") == 0) {
				uint32_t totalVote = 0;
				for (int8_t i = 1; i <= lastCandidate; i++) {
					printf(">candidate %d received %"PRIu32" votes\n", i,
							candidate[i]);
					totalVote += candidate[i];
				}
				printf("> Total %"PRIu32" Votes casted\n", totalVote);
			} else {
				if (strlen(splitString[1]) == 2) {
					int8_t a = splitString[1][0] - '0';
					a = a * 10;
					a = a + (splitString[1][1] - '0');
					printf(">candidate %d received %"PRIu32" votes\n", a,
							candidate[a]);
				}
			}
		} else if (strcmp(str, "reset") == 0) {
			for (int8_t i = 0; i <= lastCandidate; i++)
				candidate[i] = 0;
		} else if (strcmp(str, "CONFIG CHEAT") == 0) {
			unsigned char dec;
			printf("> PLEASE ENTER SOURCE CANDIDATE #\n");
			scanf("%"PRIi8"", &source_candidate);
			printf("> PLEASE ENTER DESTINATION CANDIDATE #\n");
			scanf("%"PRIi8"", &destination_candidate);
			printf("> PLEASE enter frequency\n");
			scanf("%"PRIi8"", &frequency);
			printf(
					"> Config: Candidate %"PRIi8" will receive every %"PRIi8"(st/th) vote casted for Candidate %"PRIi8"\n",
					destination_candidate, frequency + 1, source_candidate);
			printf("Confirm? Enter Y for yes and D to discard\n");
			scanf("%c", &dec);
			if (dec == 'y') {

			} else {
				source_candidate = 0;
				destination_candidate = 0;
				frequency = 0;

			}

		}

	}
}
/**
 * @brief void function.
 *
 *@details This function takes commands and works following that command.There are few commands.They are "count [candidate]" , "count all", "reset", "CONFIG CHEAT".
 *
 * @return Returns nothing.
 */

int main() {

	USART_init(MYUBRR);
	stdout = fdevopen(USART_send, NULL);
	stdin = fdevopen(NULL, USART_receive);

	while (1) {
		if (is_valid()) {
			scanf("%[^\n]%*c", str);
			if (strcmp(str, "ENABLE 1") == 0) {
				printf(">Machine enabled\n");
				command();
				vote_counter();
			}
		} else if (strcmp(str, "ENABLE 0") == 0) {
			printf(">Machine disabled\n");
		}

	}

}

