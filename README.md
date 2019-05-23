# Legendary Overlord 2.0

This project is for an Arduino Mega microcontroller to manage a variety of systems in a household. Beginning as a lamp switched by an old TV remote, my "RoomControl" project has been reformed, revived, and recreated many times by myself. Some versions in the past weren't even completed to a functional level. The best build to date was called "Mythic Overlord," a functional extension of the Legendary Overlord. Mythic attempted to make the code run lighter, but ultimately failed, since the simplicity of the base resulted in greater complexity and redundancy in once simple functions. ~~As of right now, this repository is *not* functional, as it is under development.~~ This is actually coming along pretty well and there are multiple functional pieces, including an I2C slave to expand your reach with less wire and, of course, the master device.

Documentation may appear here spontaneously. One day it will be organized, but it will begin as notes for myself to keep track of things.

## Documentation

### Data Input / Output

Info can be sent and received by the device in these ways:
* UART Serial on Serial 0 (Supported by all)
* I2C (LEGO2 main can only send [master], LEGO2-Serial-Only can receive and reply [slave])
* UDP (LEGO2 main)

#### UART

Has full permission to perform all commands. There are no end-of-transmission markers. Instead, a transmission's length must be the first byte. Note that the transmission's length does NOT include the length-indicating byte. Example: sending "1 2 3," a message of size 3, would be sent as "3 1 2 3."
Replies also follow this format. Note that UART has no default reply, so there will not be one unless the command sends specific data back.

#### I2C

Has full permission to perform all commands. Slave devices can be sent data that is forwarded directly to the data interpreter. No metadata bytes are needed, unlike serial. Slave addresses are stored in the EEPROM. All slave devices will send back data when it is requested.
Master devices can not receive commands via I2C, but they can issue them. If they expect a reply, `Wire.requestFrom(addr, amount)` must be used, and the amount to expect back MUST be accurate. If this is unsure in some circumstances, request 1 byte, program the slave to reply with the length, and then request the length that the slave returned.

#### UDP

Lacks some permissions to edit memory values. Directly forwarded to data interpreter with no required metadata. Will always reply with a positive acknowledge unless something else is provided by a command.

