/*
 * joystick.h
 *
 * Created: 2/28/2019 8:20:02 PM
 *  Author: Frank
 */ 


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include "config.h"

#define JOYSTICK_NOT_READY 0
#define JOYSTICK_READY 1

#define SIDEWINDER_BYTES 6
#define SIDEWINDER_BITS  48

// Joystick data structure stolen from https://github.com/MaZderMind/SidewinderInterface
// Huge shouts to MaZderMind for his work reverse engineering the Sidwinder comm protocol
// data structure as sent from my sidewinder device, 48 bits total
// this datatype is accessable via two ways: an integer-array and a struct
// the former is used to used to manipulate the data at a bit-level
// while the latter is used to access individual pieces of data
typedef union
{
	// the struct consisting of the various data-fields sent by the sidewinder device
	struct
	{
		unsigned int btn_fire:1;      // bit  1

		unsigned int btn_top:1;       // bit  2
		unsigned int btn_top_up:1;    // bit  3
		unsigned int btn_top_down:1;  // bit  4

		unsigned int btn_a:1;         // bit  5
		unsigned int btn_b:1;         // bit  6
		unsigned int btn_c:1;         // bit  7
		unsigned int btn_d:1;         // bit  8
		
		unsigned int btn_shift:1;     // bit  9

		unsigned int x:10;            // bits 10-19
		unsigned int y:10;            // bits 20-29
		unsigned int m:7;             // bits 30-36 (throttle)
		unsigned int r:6;             // bits 37-42 (rotation)

		unsigned int head:4;          // bits 43-46

		unsigned int reserved:1;      // bit 47
		unsigned int parity:1;        // bit 48
	};

	// the ints are used to access the struct-data at bit-level
	//uint8_t bytes[6];
	uint8_t bytes[SIDEWINDER_BYTES];
} joystick_data_t;

joystick_data_t * joystick_data;

void joystick_setup(void);
void joystick_read(void);
uint8_t joystick_is_ready(void);

#endif /* JOYSTICK_H_ */