/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file uart.c
 * @brief Serial communication
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(void);

void uart_send_char(char data);

void uart_send_string(const char *s);

void uart_send_string_p(const char *s);

char uart_read_char(void);

void uart_set(uint8_t state);

#endif /* UART_H */