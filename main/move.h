
#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>

uint8_t move_to_position(uint16_t pos, uint16_t speed);

uint8_t move_relative(uint16_t pos, uint8_t dir, uint16_t speed);

void homing_cycle(void);

void speed_set(uint16_t speed);

uint16_t speed_limit_correction(void);

#endif /* MOVE_H */