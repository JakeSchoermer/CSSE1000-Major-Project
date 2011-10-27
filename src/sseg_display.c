#include <stdlib.h>
#include <avr/interrupt.h>
#include "game.h"
#include "sseg_display.h"
#include "score.h"
#include "project.h"

int pos = 1;
int counter = 0;

void init_sseg_score_display(void) {
	
	//Set output to JH
	DDRF = 0xFF;
	
	//PORTF = 0xFF;

	//Setup Timer
	OCR1A = 9999; 
    TCCR1A = 0x00; 
    TCCR1B = 0xA;

	//Enable interrupt on timer on output compare match  
	 
    TIMSK |= (1 << 4); 
 
    /* Ensure interrupt flag is cleared */ 
    TIFR |= (1 << 4);

}



ISR(TIMER1_COMPA_vect) {

	
	int show;

	if (show_high_score == 1) {
		show = high_score;
	}	
	else {
		show = get_score();
	}


	//Show Scores Greater than 99;
	counter ++; //Use to time scrolling
	
	if (counter == 10000) {
		counter == 0; //reset counter;
		//if (pos < show/10) {
		//	pos++;
		//}
		//else {
		///	pos = 1; //Reset Pos
		//}
	}
	
	
	int leftIdx = show%(10^pos);
	int rightIdx = pos/(10^pos);

	/* Display a digit */ 
	if(seven_seg_cat == 0) { 
		/* Display rightmost digit*/ 
		PORTF = seven_seg_data[leftIdx]; 
	} else { 
	    /* Display leftmost digit*/ 
	    PORTF = seven_seg_data[rightIdx] | 0x80; 
 	}

    /* Change which digit will be displayed next - toggle 
    ** the least significant bit. 
    */ 
    seven_seg_cat ^= (1 << 0);			

	show_high_score = 0;
	

	//Reset Score and Add 1 Health;

	if (get_score() == 100) {
		if (getHealth < 4) {
			setHealth(getHealth()+1);
			outputHealth();
		}
	}

}
