
#include "usart.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>

static FILE usartout = FDEV_SETUP_STREAM(usart_putc, NULL, _FDEV_SETUP_WRITE);

/**
 * @brief Print a line of characters to USART channel
 *
 * @param line character array to print
 */
void usart_puts(const char *line) {
  for (size_t i = 0; i < strlen(line); i++) {
    usart_putc(line[i], stdout);
  }
}

void usart_init(void) {
  stdout = &usartout;
  /* Load upper 8-bits into the high byte of the UBRR register
      Default frame format is 8 data bits, no parity, 1 stop bit
      to change use UCSRC, see AVR datasheet */
  cli();
  // Enable receiver and transmitter and receive complete interrupt
  UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (0 << UCSZ01) |
           (1 << RXCIE0); //|(1<<TXCIE1);
  // 8-bit data, 1 stop bit, Aynchronous USART, no parity
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (0 << USBS0) | (0 << UMSEL01) |
           (0 << UMSEL00) | (0 << UPM01) | (0 << UPM00);
  UBRR0H = (uint8_t)(BAUD_PRESCALE >> 8);
  UBRR0L = (uint8_t)(BAUD_PRESCALE); // Load lower 8-bits into the low byte of
                                     // the UBRR register
  sei();
}

int usart_putc(char c, FILE *stream) {
  if (c == '\n') {
    usart_putc('\r', stream);
  }
  // wait until buffer is ready to receive new data
  while ((UCSR0A & (1 << UDRE0)) == 0)
    ;
  // write to USART data register TXD
  UDR0 = c;
  return 0;
}

char usart_getc(void) {
  while ((UCSR0A & (1 << RXC0)) == 0)
    ;
  // read from USART data register RXD
  return UDR0;
}