#include "game.h"
#include "sseg_display.h"
#include <stdlib.h>
#include <avr/interrupt.h>


void init_sseg_score_display(void) {
	
	//Set output to JH

	DDRF = 0xFF;
	
}
