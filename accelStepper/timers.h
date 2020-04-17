

#ifndef TIMERS_H
#define TIMERS_H

#include <stdint.h>

void speed_timer_init(void);

void speed_timer_set(uint8_t state, uint16_t t);

void speed_timer_set_raw(uint16_t c);

void aux_timer_init(void);

void aux_timer_set(uint8_t state, uint16_t t);

#endif /* TIMERS_H */