/* timer2.h
**
** We set up timer 2 to give us an interrupt
** every millisecond. Tasks that have to occur
** regularly (every few milliseconds) can be added 
** to the interrupt handler (in timer2.c) or can
** be added to the main event loop that checks the
** clock tick value. This value (32 bits) can be 
** obtained using the get_clock_ticks() function.
*/

#ifndef TIMER2_H
#define TIMER2_H

#include <stdint.h>

void init_timer2(void);

uint32_t get_clock_ticks(void);

#endif
