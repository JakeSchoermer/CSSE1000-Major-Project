/*
** game.c
**
** Original version by Peter Sutton
**
*/

#include "game.h"
#include "led_display.h"
#include "score.h"
#include "pmod.h"
#include <stdlib.h>
#include <avr/interrupt.h>
/* Stdlib needed for rand() - random number generator */


/*
** Global variables.
**
** basePosition - stores the x position of the centre point of the 
** base station. The base station is three positions wide, but is
** permitted to partially move off the game field so that the centre
** point can take on any position from 0 to 6 inclusive.
**
** numProjectiles - The number of projectiles currently in flight. Must
** be less than or equal to MAX_PROJECTILES.
**
** projectiles - x,y positions of the projectiles that are currently
** in flight. The upper 4 bits represent the x position; the lower 4
** bits represent the y position. The array is indexed by projectile
** number from 0 to numProjectiles - 1.
**
** numAsteroids - The number of asteroids currently on the game field.
** Must be less than or equal to MAX_ASTEROIDS.
**
** asteroids - x,y positions of the asteroids on the field. The upper
** 4 bits represent the x position; the lower 4 bits represent the 
** y position. The array is indexed by asteroid number from 0 to 
** numAsteroids - 1.
**
*/
int8_t    basePosition;
int8_t    numProjectiles;
uint8_t    projectiles[MAX_PROJECTILES];
int8_t    numAsteroids;
uint8_t    asteroids[MAX_ASTEROIDS];
int		  health;

/************************************************************ 
** Prototypes for internal information functions 
**  - not available outside this module.
*/
/* Is there is an asteroid/projectile at the given position?. 
** Returns -1 if no, asteroid/projectile index number if yes.
** (The index number is the array index in the asteroids/
** projectiles array above.)
*/
static int8_t asteroid_at(uint8_t x, uint8_t y);
static int8_t projectile_at(uint8_t x, uint8_t y);

/* Remove the asteroid/projectile at the given index number. If
** the index is not valid, then no removal is performed. This 
** enables the functions to be used like:
**		remove_asteroid(asteroid_at(x,y));
*/ 
static void remove_asteroid(int8_t asteroidIndex);
static void remove_projectile(int8_t projectileIndex);

/*
** Asteroid / Projectile Maintenance Functions
*/
void gameOver();
void decrementHealth();
void handleCollision(int8_t projectileIndex, int8_t asteroidIndex);
void handleBaseCollision();
void replaceAsteroid();
int8_t createAsteroid(int8_t x, int8_t y);

/***********************************************************/

/* 
** Initialise game field:
** (1) base starts in the centre (x=3)
** (2) no projectiles initially
** (3) the maximum number of asteroids, randomly distributed.
*/
void init_game_field(void) {
	uint8_t x, y, i;
    basePosition = 3;
	numProjectiles = 0;
	numAsteroids = 0;

	for(i=0; i < MAX_ASTEROIDS ; i++) {
		/* Generate random position that does not already
		** have an asteroid.
		*/
		do {
			/* Generate random x position - somewhere from 0
			** to FIELD_WIDTH - 1
			*/
			x = (uint8_t)(rand() % FIELD_WIDTH);
			/* Generate random y position - somewhere from 3
			** to FIELD_HEIGHT - 1 (i.e., not in the lowest
			** three rows)
			*/
			y = (uint8_t)(3 + (rand() % (FIELD_HEIGHT-3)));
		} while(asteroid_at(x,y) != -1);
		/* If we get here, we've now found an x,y location without
		** an existing asteroid - record the position
		*/
		asteroids[i] = (x<<4)|y;
		numAsteroids++;
	}
	health = 4;
	outputHealth(health);
}

/* 
** Copy field to LED display. Note that difference in definitions of
** rows and columns for the field and the LED display. The game field
** has 15 rows (numbered from the bottom), each 7 bits wide (with the 
** 7 columns numbered as per the bits - i.e. least significant (0) on 
** the right). The LED display has 7 rows (0 at the top, 6 at the bottom) 
** with 15 columns (numbered from 0 at the left to 14 at the right).
*/
void copy_game_field_to_led_display(void) {
	/* The field has FIELD_HEIGHT rows (e.g. 15) - ranging from y=0 (bottom)
	** to y=14 (top). These correspond to columns 0 to 14 on the LED
	** display. The field columns (from x=0 (left) to x=6 (right)
	** correspond to LED display rows 0 to 6.
	** The function updates our LED display to reflect the 
	** current state of the field.
	*/
	int8_t x, y;
	uint8_t i;
	uint16_t ledBoard[FIELD_WIDTH];

	/* Start with an empty board */
	for(x=0; x < FIELD_WIDTH; x++) {
		ledBoard[x] = 0;
	}

	/* Add the asteroids */
	for(i=0; i < numAsteroids; i++) {
		x = asteroids[i] >> 4;
		y = asteroids[i] & 0x0F;
		ledBoard[x] |= (1<<y);
	}

	/* Add the projectiles */
	for(i=0; i < numProjectiles; i++) {
		x = projectiles[i] >> 4;
		y = projectiles[i] & 0x0F;
		ledBoard[x] |= (1<<y);
	}

	/* Add the base - bottom row (0) first, followed
	** by the single bit in the next row (1).
	*/
	for(x=basePosition - 1; x <= basePosition+1; x++) {
		if(x >= 0 && x < FIELD_WIDTH) {
			ledBoard[x] |= 1;
		}
	}
	ledBoard[basePosition] |= (1<<1);

	/* Have now generated what the board should look like.
	** Copy it to the LED display variable (from which the 
	** display is drawn). We turn off interrupts while we
	** do this since the display array is read during an
	** interrupt handler (when we call display_row()) 
	** and we don't want a semi-updated display variable 
	** used for the update.
	*/
	uint8_t interrupts_enabled = bit_is_set(SREG, SREG_I);
	cli();
	for(i=0; i < FIELD_WIDTH; i++) {
		display[i] = ledBoard[i];
	}
	if(interrupts_enabled) {
		sei();
	}
}

/*
** Attempt to move the base station to the left or right. 
** The direction argument has the value MOVE_LEFT or
** MOVE_RIGHT. The move succeeds if the base isn't all 
** the way to one side, e.g., not permitted to move
** left if basePosition is already 0.
** Returns 1 if move successful, 0 otherwise.
*/
int8_t move_base(int8_t direction) {	
	/* YOUR CODE HERE - FIX THIS FUNCTION */

	if (basePosition > 0 && direction == MOVE_LEFT) {
		basePosition--;
		handleBaseCollision();
		return 1;
	}

	else if (basePosition <6 && direction == MOVE_RIGHT) {
		basePosition++;
		handleBaseCollision();
		return 1;
	}

	return 0;
}

/*
** Fire projectile - add it immediately above the base
** station, provided there is not already a projectile
** there. We are also limited in the number of projectiles
** we can have in flight (to MAX_PROJECTILES).
** Returns 1 if projectile fired, 0 otherwise.
*/
int8_t fire_projectile(void) {
	if(numProjectiles < MAX_PROJECTILES && 
			projectile_at(basePosition, 2) == -1) {
		/* Have space to add projectile */
		projectiles[numProjectiles++] = (basePosition<<4)|2;
		
		// Check for collision with asteroid right in front of base station
		int8_t asteroidIndex;
		//if (asteroidIndex = asteroid_at(basePosition, 2)) {
		//	handleCollision(numProjectiles, asteroidIndex);
		//}
		
		return 1;
	} else {
		return 0;
	}
}

/* Move projectiles up by one position, and remove those that 
** have gone off the top. Returns 1 if any projectiles moved,
** 0 otherwise. (Will return 1 if we had any projectiles at
** all at the start.)
** MODIFY THIS CODE TO CHECK FOR A HIT BETWEEN THE PROJECTILE
** AND AN ASTEROID AND (1) REMOVE BOTH, AND (2) ADJUST AND
** DISPLAY THE SCORE.
*/
int8_t advance_projectiles(void) {
	uint8_t x, y;
	int8_t projectileIndex;
	int8_t projectilesMoved = (numProjectiles > 0)?1:0;

	projectileIndex = 0;
	while(projectileIndex < numProjectiles) {
		x = projectiles[projectileIndex] >> 4;
		y = projectiles[projectileIndex] & 0x0F;
		if(y < 14) {
			/* Projectile has not yet reached the top - advance
			** the y position.
			*/
			y++;
			
			// Check for collision, If it occurs remove the projectile and asteroid
			// and continue to the next projectile
			int asteroidIndex;
			if ((asteroidIndex = asteroid_at(x,y)) != -1) {
				// Handle Collision
				handleCollision(projectileIndex, asteroidIndex);
				
				// Continue to the next asteroid
				numAsteroids++;
				continue;
			}
			
			/* Update the projectile position */
			projectiles[projectileIndex] = (x<<4)|y;
			/* Move on to the next projectile */
			projectileIndex++;


		} else {
			/* Projectile has reached the top - remove it. */
			remove_projectile(projectileIndex);

			/* Another projectile may have now taken on this same
			** projectileIndex value, so we don't increment it here.
			** Note that numProjectiles will have decreased by 1.
			*/
		}
	}
	return projectilesMoved;
}

int8_t advance_asteroids(void) {
	uint8_t x, y;
	int8_t asteroidIndex;
	int8_t asteroidsMoved = (numAsteroids > 0) ? 1 : 0;
	
	asteroidIndex = 0;
	while (asteroidIndex < numAsteroids) {
		// Get current asteroid position
		x = asteroids[asteroidIndex] >> 4;
		y = asteroids[asteroidIndex] & 0x0F;
		
		// If the asteroid hasn't reached the bottom
		if (y > 0) {
			
			// Advance the asteroid downwards
			y--;
			
			// Handle collisions between projectiles and asteroids
			// If collision occurs, remove both and continue to the next projectile
			int8_t projectileIndex;
			if ((projectileIndex = projectile_at(x, y)) != -1) {
				// Handle Collision
				handleCollision(projectileIndex, asteroidIndex);
				
				// Move on to next asteroid
				asteroidIndex++;
				continue;
			}
			
			/**
			 * Check If Asteroid Collides With Base Station
			 *
			 */
			
			// Update asteroid position
			asteroids[asteroidIndex] = (x << 4) | y;
			
			// Move on to the next asteroid
			asteroidIndex++;
		} else {
			// Asteroid reached bottom, remove it
			remove_asteroid(asteroidIndex);
			
			//replaceAsteroid();
		}
	}
	
	// Handle Collisions between base station and asteroids
	handleBaseCollision();
	
	return asteroidsMoved;
}

int getAsteroidFallInterval() {
	int interval = 5000 - (get_score() * 100);
	if (interval <= 500) {
		interval = 500;
	}
	return interval;
}

int getHealth() {
	return health;
}

void setHealth(newHealth) {
	health = newHealth;
}



/******** INTERNAL FUNCTIONS ****************/

void gameOver() {
	uint32_t currentTime;
	uint32_t displayLastUpdatedTime = 0;
	uint32_t displayLastScrolledTime = 0;
	
	/* This is the text we'll scroll on the LED display. */
	//set_display_text("Jake Schoermer s4233158 Sam Pengilly s42351382");
	set_display_text("GAME OVER");
	
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
	
	new_game();
}

void handleCollision(int8_t projectileIndex, int8_t asteroidIndex) {
	// Remove colliding objects
	remove_projectile(projectileIndex);
	remove_asteroid(asteroidIndex);

	replaceAsteroid();
	
	// Increase Score
	add_to_score(1);
}

void handleBaseCollision() {
	int8_t asteroidIndex;
	
	if ((asteroidIndex = asteroid_at(basePosition, 0)) != -1 ||
		(asteroidIndex = asteroid_at(basePosition - 1, 0)) != -1 ||
		(asteroidIndex = asteroid_at(basePosition + 1, 0)) != -1 ||
		(asteroidIndex = asteroid_at(basePosition, 1)) != -1) {
		remove_asteroid(asteroidIndex);
		// Decrement Lives
		health--;
		
		if (health <= 0) {
			gameOver();
		}
		
		add_to_score(-1);
		outputHealth(health);
		
		replaceAsteroid();
	}
}

void replaceAsteroid() {
	//Replacement Asteroids
	uint8_t newX = 0;
	uint8_t newY = 14;
	
	numAsteroids++;
	
	if (!numAsteroids >= MAX_ASTEROIDS) {

		// Find position that isn't occupied
		do {
			newX = (uint8_t)(rand() % FIELD_WIDTH);
			//newY = 13;
			//newY = (uint8_t)(3 + (rand() % (FIELD_HEIGHT-3)));
		
			asteroids[numAsteroids] = (newX<<4)|newY;	
		} while (asteroid_at(newX, newY) != -1);
		//createAsteroid(newX, newY);
	}
}

int8_t createAsteroid(int8_t x, int8_t y) {
	if (numAsteroids >= MAX_ASTEROIDS - 1) {
		return -1;
	} else {
		asteroids[MAX_ASTEROIDS - 1] = (x << 4) | y;
		return numAsteroids + 1;
	}
}

/* Check whether there is an asteroid at a given position.
** Returns -1 if there is no asteroid, otherwise we return
** the asteroid number (from 0 to numAsteroids-1).
*/
static int8_t asteroid_at(uint8_t x, uint8_t y) {
	uint8_t i;
	uint8_t positionToCheck = (x<<4)|y;
	for(i=0; i < numAsteroids; i++) {
		if(asteroids[i] == positionToCheck) {
			/* Asteroid i is at the given position */
			return i;
		}
	}
	/* No match was found - no asteroid at the given position */
	return -1;
}

/* Check whether there is a projectile at a given position.
** Returns -1 if there is no projectile, otherwise we return
** the projectile number (from 0 to numProjectiles-1).
*/
static int8_t projectile_at(uint8_t x, uint8_t y) {
	uint8_t i;
	uint8_t positionToCheck = (x<<4)|y;
	for(i=0; i < numProjectiles; i++) {
		if(projectiles[i] == positionToCheck) {
			/* Projectile i is at the given position */
			return i;
		}
	}
	/* No match was found - no projectile at the given position */
	return -1;
}

/* Remove asteroid with the given index number (from 0 to
** numAsteroids - 1).
*/
static void remove_asteroid(int8_t asteroidIndex) {
	if(asteroidIndex < 0 || asteroidIndex >= numAsteroids) {
		/* Invalid index - do nothing */
		return;
	}
	if(asteroidIndex < numAsteroids - 1) {
		/* Asteroid is not the last one in the list
		** - move the last one in the list to this position
		*/
		asteroids[asteroidIndex] = asteroids[numAsteroids - 1];
	}
	/* Last position in asteroids array is no longer used */
	numAsteroids--;
	
	//replaceAsteroid();
}

/* Remove projectile with the given index number (from 0 to
** numProjectiles - 1).
*/
static void remove_projectile(int8_t projectileIndex) {
	if(projectileIndex < 0 || projectileIndex >= numProjectiles) {
		/* Invalid index - do nothing */
		return;
	}
	if(projectileIndex < numProjectiles - 1) {
		/* Projectile is not the last one in the list
		** - move the last one in the list to this position
		*/
		projectiles[projectileIndex] = projectiles[numProjectiles - 1];
	}
	/* Last position in projectiles array is no longer used */
	numProjectiles--;
}
