# Legendary Overlord 2.0

## Deprecated Project Notice
I have not updated or otherwise maintained this project since 2019, my first semester of undergrad. In its current state, the project may be incomplete, buggy, completely broken, or absolutely fabulous. The code hasn’t been relevant in any of my technical projects since then, and I have no intention of ever using this project again. My program design skills, quality standards, language preferences, and other technical abilities have developed a lot since 2019, assisted by my interest in maintaining projects outside of academia. That being said, I have decided to keep my old projects public as I reflect on my own educational journey.

## Description

This project is for an Arduino Mega microcontroller to automate a variety of systems in a household (particularly lights). Beginning as a lamp switched by an old TV remote, my "RoomControl" was my first non-trivial Arduino project. In these early days, I learned a lot very quickly, so I frequently rewrote the project to use my new-found programming skills. Each iteration involved "upgrading" the name until they became comically intense. The best build to date was called "Mythic Overlord," a functional extension of the Legendary Overlord. Mythic attempted to make the code run lighter, but ultimately failed, since the simplicity of the base resulted in greater complexity and redundancy in once simple functions.
Documentation may appear here spontaneously. ~~One day it will be organized, but it will begin as notes for myself to keep track of things.~~

### Subprojects:
* Root: Main project as described above. Requires an LCD screen and Ethernet shield with SD card. Has a fun web interface.
* LEGO2-OLD: Incomplete early prototype. Probably should've been deleted, but I'm a few years late for that.
* LEGO2-Serial-Only: Version of the root that only offers lighting control and can be controlled over Serial or I2C
* LEGO3: Project restart in first semester of undergrad that reflects a design philosophy change. Instead of being able to give each light channel an effect, effects were made separate and could apply to a range of channels. This was a logical step for controlling individually-addressable LED strips where viewing each LED as a separate channel was logicially clunky and required tons of memory.
* LightingControlMaster: Contains some prototype code relating to dimmers.

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

