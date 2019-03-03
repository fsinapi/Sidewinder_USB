/*
 * usb.h
 *
 * Created: 3/1/2019 9:18:35 PM
 *  Author: Frank
 */ 


#ifndef USB_H_
#define USB_H_

#define MAX_ENDPOINT		4

#define EP_SIZE(s)	((s) > 32 ? 0x30 :	\
	((s) > 16 ? 0x20 :	\
	((s) > 8  ? 0x10 :	\
	0x00)))
/**************************************************
 * Public Variables
 **************************************************/
uint8_t active_usb_report[8];

/**************************************************
 * Public functions
 **************************************************/

void usb_setup(void);
uint8_t usb_connected(void);
void usb_update_report(uint8_t *data);
int8_t usb_send_report(uint8_t *data);

#endif /* USB_H_ */