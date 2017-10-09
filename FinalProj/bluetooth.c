/*
 * bluetooth.c
 *
 * Created: 4/9/2016 10:12:07 PM
 *  Author: jmay
 */ 

#include <avr/interrupt.h>
#include <string.h>
#include "bluetooth.h"
#include "ui.h"

int in_buffer_len(void);

#define OUT_BUFFER_SIZE 100
static char out_buffer[OUT_BUFFER_SIZE];
static char* out_ptr = out_buffer;
static volatile int out_buffer_empty = 1;

#define IN_BUFFER_SIZE 10
static char in_buffer[IN_BUFFER_SIZE];
static char* in_ptr = in_buffer;
static volatile int in_buffer_ready = 0;

/// Puts a message in the UART sending buffer
/**
 * Queues a message for sending through UART.  If the buffer is full, it blocks until the buffer is empty.  It is limited at OUT_BUFFER_SIZE and will truncate any larger chunks.
 * @param msg the message string to send
 */
void send_msg(char* msg) {
	// Wait for a previous message to exit the buffer.
	while (!out_buffer_empty)
		;
	// Fill the output buffer
	out_buffer_empty = 0;
	strncpy(out_buffer, msg, OUT_BUFFER_SIZE - 1);
	out_ptr = out_buffer;
	// Wait for output register to be empty
	while ((UCSR0A & _BV(UDRE)) == 0)
		;
	// Send first character to start the interrupt handling of the rest of the string
	UDR0 = *(out_ptr++);
}

/// Reads a line or full buffer from UART
/**
 * Blocks until a line of data is ready.  A line is terminated by either a new line character, max_len, or a size of IN_BUFFER_SIZE, whichever is true first.
 * @param msg the store the location of the UART input
 * @param max_len the maximum number of characters to receive
 * @return the numer of character that have been read into msg
 */
int read_line(char* msg, int max_len) {
	while (!in_buffer_ready) 
		;
	int len = max_len < IN_BUFFER_SIZE - 1 ? max_len : IN_BUFFER_SIZE;
	strncpy(msg, in_buffer, len);
	// Restore a possible missing null character that strncpy did not place.
	msg[len - 1] = '\0';
	// Set up the variables for more input
	in_ptr = in_buffer;
	in_buffer_ready = 0;
	
	return len;
}

/// The size of the current data in the buffer
/**
 * The size of the data currently in the input buffer.
 * @return the size of the data in the input buffer
 */
int in_buffer_len(void) {
	return in_ptr - in_buffer;
}

/// The ISR on USAR received data
/**
 * Reads the available character from USART and stores it into the input buffer if possible.  The character is echoed back to the user if the value is stored in the buffer.
 * If the buffer is full, a bell character is sent back to the user to alert them that their input was rejected.  If the user enters a backspace, the previous character is deleted from the input buffer.
 */
ISR (USART0_RX_vect) {
	char user_input = UDR0;
	if (in_buffer_ready) {
		// Tell the user that the buffer is full with a bell
		send_msg("\a");
	} else {
		if (user_input == '\r') {
			// Replace \n with \0 to terminate the string.
			*(in_ptr) = '\0';
			// New line character means the input is ready for the user.
			in_buffer_ready = 1;
			return;
		}
		// Echo the user's input back to the console
		UDR0 = user_input;
		// Avoid overflowing the in_buffer
		if (in_buffer_len() < IN_BUFFER_SIZE - 1) {
			if (user_input == 127) {
				// User entered backspace
				if (in_buffer_len() > 0) {
					// Delete the character that has been erased
					in_ptr--;
				} else {
					// The cursor is at the start of the buffer, backspace is not allowed
					send_msg("\a");
				}
			} else {
				*(in_ptr++) = user_input;				
			}
		} else {
			// The buffer is full
			in_buffer_ready = 1;
		}
	}
}

/// UART sending interrupt
/**
 * Sends one byte of the output buffer until the buffer is empty.  When the buffer is empty, the buffer empty flag is set.
 */
ISR (USART0_TX_vect) {
	if (!out_buffer_empty) {
		if (*out_ptr != '\0') {
			UDR0 = *(out_ptr++);
		} else {
			out_buffer_empty = 1;
		}
	}
}