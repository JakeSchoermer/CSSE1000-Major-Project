/*
** score.c
**
** Original version by Peter Sutton
*/

#include <stdint.h>

uint16_t score;

void init_score(void) {
	score = 0;
}

void add_to_score(uint16_t value) {
	score += value;
	
	if (score <=0) {
		score = 0;
	}

}

uint16_t get_score(void) {
	return score;
}
