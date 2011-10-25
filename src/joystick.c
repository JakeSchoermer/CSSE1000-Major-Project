/*
** joystick.c
**
** Original version by Peter Sutton
**
** Implements SPI interface with the Joystick PMOD.
** See the Joystick PMOD reference manual for details.
*/

#include <avr/io.h>

/* See description in .h file */
volatile int8_t joystickX;
volatile int8_t joystickY;
volatile uint8_t joystickButtons;

/* Internal variable to keep track of Joystick LED status.
** This can be changed using the set_joystick_leds function.
*/
static uint8_t joystickLEDs = 0;

/* Private functions - only used within this module */
static void delay_microsecond(void);
static uint8_t transfer_byte(uint8_t);

/* See comment in .h file */
void init_joystick(void)
{
	/* Set data direction register appropriately. The following
	** bits are configured to be outputs:
	** SS, MOSI, SCK (i.e. PB0, PB2, PB1)
	*/
	DDRB |= 0x07;

	/* Take the slave select line high */
	PORTB |= 0x01;

	/* Setup SPI Control Register (SPCR) and SPSR
	** We set as follows:
	** - SPE bit = 1 (SPI Enable)
	** - MSTR bit = 1 (Enable Master Mode)
	** - CPOL and CPHA are 0 (SPI mode 0)
	** - SPR1,SPR0 = 01 with SPI2X = 1 (in SPSR register)
	**		 (Clock / 8, i.e. 1MHz)
	*/
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	SPSR  = (1<<SPI2X);

	/* Initialise global variables */
	joystickX = 0;
	joystickY = 0;
	joystickButtons = 0;
}

/* See comment in .h file */
void set_joystick_leds(uint8_t led1, uint8_t led2)
{
	/* We save the values so they can be communicated later.
	** We store these in the format we will transmit them
	** - see the Joystick PMOD reference manual.
	*/
	joystickLEDs = 0;
	if(led1) {
		joystickLEDs |= 0x01;
	}
	if(led2) {
		joystickLEDs |= 0x02;
	}
}

/* See comment in .h file */
void joystick_update(uint8_t led1, uint8_t led2)
{
	uint8_t xLow, xHigh, yLow, yHigh;
	uint8_t i;
	uint8_t cmd;
	uint16_t X, Y;

	/* Take SS (slave select) line (bit 0 of port B) low
	** to initiate communication.
	*/
	PORTB &= 0xFE;

	/* Wait 15 microseconds - as per Joystick PMOD reference manual */
	for(i=0; i<15; i++) {
		delay_microsecond();
	}

	/* Construct command word - see figure 3 in Joystick PMOD
	** reference manual.
	*/
	cmd = 0x80 | joystickLEDs;
	

	/* Transfer each of the 5 bytes - as per the Joystick PMOD
	** reference manual - with a 10microsecond delay between
	** each byte.
	*/
	xLow = transfer_byte(cmd);

	/* Wait for 10 microseconds */
	for(i=0; i<10; i++) {
		delay_microsecond();
	}

	xHigh = transfer_byte(0);

	/* Wait for 10 microseconds */
	for(i=0; i<10; i++) {
		delay_microsecond();
	}

	yLow = transfer_byte(0);

	/* Wait for 10 microseconds */
	for(i=0; i<10; i++) {
		delay_microsecond();
	}

	yHigh = transfer_byte(0);

	/* Wait for 10 microseconds */
	for(i=0; i<10; i++) {
		delay_microsecond();
	}

	joystickButtons = transfer_byte(0);

	/* Take slave select (SS) line high again */
	PORTB |= 0x01;

	/* Reconstruct 16-bit X and Y valuess - these will be in the 
	** range of 0 to 1023.
	*/
	X = (xHigh << 8) | yLow;
	Y = (yHigh << 8) | yLow;

	/* Scale down our position to the range -2 to 2 */
	if(X < 212) {
		joystickX = -2;
	} else if (X < 412) {
		joystickX = -1;
	} else if (X < 612) {
		joystickX = 0;
	} else if (X < 812) {
		joystickX = 1;
	} else {
		joystickX = 2;
	}

	if(Y < 212) {
		joystickY = -2;
	} else if (Y < 412) {
		joystickY = -1;
	} else if (Y < 612) {
		joystickY = 0;
	} else if (Y < 812) {
		joystickY = 1;
	} else {
		joystickY = 2;
	}
}

/****************** INTERNAL FUNCTIONS *********************/

/* delay_microsecond()
**  - when used as part of loop, this function will take
**    1 microsecond of execution time (when including the 
**    loop overhead, and the procedure call and return overhead.
**	  (We assume execution on ATmega64 with 8MHz clock.)
*/
static void delay_microsecond(void) {
	/* We use inline assembly language */
	asm("nop\r\n" "nop\r\n" "nop\r\n" "nop\r\n"::);
}

/* transfer_byte()
**  - write out a byte to the SPI data register and wait until
**    communication is complete, and then return the byte
**    that was shifted in.
*/
static uint8_t transfer_byte(uint8_t byte)
{
	/* Write out byte to SPDR register. This will initiate
	** the transfer. We then wait until the most significant
	** bit of SPSR (SPIF bit) is set - this indicates that
	** the transfer is complete. (The final read of SPSR
	** (after the MSB is 1) followed by a read of SPDR will
	** cause the SPIF bit to be reset to 0. See page 169
	** of the ATmega64 datasheet.)
	*/
	SPDR = byte;
	while((SPSR & 0x80) == 0) {
		;
	}
	return SPDR;
}

