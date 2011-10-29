#include <stdlib.h>
#include <avr/interrupt.h>
#include "game.h"
#include "project.h"

void init_sfx(void) {
	
	/*Setup JE*/

	/*
		PD6
		PB5
		PD4
		PB6

	*/
	DDRD |= (1<<6);
	DDRB |= (1<<5); 
	DDRD |= (1<<4);
	DDRB |= (1<<6);

	//Setup Timer
	OCR1C = 9999;
    TCCR3A = 0x00; 
    TCCR3B = 0xA;

}

ISR(TIMER3_COMPA_vect) {

	PORTD = PORTD ^ (1<<6);
	PORTB = PORTB ^ (1<<5); 
	PORTD = PORTD ^ (1<<4);
	PORTB = PORTB ^ (1<<6);

}
