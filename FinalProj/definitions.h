/*
 * definitions.h
 *
 * Created: 4/9/2016 10:23:51 PM
 *  Author: jmay
 */ 


#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define USART_DOUBLE_TRANSMIT 1 // 0 = single transmit speed, 1 = double transmit speed
#define USART_SYNCHRONOUS 0		// 0 = asychronous, 1 = synchronous
#define USART_PARITY 0			// 0 = disabled, 2 = even, 3 = odd
#define USART_STOP_BITS 0		// 0 = 1 stop bit, 1 = 2 stop bits
#define USART_DATA_BITS 3		// 0 = 5 bits, 1 = 6 bits, 2 = 7 bits, 3 = 8 bits
#define USART_UBRR_BAUD 34		// See data sheet (34=57.6k baud)

#endif /* DEFINITIONS_H_ */