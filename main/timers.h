
#ifndef TIMERS_H
#define TIMERS_H

#include <stdint.h>

void timer_speed_init(void);

void timer_general_init(void);

void timer_aux_init(void);

void timer_speed_set(uint8_t state, uint16_t t);

void timer_speed_set_raw(uint16_t c);

void timer_general_set(uint8_t state);

void timer_aux_set(uint8_t state, uint8_t t);

uint8_t timer_speed_check(void);

uint16_t timer_speed_get(void);

#endif /* TIMERS_H */