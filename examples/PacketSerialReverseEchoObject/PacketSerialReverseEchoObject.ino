//
// Copyright (c) 2020 Jan Perner <https://github.com/janperner>
//
// SPDX-License-Identifier: MIT
//

#include <PacketSerial.h>

// This example does exactly the same stuff as PacketSerialReverseEcho.ino.
// The only difference here is that PacketSerial is encapsulated by a user
// object which can build another layer, e.g. error detection, on top of what
// PacketSerial does.

namespace {

template <typename TPacketSerial> class ReverseEchoClient final
{
public: // public special member functions
  ReverseEchoClient(Stream &serialStream) : _serialStream(serialStream)
  {
    _packetSerial.setStream(&serialStream);
    _packetSerial.setPacketHandler(
        [](const uint8_t *buffer, size_t size, void *context) {
          auto self = reinterpret_cast<ReverseEchoClient *>(context);
          self->onPacketReceived(buffer, size);
        },
        this);
  }

public: // public member functions
  void update()
  {
    _packetSerial.update();
  }

private: // private member functions
  void onPacketReceived(const uint8_t *buffer, size_t size)
  {
    if (size == 0) {
      Serial.println("no data");
      return;
    }

    Serial.println("data");

    uint8_t tempBuffer[size];
    memcpy(tempBuffer, buffer, size);

    reverse(tempBuffer, size);

    _packetSerial.send(tempBuffer, size);
  }

  void reverse(uint8_t *buffer, size_t size)
  {
    uint8_t tmp;

    for (size_t i = 0; i < size / 2; ++i) {
      tmp = buffer[i];
      buffer[i] = buffer[size - i - 1];
      buffer[size - i - 1] = tmp;
    }
  }

private: // private members
  Stream &_serialStream;
  TPacketSerial _packetSerial;
};

ReverseEchoClient<PacketSerial> _reverseEchoClient(Serial);

} // namespace

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  _reverseEchoClient.update();
}
