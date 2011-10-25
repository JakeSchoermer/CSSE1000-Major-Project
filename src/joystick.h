/* 
** joystick.h
**
** Global variables and functions for interacting with a Joystick PMOD
** connected to the AVR SPI port - lower few bits of port B.
*/

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>

/* Global variables which indicate the current state of the joystick.
** joystickX will be -2 if the joystick is hard left, -1 if partially 
** left, 0 if centered, +1 if partially right, +2 if hard right.
** Similarly, joystickY will be -2 to 2 indicating positions from down
** to up.
** The lower three bits of joystickButtons indicate whether the following
** buttons on the Joystick PMOD are depressed:
**    bit 2 - BTN2
**    bit 1 - BTN1
**    bit 0 - Joystick lever itself
*/
extern volatile int8_t joystickX;
extern volatile int8_t joystickY;
extern volatile uint8_t joystickButtons;

/* Macros to check the Joystick buttons */
#define BUTTON_1_PRESSED(buttons) ((buttons) & 0x02)
#define BUTTON_2_PRESSED(buttons) ((buttons) & 0x04)

/* init_joystick()
** - must be called before the joystick is used. Sets up the SPI control
** registers appropriate and the data direction registers
*/
void init_joystick(void);

/* set_joystick_leds()
** - set the joystick LED values the next time that the joystick is 
** communicated with. A true value indicates the LED should be on.
*/
void set_joystick_leds(uint8_t led1, uint8_t led2);

/* joystick_update()
** - gets the current joystick position and button status (global variables
** above will be updated), and update LEDs on the joystick. Note that this 
** function does not return until communication with the joystick is complete. 
** This can take over 140 microseconds.
*/
void joystick_update(void);

#endif
