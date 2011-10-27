#include "game.h"
#include <stdlib.h>
#include <avr/interrupt.h>

/* Inititalise PMOD on JH
**		- Reset Button on Pin7 on DDRD (Maps to Button 0 on PMOD)
*/

void init_pmod(void) {
	
	/* Button PMOD */
		
	DDRD |= (0 << 7); //Reset Button
	

	DDRB |= (0 << 4); //High Score Button
	
	DDRD |= (0 <<5); //Pause Button	

	/* LED PMOD */

	DDRE |= 0xF0;

	//Start with Full Health

	PORTE |= 0xF0;

}

void outputHealth(int health) {
	switch (health) {
		case 0:
			PORTE = 0x00;
			break;
		
		case 1:
			PORTE = (0xF0>>3);
			break;
			
		case 2:
			PORTE = (0xF0>>2);
			break;
			
		case 3:
			PORTE = (0xF0>>1) ;
			break;
			
		case 4:
			PORTE = 0xF0;
			break;
	}
}
