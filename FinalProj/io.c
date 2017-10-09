#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"
#include "definitions.h"
#include "movement.h"
#include "scan.h"

void init_ir(void);

/// Initializes the UART for two-way serial communication with interrups
/**
 * Initializes the UART for 2 way communication with the values specified in definitions.h.
 * Interrupts are enabled.
 */
void init_UART() {
	// Set the double transmit speed
	UCSR0A = _BV(U2X) * USART_DOUBLE_TRANSMIT;
	// RX & TX interrupts and communication enabled
	UCSR0B = _BV(RXCIE) | _BV(TXCIE) | _BV(RXEN) | _BV(TXEN);
	// Mode select, Parity, Stop Bits, and character size
	UCSR0C = (_BV(UMSEL) * USART_SYNCHRONOUS) | (USART_PARITY << UPM0) | (USART_STOP_BITS << USBS) | (USART_DATA_BITS << UCSZ0);

	// Set BAUD rate
	UBRR0H = USART_UBRR_BAUD >> 8;
	UBRR0L = USART_UBRR_BAUD & 0xFF;
	
	sei();  // enable interrupts
}

/**
 * Initializes the data transfer between the microcontroller and the robot. Initializes all the necessary
 * registers and components for the cliff sensors, the motors, and the bump sensors.
 */
oi_t* init_iRobot()
{
    //This will need to be passed around
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);
	return sensor_data;
}

/// Initializes the servo motor.  Sets the servo to 90 degrees (straight ahead)
/**
 * Initializes Port E pin 4 for output of the PWM signal.  Sets the TOP value to a value that
 * is compatible with the calculations in scan.c.  Sets the servo to 90 degrees.
 */
void init_servo() {
	DDRE |= _BV(4);		// Set port E pin 4 as an output
	OCR3A = 43000 - 1;	// TOP - number of cycles in the interval
	set_servo_pos(90);		// move servo to the middle
	TCCR3A = 0x23;		// set COM and WGM (bits 3 and 2)
	TCCR3B = 0x1A;		// set WGM (bits 1 and 0) and CS
}

/// Initializes the ADC for reading values
/**
 * Initializes the ADC for a reference voltage of 2.56 V, using input 2, and a prescaler of /128
 */
void init_ir() {
	// Reference voltage is 2.56 V and the MUX to 2
	ADMUX = _BV(REFS1) | _BV(REFS0) | 2; // 0xC2;
	// Enables the ADC and sets the prescaler to /128
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // 0x87;
}