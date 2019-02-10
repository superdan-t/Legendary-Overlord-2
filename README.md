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

After command processing, the data buffer is automatically cleared to prevent future conflicts. To give a reply, the function will populate the `replyBuffer` and set the `replySize`. If a command returns nothing, the `replyBuffer` is left untouched, but `replySize` is set to 0. If a non-existant command is specified, then the reply will be a negative acknowledge, which is a 2-byte reply consisting of the number 21, which is the negative acknowledge (NAK) in ASCII standards. A 1-byte reply containing NAK is a negative acknowledge intended as the response from a command. Alternatively, another common reply is the number 6, which is positive acknowledge, also known as just acknowledge (ACK).

To avoid explaining it multiple times, I frequently mention "lists," even though the array of bytes doesn't contain an actual List object. A list in an array means a sequence of values that varies in length. Because arrays are simple, the interpreter would have no way to tell how many elements are in the list, unless you specify. And that is exatly what you have to do. When a command asks for a list, begin the list with a count of its elements. There is no need to delimit the list and/or each element. Just make sure that the first value of the list is a count of the number of elements, and then proceed with listing the values.

The first byte of the array is used to determine the command. Each command will use the data in its own way.

0. A ping to check device existance. Returns positive acknowledge.

1. Set a dimmer. Following are two lists, with the first one specifying dimmers and the second one values. If there are more dimmers than values, then the last value will be used multiple times. If there are no values, then they will all be set to 0.
