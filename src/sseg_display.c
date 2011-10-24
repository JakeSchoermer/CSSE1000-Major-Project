#include <stdlib.h>
#include <avr/interrupt.h>
#include "game.h"
#include "sseg_display.h"
#include "score.h"
#include "project.h"



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
		/* Display a digit */ 
    if(seven_seg_cat == 0) { 
        /* Display rightmost digit*/ 
        PORTF = seven_seg_data[get_score()%10]; 
    } else { 
        /* Display leftmost digit*/ 
        PORTF = seven_seg_data[get_score()/10] | 0x80; 
    } 
 
    /* Change which digit will be displayed next - toggle 
    ** the least significant bit. 
    */ 
    seven_seg_cat ^= (1 << 0);			

}
