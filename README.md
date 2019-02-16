# Legendary Overlord 2.0

This project is for an Arduino Mega microcontroller to manage a variety of systems in a household. Beginning as a lamp switched by an old TV remote, my "RoomControl" project has been reformed, revived, and recreated many times by myself. Some versions in the past weren't even completed to a functional level. The best build to date was called "Mythic Overlord," a functional extension of the Legendary Overlord. Mythic attempted to make the code run lighter, but ultimately failed, since the simplicity of the base resulted in greater complexity and redundancy in once simple functions. As of right now, this repository is *not* functional, as it is under development.

Documentation may appear here spontaneously. One day it will be organized, but it will begin as notes for myself to keep track of things.

## Documentation

### Array Data Interpreter

The function `interpretData(byte permissionLevel)` will read the data in the `dataBuffer` array and determine what to do with it. Some functions do not allow all permission levels. Like many constants in the code, permission levels are defined in the main file. Higher numbers do not have more permission than lower numbers. Rather, each number can be compared to an ID tag used to identify who is calling the function. The permission levels are as follows:
1. PERM_SERIAL: The data was provided by input to the device's serial port.
2. PERM_UDP: Data provided by a client connected via UDP
3. PERM_SCRIPT: Data is being read from a SuperScript file on the SD card.
4. PERM_HUMAN: Data was input by a human user from a computer.

In order to allow simultaneous data input without accidentally merging or ignoring data, each input method uses its own data buffer. The buffer for serial data can be found in the corresponding structure (Ex. `serialInterface[0].data`)

After command processing, the data buffer is automatically cleared to prevent future conflicts. To give a reply, the function will populate the `replyBuffer` and set the `replySize`. If a command returns nothing, the `replyBuffer` is left untouched, but `replySize` is set to 0. If a non-existant command is specified, then the reply will be a negative acknowledge, which is a 2-byte reply consisting of the number 21, which is the negative acknowledge (NAK) in ASCII standards. A 1-byte reply containing NAK is a negative acknowledge intended as the response from a command. Alternatively, another common reply is the number 6, which is positive acknowledge, also known as just acknowledge (ACK). The reply should be used immediately after it is set, as the same buffer is used by other inputs.

To avoid explaining it multiple times, I frequently mention "lists," even though the array of bytes doesn't contain an actual List object. A list in an array means a sequence of values that varies in length. Because arrays are simple, the interpreter would have no way to tell how many elements are in the list, unless you specify. And that is exatly what you have to do. When a command asks for a list, begin the list with a count of its elements. There is no need to delimit the list and/or each element. Just make sure that the first value of the list is a count of the number of elements, and then proceed with listing the values.

The first byte of the array is used to determine the command. Each command will use the data in its own way.

0. A ping to check device existance. Returns positive acknowledge.

1. Set a dimmer. Following are two lists, with the first one specifying dimmers and the second one values. If there are more dimmers than values, then the last value will be used multiple times. If there are no values, then they will all be set to 0.

## Serial Data

The serial interface is primarly used for communication with other devices operating on the same protocol, but is also intended to support MIDI devices and a human language command prompt. Instead of hard-coding the speed/mode of each bus, there is a structure called `SerialInterface` to store the parameters as variables. There is a `SerialInterface` array titled `serialInterface` that actually holds the data for use. The values are stored in the EEPROM for recall at startup. A `SerialInterface` structure has the following components:
* mode (byte)
* busy (bool)
* data (byte array, len 50)

The mode constants are listed at the top of the main file.
0. SERIAL_DISABLED: Disables input and output on the bus. It will be skipped during initialization.
1. SERIAL_SDSCP: Super Dan Sonic Communication Protocol. Runs at a baud of 115,200. Because this (and SDSCP2) are the main methods of communication, there is another section detailing how these messages are formatted. Basically, the input data is forwarded to the array data interpreter (above), but there are more metadata parameters required.
2. SERIAL_SDSCP2: Like SDSCP the First, except it runs at 250,000. Many devices do not support a baud of 250,000, thus why it is not default.
3. SERIAL_MIDI: Send/Receive MIDI data on this bus. It is currently not implemented and few details are available.
4. SERIAL_HUMAN: "Command Prompt" designed for use with the Arduino IDE Serial Monitor. Runs at 115,200.

If any speeds are incompatible with your hardware, they can be found as hard-coded constants in the Serial.h method `serialSpeed`. This is their only reference in the whole project, so changing these will change them everywhere.

The boolean `busy` indicates that the bus is currently busy, preventing the device from trying to read its own data or scramble what's being input by trying to output.

The data array is used differently by the different modes.

By default, the project supports includes 4 serial busses. This is because the Arduino Mega, the intended host device, has a total of 4 busses.

### Using SDSCP
The Super Dan Sonic Communication Protocol is not well known (and it never will be), but that doesn't mean it isn't useful. A SDSCP message consists of two portions: metadata and data. All of the data is directly forwarded to the array data interpreter. Metadata varies based on message type, but there is one constant... Data size. This is the counted size of the data portion and DOES NOT include metadata (as metadata size can be determined by natural means). Because it is needed for every type of message, it is the first byte. The next is the message type:
5. ENQUIRY: Signals that the 
