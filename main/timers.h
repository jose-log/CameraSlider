
#ifndef TIMERS_H
#define TIMERS_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include <stdint.h>

/******************************************************************************
********************* E X T E R N A L   V A R I A B L E S *********************
******************************************************************************/

extern volatile uint16_t ms;

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

// Motor timer functions
void timer_speed_init(void);
void timer_speed_set(uint8_t state, uint16_t t);
void timer_speed_set_raw(uint16_t c);
uint8_t timer_speed_check(void);
uint16_t timer_speed_get(void);

// General timer functions
void timer_general_init(void);
void timer_general_set(uint8_t state);

// Auxiliary timer functions
void timer_aux_init(void);
void timer_aux_set(uint8_t state, uint8_t t);

#endif /* TIMERS_H */