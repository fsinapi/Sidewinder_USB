Sidewinder USB
===============
This project allows an Atmega32U4 to interface with a Microsoft Sidewinder joystick via its legacy Gameport interface, and send this information to a USB host via USB HID.

The project was developed for use with an Arduino Pro Micro board for ease of prototyping. 
A bare Atmega32U4 will require some external electronics.

Sidewinder Interface
---------------
The sidewinder communicates using a simple queried serial protocol over a DBUS connection. The pinout is as follows

    -------------------------------------
    \  +5v  Clk  Trig  Gnd       Data   /
     \        nc   nc   nc     nc      /
       -------------------------------

| DBUS Pin | MCU Pin | Usage |
| -------- | ------- | ----- |
| +5v | +5v | Power |
| Gnd | Gnd | Ground |
| Clk | 3 | Clock signal sent by the joystick. Used to trigger an ISR to read a bit from the Data line |
| Trig | 2 | Set low for 1ms by the MCU to tigger a new data packet from the joystick |
| Data | 4 | Set to logic LOW or HIGH by the joystick in sync with the clock pulses |
| nc | | No connection for this application |

Each data packet from the Sidewinder is 48 bits long. Each bit is read on the rising edge of its corresponding clock pulse.
Credit to [MaZerMind](https://github.com/MaZderMind/SidewinderInterface) for reverse engineering the comm protocol for the Sidewinder.

USB Interface
---------------
The Atmega32U4 will automatically enumerate as a HID Joystick device upon connection to a computer. The HID Report Descriptor is structured
such that the packets received from the Sidewinder can be forwarded to the USB host in the order they are received. The only changes are that:
* The button states must be flipped before sending - the Sidewinder reports buttons with logic level LOW when pressed and HIGH when released
* The HAT nibble must be decremented to follow the standard reporting procedure - the Sidewinder reports neutral as 0, but standard HID 
drivers expect 0 to mean North. 

If not queried by the USB host for report data at regular intervals, the firmware will automatically send data at regular intervals 
(currently around 100-200 Hz, this could probably be decreased to avoid cluttering the bus).
