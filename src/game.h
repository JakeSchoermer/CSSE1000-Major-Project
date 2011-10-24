/*
** game.h
**
** Written by Peter Sutton
**
** Function prototypes for those functions available externally
*/

#include <inttypes.h>

/*
** The game field is 15 rows in size by 7 columns, i.e. x (column number)
** ranges from 0 to 6 (left to right) and y (row number) ranges from
** 0 to 14 (bottom to top).
*/
#define FIELD_HEIGHT 15
#define FIELD_WIDTH 7

/*
** Limits on the number of asteroids and projectiles we can have on the 
** game field at any one time. (These numbers should fit within the 
** range of an int8_t type - i.e. max 127, though in reality
** there are tighter constraints than this - e.g. there are only 105
** positions on the game field.)
*/
#define MAX_PROJECTILES 4
#define MAX_ASTEROIDS 20

/* Arguments that can be passed to attempt_move() below */
#define MOVE_LEFT 0
#define MOVE_RIGHT 1

/*
** Initialise the game field.
*/
void init_game_field(void); 

/* 
** Copy game field (base station, projectiles, asteroids) to LED display 
*/
void copy_game_field_to_led_display(void);

/*
** Fire a projectile - release a projectile from the base station.
** Returns 1 if successful, 0 otherwise (e.g. already a projectile
** which is in the position immediately above the base station, or
** the maximum number of projectiles in flight has been reached.
*/
int8_t fire_projectile(void);

/* 
** Advance the projectiles that have been fired. Returns 1 if any
** projectiles moved, 0 otherwise.
*/
int8_t advance_projectiles(void);

/*
** Advance the Asteroids
*/
int8_t advance_asteroids(void);

int getAsteroidFallInterval();
int getHealth();

/* 
** Attempt to move the base station to the left or the right. Returns
** 1 if successful, 0 otherwise (e.g. already at edge). The "direction"
** argument takes on the value MOVE_LEFT or MOVE_RIGHT (see above).
*/
int8_t move_base(int8_t direction);

