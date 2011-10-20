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
	OCR1A = 1; 
    TCCR1A = 0x00; 
    TCCR1B = 0xA;
}



ISR(TIMER1_COMPA_vect) {
	PORTF = 0xFF;
	uint8_t score = get_score();

	//PORTF = 0xFF;

		/* Display a digit */ 
    if(seven_seg_cat == 0) { 
        /* Display rightmost digit*/ 
        PORTF = 0xF0;//seven_seg_data[score%10]; 
    } else { 
        /* Display leftmost digit*/ 
        PORTF = 0x0F;//seven_seg_data[score/10] | 0x80; 
    } 
 
    /* Change which digit will be displayed next - toggle 
    ** the least significant bit. 
    */ 
    seven_seg_cat ^= (1 << 0);			

}
