/*
 * joystick.c
 *
 * Created: 2/28/2019 8:21:07 PM
 *  Author: Frank
 */ 

#include "joystick.h"
#include "config.h"

/**************************************************
 * Private Defines
 **************************************************/

#define joystick_trig_high() SETBIT(PORTD, JOYSTICK_TRIG)
#define joystick_trig_low()  CLEARBIT(PORTD, JOYSTICK_TRIG)
#define joystick_data_val()  READBIT(PIND, JOYSTICK_DATA)

/**************************************************
 * Private Variables
 **************************************************/

// Struct to keep track of the joystick tracker
// TODO would bit packing make this slower to access?
struct
{
	uint8_t activeSlot;	// 0 or 1; signifies slots in joystate[]
	uint8_t isr_ctr;		// 2^6 = 64 to keep track of isr count
	joystick_data_t *joystick_write_data;
} joystick_state;

static uint8_t joystick_data_valid;

// Keep two joystick_data_t's so we can have one for writing internally and one for reading
joystick_data_t joystate[2] = { {}, {} };

/**************************************************
 * Public Functions
 **************************************************/

void joystick_setup(void)
{
	// Joystick trig is an output
	SETBIT(DDRD, JOYSTICK_TRIG);
	// Default to trig line high
	joystick_trig_high();
	// Joystick clock line and data are inputs
	CLEARBIT(DDRD, JOYSTICK_INT);
	CLEARBIT(DDRD, JOYSTICK_DATA);
	// Enable rising-edge interrupts for INT0
	EICRA |= (1 << ISC01) | (1 << ISC00);
	// but disable INT0 for now
	CLEARBIT(EIMSK, INT0);
	
	// Joystick data is defaulted to all 0's at first
	memset(&joystate[0], 0, sizeof(joystick_data_t));
	memset(&joystate[1], 0, sizeof(joystick_data_t));
	joystick_state.joystick_write_data = &joystate[0];
	joystick_data = &joystate[1];
	joystick_data_valid = JOYSTICK_NOT_READY;
}

/**
 * @TODO it would be nice to take care of timing with a timer ISR instead
 * of having a blocking _delay_ms() call in here
 */
void joystick_read(void)
{
	uint8_t *itr;
	joystick_data_valid = JOYSTICK_NOT_READY;
	// Turn off INT0 interrupts
	CLEARBIT(EIMSK, INT0);
	// Pull trig line low for 2ms
	joystick_trig_low();
	// Reset joystick state
	itr = joystick_state.joystick_write_data->bytes;
	while((itr - joystick_state.joystick_write_data->bytes) < SIDEWINDER_BYTES) 
	{
		*(itr++) = 0;
	}
	joystick_state.isr_ctr = 0;
	_delay_ms(1);
	// Clear INT0 flag
	SETBIT(EIFR, INTF0);
	// Turn INT0 back on
	SETBIT(EIMSK, INT0);
	// Set joystick trig back high again
	joystick_trig_high();
}

uint8_t joystick_is_ready(void)
{
	return joystick_data_valid == JOYSTICK_READY;
}

/**************************************************
 * ISR's
 **************************************************/

/**
 * Interrupt for the joystick's clock line - read data bits for each ISR
 */
ISR(INT0_vect)
{
	if(joystick_data_val())
	{
		uint8_t *byte = &(joystick_state.joystick_write_data->bytes[joystick_state.isr_ctr / 8]);
		// Bitsets have to be done via switchcase to keep up with the interrupt rate.
		switch(joystick_state.isr_ctr % 8)
		{
			case 0: SETBIT(*byte, 0); break;
			case 1: SETBIT(*byte, 1); break;
			case 2: SETBIT(*byte, 2); break;
			case 3: SETBIT(*byte, 3); break;
			case 4: SETBIT(*byte, 4); break;
			case 5: SETBIT(*byte, 5); break;
			case 6: SETBIT(*byte, 6); break;
			case 7: SETBIT(*byte, 7); break;
		}	
	}
	// If we're at SIDEWINDER_BITS, stop checking interrupts
	if(++joystick_state.isr_ctr == SIDEWINDER_BITS)
	{
		CLEARBIT(EIMSK, INT0);
		// Swap write/read pointers
		joystick_state.joystick_write_data = joystick_data;
		joystick_state.activeSlot = 1 ^ joystick_state.activeSlot;
		joystick_data = &joystate[joystick_state.activeSlot];
		// Let calling application know that data has been flipped
		joystick_data_valid = JOYSTICK_READY;
	}
}