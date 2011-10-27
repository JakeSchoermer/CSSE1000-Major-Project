/*
** project.c - main file 
**
** Original version by Peter Sutton
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#define _CSSE1000_MAIN
#include "game.h"
#include "joystick.h"
#include "led_display.h"
#include "score.h"
#include "timer2.h"
#include "scrolling_char_display.h"
#include "sseg_display.h"
#include "pmod.h"



/*Global Variables*/

uint16_t high_score = 0;
uint8_t show_high_score = 0;

uint8_t seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};
uint8_t seven_seg_cat = 0; 

/*
** Function prototypes - these are defined below main()
*/
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void handle_game_over(void);
void game_pause_loop(void);
void wait_seconds(int);

/*
 * main -- Main program.
 */
int main(void) {
	/* Flag to keep track of whether the game field changes or.
	** We use this to know whether we must redraw the dispaly
	** or not.
	*/
	uint8_t gameFieldUpdated = 0;

	/* Keep track of the previous joystick values - so we know
	** whether the joystick (X position or buttons) have changed
	** or not.
	*/
	int8_t prevJoystickX;
	uint8_t prevJoystickButtons;
	uint32_t currentTime;				/* clock ticks */
	uint32_t displayLastUpdatedTime = 0;	/* clock ticks */
	uint32_t joystickLastCheckedTime = 0;	/* clock ticks */
	uint32_t projectilesLastAdvancedTime = 0; /* clock ticks */
	uint32_t asteroidsLastAdvancedTime = 0; /* clock ticks */
	
	initialise_hardware();

	/* Show the splash screen message. This returns when 
	** message display is complete. */
	//splash_screen();
	
	/* Perform necessary initialisations for a new game. */
	new_game();
		
	/*
	** Event loop. We wait for various times to be reached
	** to take actions (e.g. advancing projectiles up the 
	** screen.) We monitor various button values to check
	** whether they have changed.
	*/
	prevJoystickX = 0;
	prevJoystickButtons = 0;
	while(1) {
		currentTime = get_clock_ticks();
		if(currentTime >= projectilesLastAdvancedTime + 1000) {
			/* Advance any projectiles every 1000ms. */
			gameFieldUpdated |= advance_projectiles();
			projectilesLastAdvancedTime = currentTime;
		}

		if(currentTime >= asteroidsLastAdvancedTime + getAsteroidFallInterval()) {
			/* Advance any asteroids. */
			gameFieldUpdated |= advance_asteroids();
			asteroidsLastAdvancedTime = currentTime;
		}

		/* Check clock tick value and take action if necessary */
		
		if(currentTime >= displayLastUpdatedTime + 2) {
			/* Update LED display every 2ms - i.e. show a different row */
			display_row();
			displayLastUpdatedTime = currentTime;
		}

		if(currentTime >= joystickLastCheckedTime + 4) {
			/* Check the joystick every 4ms */
			joystick_update();
			joystickLastCheckedTime = currentTime;
		}

		if(prevJoystickX != joystickX) {
			/* Joystick has moved left or right */
			if(prevJoystickX >= 0 && joystickX < 0) {
				/* Joystick has moved left */ 
				gameFieldUpdated |= move_base(MOVE_LEFT);
			}
			if(prevJoystickX <= 0 && joystickX > 0) {
				gameFieldUpdated |= move_base(MOVE_RIGHT);
			}
			/* Update our record of the previous Joystick value */
			prevJoystickX = joystickX;
		}
		if(prevJoystickButtons != joystickButtons) {
			/* A joystick button has been pressed or released */
			if(BUTTON_1_PRESSED(joystickButtons) && 
					!BUTTON_1_PRESSED(prevJoystickButtons)) {
				/* Button one has been pressed */
				gameFieldUpdated = fire_projectile();
			}
			prevJoystickButtons = joystickButtons;
		}
		if(gameFieldUpdated) {
			/* 
			** Update display of board since its appearance has changed.
			*/
			copy_game_field_to_led_display();
			
			// Update Health Output
			outputHealth(getHealth());
			
			gameFieldUpdated = 0;
		}
		//Reset Button
		if((PIND & (1<<7)) == (1<<7)) {
			
			while((PIND & (1<<7)) == (1<<7)) {
			}
			if (high_score < get_score()) {
				high_score = get_score();
			}
			add_to_score(10);
			new_game();
		}

		//High Score
		if ((PINB & (1<<4)) == (1<<4)) {
			show_high_score = 1;
		}
		
		// Pause Game
		if ((PIND & (1<<9)) == (1<<9)) {
			while ((PIND & (1<<9)) == (1<<9)) { /* wait for button release */ }
			game_pause_loop();
		}
	}
}

void game_pause_loop() {
	uint32_t currentTime;
	uint32_t displayLastUpdatedTime = 0;
	uint32_t displayLastScrolledTime = 0;
	
	/* This is the text we'll scroll on the LED display. */
	//set_display_text("Jake Schoermer s4233158 Sam Pengilly s42351382");
	set_display_text("Paused");
	
	/* We scroll the message until the display is blank */
	while(1) {
		currentTime = get_clock_ticks();
		
		if(currentTime >= displayLastUpdatedTime + 2) {
			/* Update LED display every 2ms - i.e. show a different row */
			display_row();
			displayLastUpdatedTime = currentTime;
		}
		
		if(currentTime >= displayLastScrolledTime + 150) {
			/* Scroll our message every 150ms. Exit the loop
			 ** if finished.
			 */
			if(!scroll_display()) {
				//break;
			}
			displayLastScrolledTime = currentTime;
		}

		// Unpause Game
		if ((PIND & (1<<9)) == (1<<9)) {
			while ((PIND & (1<<9)) == (1<<9)) { /* wait for button release */ }
			break;
		}
	}
}

void initialise_hardware(void) {
	/* Initialise hardware modules (interrupts, data direction
	** registers etc. This should only need to be done once.
	*/

	/* Initialise the LED board display */
	init_display();

	/* Initialise communication with the Joystick */
	init_joystick();

	/* Initialise the timer which gives us clock ticks
	** to time things by.
	*/
	init_timer2();

	/* Initialise SSEG Score
	**
	*/
	
	init_sseg_score_display();
	
	
	/* Inititalise PMOD on JF
	**		- Reset Button on Pin6
	*/
	
	init_pmod();

	/*
	** Turn on interrupts (needed for timer to work)
	*/

	sei();
}

void splash_screen(void) 
{
	uint32_t currentTime;
	uint32_t displayLastUpdatedTime = 0;
	uint32_t displayLastScrolledTime = 0;

	/* This is the text we'll scroll on the LED display. */
	//set_display_text("Jake Schoermer s4233158 Sam Pengilly s42351382");
	set_display_text("s");

	/* We scroll the message until the display is blank */
	while(1) {
		currentTime = get_clock_ticks();
		
		if(currentTime >= displayLastUpdatedTime + 2) {
			/* Update LED display every 2ms - i.e. show a different row */
			display_row();
			displayLastUpdatedTime = currentTime;
		}

		if(currentTime >= displayLastScrolledTime + 150) {
			/* Scroll our message every 150ms. Exit the loop
			** if finished.
			*/
			if(!scroll_display()) {
				break;
			}
			displayLastScrolledTime = currentTime;
		}
	}		
}

void new_game(void) 
{
	/* 
	** Initialise the game field and the screen
	*/
	init_game_field();
	copy_game_field_to_led_display();
	init_score();
}


/************* Internal Functions added by Jake ****************************/

void wait_seconds (seconds) {
	/* We use inline assembly language */

	for (int i = 0; i <1000*seconds; i++) {
		asm("nop\r\n" "nop\r\n" "nop\r\n" "nop\r\n"::);
	}
}


