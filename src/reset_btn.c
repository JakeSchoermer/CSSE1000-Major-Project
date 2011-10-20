#include "game.h"
#include <stdlib.h>
#include <avr/interrupt.h>

/* Inititalise PMOD on JH
**		- Reset Button on Pin7 on DDRD (Maps to Button 0 on PMOD)
*/

void init_reset_btn(void) {
	
	DDRD |= (1 << 7);

}
