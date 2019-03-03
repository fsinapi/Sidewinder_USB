/*
 * config.c
 *
 * Created: 2/27/2019 7:16:12 AM
 *  Author: Frank
 */ 

#include "config.h"
#include "joystick.h"
#include "usb.h"

#define UBRR_VAL (F_CPU / (16 * (uint16_t)UART_BAUD ) - 1)

void system_setup(void)
{
	// Set up LED
	SETBIT(DDRB, LED_PIN);
	LED_OFF();
	joystick_setup();
	usb_setup();
	while(!usb_connected());
	sei();
}