
#include "util.h"
#include "config.h"

#include <stdint.h>

uint16_t millis(void)
{
	return ms;
}

void clear_millis(void)
{
	ms = 0;
}