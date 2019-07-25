# Getting Started

First, it might be worth it to read the [background introduction](BACKGROUND.md).

## Installation

This project can be installed using the Arduino IDE. Navigate to the _Tools > Manage Libraries ..._ menu item and search for `PacketSerial`. 

Alternatively, this library can be downloaded or cloned from github and to your Arduino `libraries` folder.

## Use

`PacketSerial` class wraps the Arduino `Stream` class to automatically encode and decode byte packets that are sent and received. Typically serial communication uses the default `Serial` object, which implements the `Stream` class. In most cases, `PacketSerial` should be given exclusive access to the serial `Stream` (e.g. for a default setup using `Serial`, users should avoid calling functions like `Serial.print()`, `Serial.write()`, etc directly). Data should be sent via the `send(const uint8_t* buffer, size_t size) const` method and received in a `PacketSerial` callback function (see below).

## Setup

### Basic

To use the default `Serial` object and the default communication settings (usually `SERIAL_8N1`), set up `PacketSerial` like this:

```cpp
PacketSerial myPacketSerial;

void setup()
{
    myPacketSerial.begin(9600);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

### Advanced

For a non-default Serial connection, a class implementing the `Stream` interface should be configured and then set for the `PacketSerial` instance.

#### Using A Non-Standard Serial Configuration

```cpp
PacketSerial myPacketSerial;

void setup()
{
    Serial.begin(300, SERIAL_7N1);
    myPacketSerial.setStream(&Serial);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

#### Using Secondary Serial Ports (e.g. Serial1, Serial2, etc)

```cpp
PacketSerial myPacketSerial;

void setup()
{
    Serial1.begin(9600);
    myPacketSerial.setStream(&Serial1);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

#### Using SoftwareSerial

```cpp
PacketSerial myPacketSerial;
SoftwareSerial mySoftwareSerial(10, 11);

void setup()
{
    mySoftwareSerial.begin(38400);
    myPacketSerial.setStream(&mySoftwareSerial);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

#### Other Streams

Any class that correctly implements the `Stream` interface should work, which includes some network communication objects.

### Loop

In order to processing incoming serial packets, the user must call the `update()` method at the end of the `loop()` method.

```cpp
void loop()
{
    // Your program here.


    // Call update to receive, decode and process incoming packets.
    myPacketSerial.update();
}

```

### Receiving Packets

All packets are received via handler functions. A typical handler function would be registered in the `void setup()` function like:

```cpp
PacketSerial myPacketSerial;

void setup()
{
    myPacketSerial.begin(9600);
    myPacketSerial.setPacketHandler(&onPacketReceived);
}
```

The `onPacketReceived` function can take two forms. The simplest looks like this:

```cpp
void onPacketReceived(const uint8_t* buffer, size_t size)
{
    // Process your decoded incoming packet here.
}
```

For more advanced programs with multiple PacketSerial instances and a shared handler, it may be useful to know which PacketSerial instance received the packet. In this case you could define a callback like this:

```cpp
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

```cpp
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

```cpp

// Make an array.
uint8_t myPacket[2] = { 255, 10 };

// Send the array.
myPacketSerial.send(myPacket, 2);
```

### Multiple Streams

On boards with multiple serial ports, this strategy can also be used to set up two Serial streams, one for packets and one for debug ASCII (see [this discussion](https://github.com/bakercp/PacketSerial/issues/10) for more).

### Checking for Receive Buffer Overflows

In some cases the receive buffer may not be large enough for an incoming encoded packet.

To check for overflows, call the `receiveBufferOverflowed()` method after calling `update()`.  

For example:

```cpp
void loop()
{
    // Other program code.
    myPacketSerial.update();

    // Check for a receive buffer overflow.
    if (myPacketSerial.overflow())
    {
        // Send an alert via a pin (e.g. make an overflow LED) or return a
        // user-defined packet to the sender.
        //
        // Ultimately you may need to just increase your recieve buffer via the
        // template parameters.
    }
}
```

The state of the overflow flag is reset every time a new packet marker is detected, NOT when the `overflow()` method is called.

### Customizing the PacketSerial Class

The `PacketSerial_` class is a templated class that allows us to statically set the encoder type, packet marker and buffer size at compile time.

The the template parameters are as follows:

```cpp
template<typename EncoderType, uint8_t PacketMarker = 0, size_t BufferSize = 256>
class PacketSerial_

(...)
```

The `PacketMarker` has a default of `0` while the `BufferSize` has a default of `256` bytes.

Thus, if you define your class as:

```cpp
PacketSerial_<COBS> myPacketSerial;
```

You will use the `COBS` encoder type and a default PacketMarker of `0` and buffer size of `256`.

Currently there are three default `PacketSerial_` types defined via `typedef` for convenience:

```cpp
/// \brief A typedef for the default COBS PacketSerial class.
typedef PacketSerial_<COBS> PacketSerial;

/// \brief A typedef for a PacketSerial type with COBS encoding.
typedef PacketSerial_<COBS> COBSPacketSerial;

/// \brief A typedef for a PacketSerial type with SLIP encoding.
typedef PacketSerial_<SLIP, SLIP::END> SLIPPacketSerial;
```

### Changing the EncoderType Type

To use a custom encoding type, the `EncoderType` class must implement the following functions:

```cpp
    static size_t encode(const uint8_t* buffer, size_t size, uint8_t* encodedBuffer);
    static size_t decode(const uint8_t* encodedBuffer, size_t size, uint8_t* decodedBuffer);
    static size_t getEncodedBufferSize(size_t unencodedBufferSize);
```

See the `Encoding/COBS.h` and `Encoding/SLIP.h` for examples and further documentation.

### Changing the Packet Marker Byte and Receive Buffer Size

For example, to increase the buffer size for a standard `COBS` encoder to 512, one can defined the templated class like this:

```cpp
PacketSerial_<COBS, 0, 512> myPacketSerial;
```

This uses the COBS encoder type, a PacketMarker of 0 and a buffer size of 512.

Likewise, a custom `SLIP` encoder with a buffer size of 512 bytes would be defined like this:

```cpp
PacketSerial_<SLIP, SLIP::END, 512> myPacketSerial;
```