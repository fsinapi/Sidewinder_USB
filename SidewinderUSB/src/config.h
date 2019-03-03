/*
 * config.h
 *
 * Created: 2/27/2019 7:09:17 AM
 *  Author: Frank
 */ 

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>

/* define CPU frequency in hz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 16000000L
#endif

// Joystick pins
#define JOYSTICK_INT	PORTD0	// Sidwinder's clock pin - pin 3 
#define JOYSTICK_TRIG	PORTD1	// Trig generator - pin 2
#define JOYSTICK_DATA	PORTD4	// Data input - pin 4

// Green LED
#define LED_PIN			PORTB6

/************************************************************************
  Defines used to set/read pins nice and quick
************************************************************************/
#define SETBIT(reg, bit)   reg |= (1 << bit)
#define CLEARBIT(reg, bit) reg &= ~(1 << bit)
#define READBIT(reg, bit)  ((reg & (1 << bit)) == (1 << bit))

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

#define LED_OFF()			SETBIT(PORTB, LED_PIN)
#define LED_ON()			CLEARBIT(PORTB, LED_PIN)

/************************************************************************
  Public Functions
************************************************************************/
void system_setup(void);

#endif /*__CONFIG_H_*/