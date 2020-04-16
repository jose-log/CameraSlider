

#ifndef TIMERS_H
#define TIMERS_H

#include <stdint.h>

void speed_timer_init(void);

void speed_timer_set(uint8_t state, uint16_t t);

void speed_timer_set_raw(uint16_t c);

void general_timer_init(void);

void general_timer_set(uint8_t state);

#endif /* TIMERS_H */