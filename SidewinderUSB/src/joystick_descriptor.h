// C:\Users\Frank\Documents\Atmel Studio\7.0\SidewinderUSB\SidewinderUSB\joystick_descriptor.h

#ifndef JOYSTICK_DESCRIPTOR_H_
#define JOYSTICK_DESCRIPTOR_H_

const uint8_t PROGMEM ReportDescriptor[99] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x1b, 0x01, 0x00, 0x09, 0x00,  //   USAGE_MINIMUM (Button:Button 1)
    0x2b, 0x09, 0x00, 0x09, 0x00,  //   USAGE_MAXIMUM (Button:Button 9)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x09,                    //   REPORT_COUNT (9)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //   USAGE (X)
    0x09, 0x31,                    //   USAGE (Y)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x03,              //   LOGICAL_MAXIMUM (1023)
    0x75, 0x0a,                    //   REPORT_SIZE (10)
    0x95, 0x02,                    //   REPORT_COUNT (2)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x02,                    //   USAGE_PAGE (Simulation Controls)
    0x09, 0xbb,                    //   USAGE (Throttle)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
    0x75, 0x07,                    //   REPORT_SIZE (7)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x35,                    //   USAGE (RZ)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x3f,                    //   LOGICAL_MAXIMUM (63)
    0x75, 0x06,                    //   REPORT_SIZE (6)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x39,                    //   USAGE (Hat switch)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x07,                    //   LOGICAL_MAXIMUM (7)
    0x35, 0x00,                    //   PHYSICAL_MINIMUM (0)
    0x46, 0x3b, 0x01,              //   PHYSICAL_MAXIMUM (315)
    0x75, 0x04,                    //   REPORT_SIZE (4)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //	REPORT_COUNT (1)
    0x75, 0x02,                    //	REPORT_SIZE (2)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0xc0                           // END_COLLECTION
};

#define REPORT_BYTES 6

#endif /*JOYSTICK_DESCRIPTOR_H_*/