/*
 * SidewinderUSB.c
 *
 * Firmware to use an Atmega32U4 as a Game port -> USB converter for a Microsoft
 * Sidewinder joystick. The program will read the joystick every 50ms, and if there
 * is new data it will send the updated joystick status. The device will also automatically
 * respond to requests for new data.
 *
 * Created: 3/1/2019 8:57:20 PM
 * Author : Frank
 */ 

#include <avr/io.h>

#include "config.h"
#include "joystick.h"
#include "usb.h"

int main(void)
{
    uint8_t i;
	LED_OFF();
	system_setup();
	for (i = 0; i < 100; i++)
		_delay_ms(10);
    while (1) 
    {
		joystick_read();
		_delay_ms(50);
		if(joystick_is_ready()) 
		{ 
			// Invert buttons
			//joystick_data->m = (0x7F) - joystick_data->m;
			joystick_data->bytes[0] = ~(joystick_data->bytes[0]);
			joystick_data->btn_shift = ~(joystick_data->btn_shift);
			// Fix hat for HID report
			joystick_data->head--;
			if(usb_send_report(joystick_data->bytes) == 0)
			{
				LED_ON();
			} else
			{
				LED_OFF();
			}
		} else
		{
			// Assume that if the joystick isn't ready by 50ms we missed an interrupt and
			// aren't going to get that data packet... give up and make another read.
			LED_OFF();
		}
    }
}

