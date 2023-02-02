
#ifndef __USART_H__
#define __USART_H__

#include "config.h"
#include <stdint.h>
#include <stdio.h>

// Define baud rate
#define USART_BAUDRATE 115200
// #define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define BAUD_PRESCALE (uint16_t)(F_CPU / (16.0 * USART_BAUDRATE) - 0.5)

void usart_init(void);
void usart_puts(const char *line);
int usart_putc(char c, FILE *stream);
char usart_getc(void);

#endif //__USART_H__
