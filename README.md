PacketSerial
============

_PacketSerial_ is an small, efficient, library that allows [Arduinos](http://www.arduino.cc/) to send and receive serial data packets (with COBS or SLIP encoding) that include bytes with any value (0-255).  A _packet_ is simply an array of bytes.

_"Why do I need this?"_ you may ask.  The truth is that you may not need it if you are converting your values to ASCII strings and separating them with a known character (like a new line `\n`) before sending them.   This is what happens if you call `Serial.print()` or `Serial.println()`.  For instance, if you just want to send a byte with the value of 255 and follow it with a new line character (i.e. `Serial.println(255)`) the Arduino automatically converts the number to the equivalent printable ASCII characters, sending 4 bytes total.  As a result the receiver won't just receive a byte for the number and a byte for the new line character.  Instead it will receive a stream of 4 bytes:

```
50 // ASCII 2
53 // ASCII 5
53 // ASCII 5
10 // Serial.println() appends a new line \n character.
```

The receiver must then collect the 3 ASCII characters (2, 5, 5), combine them and convert them back into a single byte with a value of 255.  This process can get complicated when the user wants to send large quantities of structured data between the Arduino and a receiver.

One way to send a _packet_ of data without this library is to send each byte separated by a comma or space and terminate the sequence with a new line character.  Thus, to send the value 255 and the value 10, one might call:

```
Serial.print(255);
Serial.print(',');
Serial.print(10);
Serial.print('\n');
```

The receiver will actually see a stream of 8 bytes:

```
50 // ASCII 2
53 // ASCII 5
53 // ASCII 5
44 // ASCII ,
49 // ASCII 1
48 // ASCII 0
10 // ASCII \n
```

In this case, the receiver must then collect the ASCII characters, combine them, skip the delimiter (the comma in this case) and then process the packet when a new line is encountered.  While effective, this method doesn't scale well.  Bytes with values larger than 9 require are encoded as 2 bytes and bytes with values larger than 99 are encoded as 3 bytes.  If the user would like to send the number 4,294,967,295 (the maximum value of a 4 byte `unsigned long`), it would be encoded as 10 bytes.  This means that there is an overhead of 6 extra bytes to transmit a 4 byte `unsigned long`.

An alternative to ASCII encoding is to write the bytes directly to using the `Serial.write()` methods.  These methods do not convert the byte values to ASCII.  So if the user wants to send a single byte with the value of 255 and follow it with a new line character (i.e. `Serial.write(255); Serial.write('\n');`), the receiver will see a stream of 2 bytes:

```
255 // The value transmitted.
10  // The new line character (\n).
```

This is much more compact but can create problems when the user wants to send a _packet_ of data.  If the user wants to send a packet consisting of two values such as 255 and 10, we run into problems if we also use the new line ('\n' ASCII 10) character as a packet boundary.  This essentially means that the receiver will incorrectly think that a new packet is beginning when it receives the _value_ of 10.  Thus, to use this more compact form of sending bytes while reserving one value for a packet boundary marker.  Several unambiguous packet boundary marking encodings exist, but one with a small predictable overhead is called [Consistent Overhead Byte Stuffing](http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing).  For a raw packet of length `SIZE`, the maximum encoded buffer size will only be `SIZE + SIZE / 254 + 1`.  This is significantly less than ASCII encoding and the encoding / decoding algorithm is simple and fast.  In its default mode, the COBS encoding process simply removes all _zeros_ from the packet, allowing the sender and receiver to use the value of _zero_ as a packet boundary marker.
Another coding available in PacketSerial is [Serial Line Internet Protocol](https://en.wikipedia.org/wiki/Serial_Line_Internet_Protocol) which is often used to send OSC over serial or TCP connection. To use SLIP encoding instead of COBS, use `SLIPPacketSerial` instead of `PacketSerial`. You can find some example about sending OSC data over serial in the [ofxSerial](https://github.com/bakercp/ofxSerial) repository.

## PacketSerial

The `PacketSerial` class wraps the standard Arduino `Serial` class to automatically encode and decode byte packets.  Thus users can still call methods on the `Serial` object (e.g. `Serial.write()`, the built in `serialEvent()` callback etc), but it is not recommended.  Users are advised to let PacketSerial manage all Serial communication via the packet handler callback for incoming packets and the `send(const uint8_t* buffer, size_t size)` method for outgoing packets.  Mixing raw Serial calls with PacketSerial may lead to unexpected results, as the endpoint will not know what data is encoded and what data is not.


## Setup 

For Arduino boards with more than one serial port, `PacketSerial` the desired serial port can be specified with the `begin` method, i.e.

```c++
    void begin(unsigned long baud, size_t port = 0)
```

Where:

| _Arduino_ Serial Port | _PacketSerial_ Port Number |
| ------------- |:-------------:|
| `Serial`      | 0           |
| `Serial1`     | 1           |
| `Serial2`     | 2           |
| `Serial3`     | 3           |

To use a software serial port you can also use the `begin` method, this time with only a Stream* as argument, i.e.
```c++
    void begin(Stream* serial)
```

Usage:
```c++
    PacketSerial packet_serial;
    SoftwareSerial software_serial(10, 11);
    
    // in this case the serial port has to be initialized already when passing it to PacketSerial!
    software_serial.begin(38400);
    packet_serial.begin(&software_serial);
```


To receive decoded packets automatically, the user should register a packet callback.  The packet callback should be placed in your main Arduino Sketch and should have a method that looks like this signatur that looks like:

```c++
void onPacket(const uint8_t* buffer, size_t size)
{
    /// Process your incoming packet here.
}
```

Your callback can have any name and should be registered in the `setup()` method like this:

```c++
void setPacketHandler(PacketHandlerFunction onPacketFunction)
```

## Main Loop

In order to processing incoming serial packets, the user must call the `update()` method at the end of the `loop()` method.

```c++
void loop()
{
    // Your program here. 
    
    serial.update();
}

```

## Sending Packets

To send packets call the `send()` method.  The send method will take a packet (an array of bytes, encode it, transmit the array and transmit the packet boundary marker (`0`).  To send the values 255 and 10, one might do the following:

```c++

// Make an array.
uint8_t myPacket[] { 255, 10 };

serial.send(myPacket, 2);
```
 
# Example

In this "reverse echo" example, we listen for incoming packets.  When a new packet arrives in the `onPacket` method, we reverse the contents and send it back to the sender.  

```c++
#include <PacketSerial.h>


// The PacketSerial object.
// It cleverly wraps one of the Serial objects.
// While it is still possible to use the Serial object
// directly, it is recommended that the user let the 
// PacketSerial object manage all serial communication.
// Thus the user should not call Serial.write(), etc.
// Additionally the user should not use the serialEvent()
// callbacks.
PacketSerial serial;


void setup()
{
  // We must specify a packet handler method so that
  serial.setPacketHandler(&onPacket);
  serial.begin(115200);
}


void loop()
{
  // Do other things here.
  
  // The update() method attempts to read in
  // any incoming serial data and emits packets via
  // the user's onPacket(const uint8_t* buffer, size_t size) 
  // method registered with the setPacketHandler() method.  
  //
  // The update() method should be called at the end of the loop().
  serial.update(); 
}

// This is our packet callback.
// The buffer is delivered already decoded.
void onPacket(const uint8_t* buffer, size_t size)
{
  // Make a temporary buffer.
  uint8_t tmp[size]; 
  
  // Copy the packet into our temporary buffer.
  memcpy(tmp, buffer, size); 
  
  // Reverse our  buffer.
  reverse(tmp, size);
  
  // Send the reversed buffer back.
  // The send() method will encode the buffer
  // as a packet, set packet markers, etc.
  serial.send(tmp, size);
}

/// \brief A simple array reversal method.
void reverse(uint8_t* buffer, size_t size)
{
  uint8_t tmp;
  
  for (int i=0; i < size / 2; i++)
  {
    tmp = buffer[i];
    buffer[i] = buffer[size-i-1];
    buffer[size-i-1] = tmp;
  }
}
```

## Compatible libraries

- openFrameworks
    - https://github.com/bakercp/ofxSerial
    - See the `ofx::IO::PacketSerial` object which is directly compatible with this library. 


## License
See [LICENSE.md](LICENSE.md)
