PacketSerial
============

[![Build Status](https://travis-ci.org/bakercp/PacketSerial.svg?branch=master)](https://travis-ci.org/bakercp/PacketSerial)

Description
-----------

An Arduino Library that facilitates packet-based serial communication using COBS or SLIP encoding.

Features
--------

_PacketSerial_ is an small, efficient, library that allows [Arduinos](http://www.arduino.cc/) to send and receive serial data packets (with COBS, SLIP or a user-defined encoding) that include bytes of any value (0 - 255). A _packet_ is simply an array of bytes.

Background
----------

_Why do I need this?_ you may ask. The truth is that you may not need it if you are converting your values to ASCII strings and separating them with a known character (like a carriage return `\r` and a line feed `\n`) before sending them. This is what happens if you call and `Serial.println();`. For instance, if you just want to send a byte with the value of 255 and follow it with a new line character (i.e. `Serial.println(255);`) the Arduino automatically converts the number to the equivalent printable ASCII characters, sending 5 bytes total. As a result the receiver won't just receive a byte for the number and two bytes for the carriage return and new line character. Instead it will receive a stream of 5 bytes:

```
50 // ASCII '2'
53 // ASCII '5'
53 // ASCII '5'
13 // ASCII '\r'
10 // ASCII '\n'
```

The receiver must then collect the 3 ASCII characters { '2', '5', '5' }, combine them and convert them back into a single byte with a value of `255`. This process can get complicated when the user wants to send large quantities of structured data between the Arduino and a receiver.

One way to send a _packet_ of data without this library is to send each byte separated by a comma or space and terminate the sequence with a new line character. Thus, to send the value `255` and the value `10`, one might call:

```c++
Serial.print(255);
Serial.print(',');
Serial.print(10);
Serial.print('\n');
```

The receiver will actually see a stream of 7 bytes:

```
50 // ASCII '2'
53 // ASCII '5'
53 // ASCII '5'
44 // ASCII ','
49 // ASCII '1'
48 // ASCII '0'
10 // ASCII '\n'
```

In this case, the receiver must then collect the ASCII characters, combine them, skip the delimiter (the comma in this case) and then process the packet when a new line is encountered. While effective, this method doesn't scale particularly well. Bytes with values larger than 9 are encoded as 2 bytes and bytes with values larger than 99 are encoded as 3 bytes, etc. If the user would like to send the number 4,294,967,295 (the maximum value of a 4 byte `unsigned long`), it would be encoded as 10 bytes. This means that there is an overhead of 6 extra bytes to transmit a 4 byte `unsigned long`.

An alternative to ASCII encoding is to write the bytes directly to using the `Serial.write()` methods. These methods do not convert the byte values to ASCII. So if the user wants to send a single byte with the value of `255` and follow it with a new line character:

```c++
Serial.write(255);
Serial.write('\n');
```

the receiver will see a stream of 2 bytes:

```
255 // The value transmitted.
10  // The new line character (\n).
```

This is much more compact but can create problems when the user wants to send a _packet_ of data. If the user wants to send a packet consisting of two values such as 255 and 10, we run into problems if we also use the new line ('\n' ASCII 10) character as a packet boundary. This essentially means that the receiver will incorrectly think that a new packet is beginning when it receives the _value_ of 10. Thus, to use this more compact form of sending bytes while reserving one value for a packet boundary marker. Several unambiguous packet boundary marking encodings exist, but one with a small predictable overhead is called [Consistent Overhead Byte Stuffing](http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing). For a raw packet of length `SIZE`, the maximum encoded buffer size will only be `SIZE + SIZE / 254 + 1`. This is significantly less than ASCII encoding and the encoding / decoding algorithm is simple and fast. In its default mode, the COBS encoding process simply removes all _zeros_ from the packet, allowing the sender and receiver to use the value of _zero_ as a packet boundary marker.
Another encoding available in `PacketSerial` is [Serial Line Internet Protocol](https://en.wikipedia.org/wiki/Serial_Line_Internet_Protocol) which is often used to send OSC over serial or TCP connections. To use SLIP encoding instead of COBS, use `SLIPPacketSerial` instead of `PacketSerial`. You can find an openFrameworks example of sending OSC data over serial in the [ofxSerial](https://github.com/bakercp/ofxSerial) repository.

Use
---

`PacketSerial` class wraps the Arduino `Stream` class to automatically encode and decode byte packets that are sent and received. Typically serial communication uses the default `Serial` object, which implements the `Stream` class. In most cases, `PacketSerial` should be given exclusive access to the serial `Stream` (e.g. for a default setup using `Serial`, users should avoid calling functions like `Serial.print()`, `Serial.write()`, etc directly). Data should be sent via the `send(const uint8_t* buffer, size_t size) const` method and received in a `PacketSerial` callback function (see below).

Setup
-----

#### Basic

To use the default `Serial` object and the default communication settings (usually `SERIAL_8N1`), set up `PacketSerial` like this:

```c++
PacketSerial myPacketSerial;

void setup()
{
    myPacketSerial.begin(9600);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

#### Advanced

For a non-default Serial connection, a class implementing the `Stream` interface should be configured and then set for the `PacketSerial` instance.

##### Using A Non-Standard Serial Configuration

```c++
PacketSerial myPacketSerial;

void setup()
{
    Serial.begin(300, SERIAL_7N1);
    myPacketSerial.setStream(&Serial);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

##### Using Secondary Serial Ports (e.g. Serial1, Serial2, etc)

```c++
PacketSerial myPacketSerial;

void setup()
{
    Serial1.begin(9600);
    myPacketSerial.setStream(&Serial1);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

##### Using SoftwareSerial

```c++
PacketSerial myPacketSerial;
SoftwareSerial mySoftwareSerial(10, 11);

void setup()
{
    mySoftwareSerial.begin(38400);
    myPacketSerial.setStream(&mySoftwareSerial);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

##### Other Streams

Any class that correctly implements the `Stream` interface should work, which includes some network communication objects.

### Loop

In order to processing incoming serial packets, the user must call the `update()` method at the end of the `loop()` method.

```c++
void loop()
{
    // Your program here.


    // Call update to receive, decode and process incoming packets.
    myPacketSerial.update();
}

```

### Receiving Packets

All packets are received via handler functions. A typical handler function would be registered in the `void setup()` function like:

```c++
PacketSerial myPacketSerial;

void setup()
{
    myPacketSerial.begin(9600);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

The `onPacketReceived` function can take two forms. The simplest looks like this:


```c++
void onPacketReceived(const uint8_t* buffer, size_t size)
{
    // Process your decoded incoming packet here.
}
```

For more advanced programs with multiple PacketSerial instances and a shared handler, it may be useful to know which PacketSerial instance received the packet. In this case you could define a callback like this:

```c++
void onPacketReceived(const void* sender, const uint8_t* buffer, size_t size)
{
    if (sender == &myPacketSerial)
    {
        // Do something with the packet from myPacketSerial.
    }
    else if (sender == &myOtherPacketSerial)
    {
        // Do something with the packet from myOtherPacketSerial.
    }
}

```

Finally, it is also possible to set arbitrary packet handlers that point to member functions of a given class instance using lambda functions. For example:

```c++
// Instances of this class can receive data packets when registered.
class MyClass
{
public:
    void processPacketFromSender(const PacketSerial& sender, const uint8_t* buffer, size_t size)
    {
        // Just send the buffer back to the sender.
        sender.send(buffer, size);
    }
};


MyClass myClassInstance;
PacketSerial myPacketSerial;

void setup()
{
    myPacketSerial.begin(115200);
    myPacketSerial.setPacketHandler([](const uint8_t* buffer, size_t size) {
         myClassInstance.processPacketFromSender(myPacketSerial, buffer, size);
    });
}
```

### Sending Packets

To send packets call the `send()` method. The send method will take a packet (an array of bytes), encode it, transmit it and send the packet boundary marker. To send the values `255` and `10`, one might do the following:

```c++

// Make an array.
uint8_t myPacket[2] = { 255, 10 };

// Send the array.
myPacketSerial.send(myPacket, 2);
```

### Multiple Streams
On boards with multiple serial ports, this strategy can also be used to set up two Serial streams, one for packets and one for debug ASCII (see [this discussion](https://github.com/bakercp/PacketSerial/issues/10) for more).

Examples
--------

See the included examples for further usage options.

Tested Compatible Libraries
---------------------------

-   openFrameworks (C/C++ on macOS, Windows, Linux, Android, etc).
    -   [ofxSerial](https://github.com/bakercp/ofxSerial) addon.
        -   See the `ofx::IO::PacketSerial` object which is directly compatible with this library.
-   Python
    -   [PySerial](https://pythonhosted.org/pyserial/index.html)
        -   [COBS](https://pythonhosted.org/cobs/) (see [this discussion](https://github.com/bakercp/PacketSerial/issues/10) for more).
        -   [SLIP](https://pypi.python.org/pypi/sliplib/0.0.1)


_Any library that correctly implements a COBS or SLIP encoding scheme should be compatible._

Changelog
---------
See [CHANGELOG.md](CHANGELOG.md).


License
-------
See [LICENSE.md](LICENSE.md).
