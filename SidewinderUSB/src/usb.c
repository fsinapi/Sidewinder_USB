/*
 * usb.c
 *
 * Much of this is basically transcribed from the Teensy2.0 usb hid example
 * code.
 *
 * Created: 3/1/2019 9:18:44 PM
 *  Author: Frank
 */ 

#include "config.h"
#include "usb.h"

#include "joystick_descriptor.h"

/**************************************************
 * Defines
 **************************************************/

// Endpoint types
#define EP_TYPE_CONTROL			0x00
#define EP_TYPE_BULK_IN			0x81
#define EP_TYPE_BULK_OUT		0x80
#define EP_TYPE_INTERRUPT_IN	0xC1
#define EP_TYPE_INTERRUPT_OUT	0xC0
#define EP_TYPE_ISOCHRONOUS_IN	0x41
#define EP_TYPE_ISOCHRONOUS_OUT	0x40
#define EP_SINGLE_BUFFER		0x02
#define EP_DOUBLE_BUFFER		0x06

// standard control endpoint request types
#define GET_STATUS              0
#define CLEAR_FEATURE           1
#define SET_FEATURE             3
#define SET_ADDRESS             5
#define GET_DESCRIPTOR          6
#define GET_CONFIGURATION       8
#define SET_CONFIGURATION       9
#define GET_INTERFACE           10
#define SET_INTERFACE           11

// HID (human interface device)
#define HID_GET_REPORT          1
#define HID_GET_IDLE            2
#define HID_GET_PROTOCOL        3
#define HID_SET_REPORT          9
#define HID_SET_IDLE            10
#define HID_SET_PROTOCOL        11

// Endpoint configuration
#define ENDPOINT0_SIZE			32

#define set_endpoint(point) (UENUM = point);

// Misc macros to wait for ready and send/receive packets
#define usb_wait_in_ready()		while(!READBIT(UEINTX, TXINI))
#define usb_send_in()			CLEARBIT(UEINTX,TXINI)
#define usb_wait_receive_out()	while(!READBIT(UEINTX,RXOUTI))
#define usb_ack_out()			CLEARBIT(UEINTX,RXOUTI)

// Device config
#define VENDOR_ID		0x16C0
#define PRODUCT_ID		0x0481

#define INTERFACE_NUM	0
#define IF_CLASS		0x03
#define IF_PROTOCOL		0
#define IF_SUBCLASS		0

#define ENDPOINT_NUM           3

#define DATA_EP_BUF_SIZE     64
#define AVR_DATA_BUF_CODE    0x30   // Atmega code for 64 bytes

#define STR_VENDOR      L"Frank Sinapi"
//#define STR_PRODUCT     L"Sidewinder"
#define STR_PRODUCT     L"SW Joystick"

#define HID_TX_BUFFER EP_DOUBLE_BUFFER
#define HID_RX_BUFFER EP_DOUBLE_BUFFER

/*
static const uint8_t PROGMEM endpoint_config_table[] = {
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(DATA_EP_BUF_SIZE) | HID_TX_BUFFER,
	1, EP_TYPE_INTERRUPT_OUT, EP_SIZE(DATA_EP_BUF_SIZE) | HID_RX_BUFFER,
	0,
	0
};
*/
static const uint8_t PROGMEM endpoint_config_table[] = 
{
	  0,
	  0,
	  1, EP_TYPE_INTERRUPT_IN, AVR_DATA_BUF_CODE | EP_DOUBLE_BUFFER,
	  0
};

/**************************************************
 * Typedefs
 **************************************************/
typedef union  
{
	struct 
	{
		uint8_t  bmRequestType;
		uint8_t  bRequest;
		uint16_t wValue;
		uint16_t wIndex;
		uint16_t wLength;
	};
	uint8_t bytes[8];
}control_packet_t;

/**************************************************
 * Device Descriptor Data
 **************************************************/

static const uint8_t PROGMEM device_descriptor[] = {
	18,					// bLength
	1,					// bDescriptorType
	0x00, 0x02,			// bcdUSB
	0,					// bDeviceClass
	0,					// bDeviceSubClass
	0,					// bDeviceProtocol
	ENDPOINT0_SIZE,		// bMaxPacketSize0
	LSB(VENDOR_ID), MSB(VENDOR_ID),		// idVendor
	LSB(PRODUCT_ID), MSB(PRODUCT_ID),	// idProduct
	0x00, 0x01,			// bcdDevice
	1,					// iManufacturer
	2,					// iProduct
	0,					// iSerialNumber
	1					// bNumConfigurations
};


// Size of HID_DESC_SIZE
#define HID_DESC_SIZE sizeof(ReportDescriptor)

#define CONFIG_DESC_SIZE     (9+9+9+7)
#define HID_DESC_OFFSET      (9+9)
const static uint8_t PROGMEM config_desc[CONFIG_DESC_SIZE] =
{
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9,                              // bLength;
	2,                              // bDescriptorType;
	LSB(CONFIG_DESC_SIZE),          // wTotalLength
	MSB(CONFIG_DESC_SIZE),
	1,                              // bNumInterfaces
	1,                              // bConfigurationValue
	0,                              // iConfiguration
	0x80,                           // bmAttributes (bus pwr)
	250,                            // bMaxPower (250 x 2 = 500 mA)

	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                              // bLength
	4,                              // bDescriptorType
	INTERFACE_NUM,                  // bInterfaceNumber
	0,                              // bAlternateSetting
	1,                              // bNumEndpoints
	IF_CLASS,                       // bInterfaceClass
	IF_SUBCLASS,                    // bInterfaceSubClass
	IF_PROTOCOL,                    // bInterfaceProtocol
	0,                              // iInterface

	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,                              // bLength
	0x21,                           // bDescriptorType
	0x11, 0x01,                     // bcdHID
	0,                              // bCountryCode
	1,                              // bNumDescriptors
	0x22,                           // bDescriptorType
	HID_DESC_SIZE,                  // wDescriptorLength
	0,

	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                              // bLength
	5,                              // bDescriptorType
	ENDPOINT_NUM | 0x80,			// bEndpointAddress
	0x03,                           // bmAttributes (0x03=intr)
	DATA_EP_BUF_SIZE, 0,            // wMaxPacketSize
	0x20                            // bInterval (32 ms)
};

typedef struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	int16_t wString[];
} string_desc_t;

const static string_desc_t PROGMEM string0 =
{
	4,
	3,
	{0x0409}
};

const static string_desc_t PROGMEM string1 =
{
	sizeof(STR_VENDOR),
	3,
	STR_VENDOR
};

const static string_desc_t PROGMEM string2 =
{
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};

// This table defines which descriptor data is sent for each specific
// request from the host (specified in wValue and wIndex of the request).
typedef struct
{
	uint16_t       wValue;
	uint16_t       wIndex;
	const uint8_t *addr;
	uint8_t        length;
} desc_list_t;

const static desc_list_t PROGMEM desc_list[] =
{
	{ 0x0100, 0x0000, device_descriptor, sizeof(device_descriptor) },
	{ 0x0200, 0x0000, config_desc, sizeof(config_desc) },
	{ 0x2200, INTERFACE_NUM, ReportDescriptor, HID_DESC_SIZE },
	{ 0x2100, INTERFACE_NUM, config_desc + HID_DESC_OFFSET, 9 },
	{ 0x0300, 0x0000, (const uint8_t *) &string0, 4 },
	{ 0x0301, 0x0409, (const uint8_t *) &string1, sizeof(STR_VENDOR) },
	{ 0x0302, 0x0409, (const uint8_t *) &string2, sizeof(STR_PRODUCT) }
};

#define NUM_DESC_LIST (sizeof(desc_list)/sizeof(desc_list_t))

/**************************************************
 * Variables
 **************************************************/

// zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_config = 0;

// the idle configuration, how often we send the report to the
// host (ms * 4) even when it hasn't changed
static uint8_t idle_config = 125;

// count until idle timeout
static uint8_t idle_count = 0;

// protocol setting from the host.  We use exactly the same report
// either way, so this variable only stores the setting since we
// are required to be able to report which setting is in use.
static uint8_t hid_protocol = 1;

/**************************************************
 * Private Function Prototypes
 **************************************************/
void copy_report(void);

/**************************************************
 * Public functions
 **************************************************/

/**
 * USB set up steps:
 * - Power on USB pads regulator
 * - Configure PLL
 * - Enable PLL
 * - Check PLL lock
 * - Enable USB interface
 * - Configure USB interface (Speed, endpoints)
 * - Wait for VBUS information connection
 * - Attach USB device
 */
void usb_setup(void)
{
	// Interrupts OFF until we disable them all
	cli();
	// Disable USB interrupts for now
	CLEARBIT(USBCON, VBUSTE);
	// Disable all USB device interrupts
	//   (clear all non-reserved bits)
	UDIEN &= 0x82;
	// Disable USB endpoint interrupts
	UEIENX &= 0x20;
		
	// Power on USB pads regulator
	SETBIT(UHWCON, UVREGE);
	
	// Configure PLL
	SETBIT(PLLCSR, PINDIV);		// 16MHz
	SETBIT(PLLCSR, PLLE);		// Enable PLL
	// Wait for PLL Lock
	while(!READBIT(PLLCSR, PLOCK));
	// Enable USB interface
	SETBIT(USBCON, USBE);		// Enable USB interface
	SETBIT(USBCON, OTGPADE);	// Enable VBUS pad
	CLEARBIT(USBCON, FRZCLK);	// Keep clock inputs enabled
	UDCON &= 0xF0;				// Enable high-speed mode
	// Enable
		// End of Reset Interrupt
		// Start of Frame Interrupt
	UDIEN = (1 << EORSTE) | (1 << SOFE);
	// Interrupts can be allowed again
	sei();
}

uint8_t usb_connected(void)
{
	return usb_config;
}

void usb_update_report(uint8_t *data)
{
	memcpy(active_usb_report, data, REPORT_BYTES);
}

int8_t usb_send_report(uint8_t *data)
{
	uint8_t intr_state, timeout;
	if(data != NULL)
	{
		usb_update_report(data);
	}
	if(!usb_config)
	{
		return -1;
	}
	timeout = UDFNUML + 50;
	do 
	{
		// Check if we can send
		intr_state = SREG;
		cli();
		set_endpoint(ENDPOINT_NUM);
		
		// Read to tx
		if(READBIT(UEINTX, RWAL))
		{
			break;
		}
		SREG = intr_state;
		
		if(!usb_config)
		{
			return -1;
		}
		if(UDFNUML >= timeout)
		{
			return -1;
		}
	} while (1);
	
	copy_report();
	
	UEINTX = 0x3A;
	idle_count = 0;
	SREG = intr_state;
	return 0;
}

/**************************************************
 * Private Functions
 **************************************************/

/**
 * Copies the active HID report to the USB buffer
 */
void copy_report(void)
{
	uint8_t i;
	// Copy 8 bytes
	for(i = 0; i < REPORT_BYTES; ++i)
	{
		UEDATX = active_usb_report[i];
	}
}

/**************************************************
 * ISRs
 **************************************************/

// Catch USB General Vector interrupts
ISR(USB_GEN_vect)
{
	static uint8_t div4 = 0;
	if(READBIT(UDINT, EORSTI))	// End of Reset interrupt flag
	{
		// Switch to endpoint 0
		set_endpoint(0);
		SETBIT(UECONX, EPEN);	// Enable endpoint 0
		UECFG0X = EP_TYPE_CONTROL;
		// Configure to 32 Byte buffer (0x20) and one-bank buffer
		UECFG1X = (1 << EPSIZE1) | (1 << ALLOC);
		UEINTX = 0;				// Clear interrupts reg
		SETBIT(UEIENX, RXSTPE);	// Enable endpoint interrupt when RXSTPI sent
		usb_config = 0;
	}
	if(READBIT(UDINT, SOFI) && usb_config)	// Timeout stuff
	{
		if(idle_config && (++div4 & 3) == 0)
		{
			set_endpoint(ENDPOINT_NUM);
			if(READBIT(UEINTX, RWAL))
			{
				++idle_count;
				if(idle_count == idle_config)
				{
					idle_count = 0;
					copy_report();
					UEINTX = 0x3A;
				}
			}
		}
	}
	// Clear interrupt flags
	UDINT = 0;
}

// Interrupts for endpoint 0 (control)
ISR(USB_COM_vect)
{
	uint8_t interrupt_bits;
	const uint8_t *list_itr;
	const uint8_t *cfg_itr;
	control_packet_t rcvd;
	uint8_t i, len, n, en;
	uint16_t desc_val;
	const uint8_t *desc_addr;
	uint8_t	desc_length;
	
	set_endpoint(0);
	interrupt_bits = UEINTX;	// Copy for later use
	if(READBIT(interrupt_bits,RXSTPI))
	{
		// Read data from UEDATX
		rcvd.bmRequestType = UEDATX;
		rcvd.bRequest = UEDATX;
		rcvd.wValue = UEDATX;
		rcvd.wValue |= (UEDATX  <<  8);
		rcvd.wIndex = UEDATX;
		rcvd.wIndex |= (UEDATX  <<  8);
		rcvd.wLength = UEDATX;
		rcvd.wLength |= (UEDATX  <<  8);
		UEINTX = ~((1 << RXSTPI) | (1 << RXOUTI) | (1 << TXINI));
		if(rcvd.bRequest == GET_DESCRIPTOR)
		{
			list_itr = (const uint8_t *)desc_list;
			for(i=0; ; ++i)
			{
				if(i > NUM_DESC_LIST)
				{
					UECONX = (1 << STALLRQ) | (1 << EPEN); // Stall
					return;
				}
				desc_val = pgm_read_word(list_itr);
				if(desc_val != rcvd.wValue)
				{
					list_itr += sizeof(desc_list_t);
					continue;
				}
				list_itr += 2;
				desc_val = pgm_read_word(list_itr);
				if(desc_val != rcvd.wIndex)
				{
					list_itr += sizeof(desc_list_t) - 2;
					continue;
				}
				list_itr += 2;
				desc_addr = (const uint8_t *) pgm_read_word(list_itr);
				list_itr += 2;
				desc_length = pgm_read_byte(list_itr);
				break;
			}
			len = (rcvd.wLength < 256) ? rcvd.wLength : 255;
			if(len > desc_length) len = desc_length;
			do 
			{
				// Wait for host to be ready for IN packet
				do 
				{
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				
				if(READBIT(i,RXOUTI)) return; // Abort
				// send the IN packet
				n = (len < ENDPOINT0_SIZE) ? len : ENDPOINT0_SIZE;
				for(i = n; i; i--)
				{
					UEDATX = pgm_read_byte(desc_addr++);
				}
				len -= n;
				usb_send_in();
			} while (len || n == ENDPOINT0_SIZE);
			return;
		}
		
		if(rcvd.bRequest == SET_ADDRESS)
		{
			usb_send_in();
			usb_wait_in_ready();
			UDADDR = rcvd.wValue | (1 << ADDEN);
			return;
		}
		
		if(rcvd.bRequest == SET_CONFIGURATION && rcvd.bmRequestType == 0)
		{
			usb_config = rcvd.wValue;
			usb_send_in();
			cfg_itr = endpoint_config_table;
			for(i = 1; i < 5; i++)
			{
				set_endpoint(i);
				en = pgm_read_byte(cfg_itr++);
				UECONX = en;
				if(en)
				{
					UECFG0X = pgm_read_byte(cfg_itr++);
					UECFG1X = pgm_read_byte(cfg_itr++);
				}
			}
			UERST = 0x1E;
			UERST = 0;
			return;
		}
		
		if(rcvd.bRequest == GET_CONFIGURATION && rcvd.bmRequestType == 0x80)
		{
			usb_wait_in_ready();
			UEDATX = usb_config;
			usb_send_in();
			return;
		}
		
		if(rcvd.bRequest == GET_STATUS)
		{
			usb_wait_in_ready();
			i = 0;
			if(rcvd.bmRequestType == 0x82)
			{
				set_endpoint(rcvd.wIndex);
				if(READBIT(UECONX, STALLRQ))
				{
					i = 1;
				}
				set_endpoint(0);
			}
			UEDATX = i;
			UEDATX = 0;
			usb_send_in();
			return;
		}
		
		if( (rcvd.bRequest == CLEAR_FEATURE || rcvd.bRequest == SET_FEATURE) 
			&& (rcvd.bmRequestType == 0x02) && (rcvd.wValue == 0) )
		{
			i = rcvd.wIndex & 0x7F;
			if( i > 0 && i <= MAX_ENDPOINT)
			{
				usb_send_in();
				set_endpoint(i);
				if(rcvd.bRequest == SET_FEATURE)
				{
					UECONX = (1 << STALLRQ) | (1 << EPEN);
				} else
				{
					UECONX = (1 << STALLRQC) | (1 << RSTDT) | (1 << EPEN);
					UERST = (1 << i);
					UERST = 0;
				}
				return;
			}		
		}
		
		if(rcvd.wIndex == INTERFACE_NUM)
		{
			if(rcvd.bmRequestType == 0xA1)
			{
				if(rcvd.bRequest == HID_GET_REPORT)
				{
					usb_wait_in_ready();
					copy_report();
					usb_send_in();
					return;
				}
				if (rcvd.bRequest == HID_GET_IDLE)
				{
					usb_wait_in_ready();
					UEDATX = idle_config;
					usb_send_in();
					return;
				}

				if (rcvd.bRequest == HID_GET_PROTOCOL)
				{
					usb_wait_in_ready();
					UEDATX = hid_protocol;
					usb_send_in();
					return;
				}
			}
			if(rcvd.bmRequestType == 0x21)
			{
				if(rcvd.bRequest == HID_SET_REPORT)
				{
					usb_wait_receive_out();
					usb_ack_out();
					usb_send_in();
					return;
				}
				if (rcvd.bRequest == HID_SET_IDLE)
				{
					idle_config = (rcvd.wValue >> 8);
					idle_count = 0;
					usb_send_in();
					return;
				}
				if (rcvd.bRequest == HID_SET_PROTOCOL)
				{
					hid_protocol = rcvd.wValue;
					usb_send_in();
					return;
				}
			}
		}
	}
	// Request a STALL request and keep endpoint 0 enabled
	UECONX = (1 << STALLRQ) | (1 << EPEN);
}