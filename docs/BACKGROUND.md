# Background

_Why do I need this?_ you may ask. The truth is that you may not need it if you are converting your values to ASCII strings and separating them with a known character (like a carriage return `\r` and a line feed `\n`) before sending them. This is what happens if you call and `Serial.println();`. For instance, if you just want to send a byte with the value of 255 and follow it with a new line character (i.e. `Serial.println(255);`) the Arduino automatically converts the number to the equivalent printable ASCII characters, sending 5 bytes total. As a result the receiver won't just receive a byte for the number and two bytes for the carriage return and new line character. Instead it will receive a stream of 5 bytes:

```console
50 // ASCII '2'
53 // ASCII '5'
53 // ASCII '5'
13 // ASCII '\r'
10 // ASCII '\n'
```

The receiver must then collect the 3 ASCII characters { '2', '5', '5' }, combine them and convert them back into a single byte with a value of `255`. This process can get complicated when the user wants to send large quantities of structured data between the Arduino and a receiver.

One way to send a _packet_ of data without this library is to send each byte separated by a comma or space and terminate the sequence with a new line character. Thus, to send the value `255` and the value `10`, one might call:

```cpp
Serial.print(255);
Serial.print(',');
Serial.print(10);
Serial.print('\n');
```

The receiver will actually see a stream of 7 bytes:

```console
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

```cpp
Serial.write(255);
Serial.write('\n');
```

the receiver will see a stream of 2 bytes:

```console
255 // The value transmitted.
10  // The new line character (\n).
```

This is much more compact but can create problems when the user wants to send a _packet_ of data. If the user wants to send a packet consisting of two values such as 255 and 10, we run into problems if we also use the new line (`\n` ASCII 10) character as a packet boundary. This essentially means that the receiver will incorrectly think that a new packet is beginning when it receives the _value_ of 10. Thus, to use this more compact form of sending bytes while reserving one value for a packet boundary marker. Several unambiguous packet boundary marking encodings exist, but one with a small predictable overhead is called [Consistent Overhead Byte Stuffing](http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing). For a raw packet of length `SIZE`, the maximum encoded buffer size will only be `SIZE + SIZE / 254 + 1`. This is significantly less than ASCII encoding and the encoding / decoding algorithm is simple and fast. In its default mode, the COBS encoding process simply removes all _zeros_ from the packet, allowing the sender and receiver to use the value of _zero_ as a packet boundary marker.

Another encoding available in `PacketSerial` is [Serial Line Internet Protocol](https://en.wikipedia.org/wiki/Serial_Line_Internet_Protocol) which is often used to send OSC over serial or TCP connections. To use SLIP encoding instead of COBS, use `SLIPPacketSerial` instead of `PacketSerial`. You can find an openFrameworks example of sending OSC data over serial in the [ofxSerial](https://github.com/bakercp/ofxSerial) repository.
