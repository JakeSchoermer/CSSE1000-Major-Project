#include "game.h"
#include "sseg_display.h"
#include "score.h"
#include <stdlib.h>
#include <avr/interrupt.h>

//Global Variables
uint8_t seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};
volatile uint8_t seven_seg_cat = 0; 


void init_sseg_score_display(void) {
	
	//Set output to JH
	DDRF = 0xFF;
	
	//PORTF = 0xFF;

	//Setup Timer
	OCR1A = 99; 
    TCCR1A = 0x00; 
    TCCR1B = 0xA;

	//Enable interrupt on timer on output compare match  
	 
    TIMSK |= (1 << 4); 
 
    /* Ensure interrupt flag is cleared */ 
    TIFR |= (1 << 4); 

}



ISR(TIMER1_COMPA_vect) {
	uint8_t score = get_score();


		/* Display a digit */ 
    if(seven_seg_cat == 0) { 
        /* Display rightmost digit*/ 
        PORTF = seven_seg_data[1]; 
    } else { 
        /* Display leftmost digit*/ 
        PORTF = seven_seg_data[2] | 0x80; 
    } 
 
    /* Change which digit will be displayed next - toggle 
    ** the least significant bit. 
    */ 
    seven_seg_cat ^= (1 << 0);			

}
