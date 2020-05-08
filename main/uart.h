
#ifndef UART_H
#define UART_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"

#include <stdint.h>

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void uart_init(void);
void uart_send_char(char data);
void uart_send_string(const char *s);
void uart_send_string_p(const char *s);
char uart_read_char(void);
void uart_set(uint8_t state);

#endif /* UART_H */