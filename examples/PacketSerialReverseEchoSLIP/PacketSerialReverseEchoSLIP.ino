//
// Copyright (c) 2012 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier: MIT
//


#include <PacketSerial.h>


// By default, PacketSerial automatically wraps the built-in `Serial` object.
// While it is still possible to use the Serial object directly, it is
// recommended that the user let the PacketSerial object manage all serial
// communication. Thus the user should not call Serial.write(), Serial.print(),
// etc. Additionally the user should not use the serialEvent() framework.
//
// By default, SLIPPacketSerial uses SLIP encoding and has a 256 byte receive
// buffer. This can be adjusted by the user by replacing `SLIPPacketSerial`
// with a variation of the `PacketSerial_<SLIP, SLIP::END, BufferSize>` template
// found in PacketSerial.h.
SLIPPacketSerial myPacketSerial;


void setup()
{
  // We begin communication with our PacketSerial object by setting the
  // communication speed in bits / second (baud).
  myPacketSerial.begin(115200);

  // If we want to receive packets, we must specify a packet handler function.
  // The packet handler is a custom function with a signature like the
  // onPacketReceived function below.
  myPacketSerial.setPacketHandler(&onPacketReceived);
}


void loop()
{
  // Do your program-specific loop() work here as usual.

  // The PacketSerial::update() method attempts to read in any incoming serial
  // data and emits received and decoded packets via the packet handler
  // function specified by the user in the void setup() function.
  //
  // The PacketSerial::update() method should be called once per loop(). Failure
  // to call the PacketSerial::update() frequently enough may result in buffer
  // serial overflows.
  myPacketSerial.update();

  // Check for a receive buffer overflow (optional).
  if (myPacketSerial.overflow())
  {
    // Send an alert via a pin (e.g. make an overflow LED) or return a
    // user-defined packet to the sender.
    //
    // Ultimately you may need to just increase your recieve buffer via the
    // template parameters (see the README.md).
  }
}

// This is our handler callback function.
// When an encoded packet is received and decoded, it will be delivered here.
// The `buffer` is a pointer to the decoded byte array. `size` is the number of
// bytes in the `buffer`.
void onPacketReceived(const uint8_t* buffer, size_t size)
{
  // In this example, we will simply reverse the contents of the array and send
  // it back to the sender.

  // Make a temporary buffer.
  uint8_t tempBuffer[size];

  // Copy the packet into our temporary buffer.
  memcpy(tempBuffer, buffer, size);

  // Reverse our temporaray buffer.
  reverse(tempBuffer, size);

  // Send the reversed buffer back to the sender. The send() method will encode
  // the whole buffer as as single packet, set packet markers, etc.
  // The `tempBuffer` is a pointer to the `tempBuffer` array and `size` is the
  // number of bytes to send in the `tempBuffer`.
  myPacketSerial.send(tempBuffer, size);
}

// This function takes a byte buffer and reverses it.
void reverse(uint8_t* buffer, size_t size)
{
  uint8_t tmp;

  for (size_t i = 0; i < size / 2; i++)
  {
    tmp = buffer[i];
    buffer[i] = buffer[size - i - 1];
    buffer[size - i - 1] = tmp;
  }
}
