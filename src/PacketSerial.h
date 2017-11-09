//
// Copyright (c) 2013 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include <Arduino.h>
#include "Encoding/COBS.h"
#include "Encoding/SLIP.h"


/// \brief A template class defining a packet-based Serial device.
///
/// Typically one of the typedefined versions are used, for example,
/// COBSPacketSerial or SLIPPacketSerial.
///
/// The template parameters allow the user to define their own packet encoder /
/// decoder, custom packet marker and receive buffer size.
///
/// \tparam EncoderType The static packet encoder class name.
/// \tparam PacketMarker The byte value used to mark the packet boundary.
/// \tparam BufferSize The number of bytes allocated for the receive buffer.
template<typename EncoderType, uint8_t PacketMarker = 0, size_t BufferSize = 256>
class PacketSerial_
{
public:
    /// \brief A typedef describing the packet handler method.
    ///
    /// The packet handler method usually has the form:
    ///
    ///     void onPacket(const uint8_t* buffer, size_t size)
    ///
    /// where buffer is a pointer to the incoming buffer array, and size is the
    /// number of bytes in the incoming buffer.
    typedef void (*PacketHandlerFunction)(const uint8_t* buffer, size_t size);

    /// \brief Construct a default PacketSerial_ device.
    PacketSerial_():
        _receiveBufferIndex(0),
        _stream(nullptr),
        _onPacketFunction(nullptr)
    {
    }

    /// \brief Destroy the PacketSerial_ device.
    ~PacketSerial_()
    {
    }

    /// \brief Begin a serial connection with the given speed.
    ///
    /// The default Serial Port `Serial` and default config `SERIAL_8N1` will be
    /// used.
    ///
    /// \param speed The serial data transmission speed in bits / second (baud).
    /// \sa https://www.arduino.cc/en/Serial/Begin
    void begin(unsigned long speed)
    {
        begin(speed, SERIAL_8N1, 0);
    }

    /// \brief Deprecated begin() method.
    /// \param speed The serial data transmission speed in bits / second (baud).
    /// \param port The Serial port number (e.g. 0 is Serial, 1 is Serial1).
    /// \deprecated Use the alternative begin().
    void begin(unsigned long speed, size_t port) __attribute__ ((deprecated))
    {
        begin(port, SERIAL_8N1, port);
    }

    /// \brief Begin a serial connection with the given speed, config and port.
    /// \param speed The serial data transmission speed in bits / second (baud).
    /// \param config The data, parity, and stop bits configuration.
    /// \param port The Serial port number (e.g. 0 is Serial, 1 is Serial1).
    /// \sa https://www.arduino.cc/en/Serial/Begin
    void begin(unsigned long speed, uint8_t config, size_t port)
    {
        switch(port)
        {
        #if defined(UBRR1H)
            case 1:
                Serial1.begin(speed, config);
                _stream = &Serial1;
                break;
        #endif
        #if defined(UBRR2H)
            case 2:
                Serial2.begin(speed, config);
                _stream = &Serial2;
                break;
        #endif
        #if defined(UBRR3H)
            case 3:
                Serial3.begin(speed, config);
                _stream = &Serial3;
                break;
        #endif
            default:
                Serial.begin(speed, config);
                _stream = &Serial;
        }
    }

    /// \brief Begin a serial connection with the given Stream.
    ///
    /// This Stream could be a standard Serial object e.g.:
    ///
    ///     myPacketSerial.begin(&Serial);
    ///
    /// or can be any `Stream` object, including a network or other connection
    /// that implements the `Stream` interface.
    ///
    /// \param stream A pointer to an Arduino `Stream`.
    void begin(Stream* stream)
    {
        _stream = stream;
    }

    /// \brief The update function services the serial connection.
    ///
    /// This must be called often, ideally once per `loop()`, e.g.:
    ///
    ///     void loop()
    ///     {
    ///         // Other program code.
    ///
    ///         myPacketSerial.update();
    ///     }
    ///
    void update()
    {
        if (_stream == nullptr) return;

        while (_stream->available() > 0)
        {
            uint8_t data = _stream->read();

            if (data == PacketMarker)
            {
                if (_onPacketFunction)
                {
                    uint8_t _decodeBuffer[_receiveBufferIndex];

                    size_t numDecoded = EncoderType::decode(_receiveBuffer,
                                                            _receiveBufferIndex,
                                                            _decodeBuffer);

                    _onPacketFunction(_decodeBuffer, numDecoded);
                }

                _receiveBufferIndex = 0;
            }
            else
            {
                if ((_receiveBufferIndex + 1) < BufferSize)
                {
                    _receiveBuffer[_receiveBufferIndex++] = data;
                }
                else
                {
                    // Error, buffer overflow if we write.
                }
            }
        }
    }

    /// \brief Set a packet of data.
    ///
    /// This function will encode and send an arbitrary packet of data. After
    /// sending, it will send the specified `PacketMarker` defined in the
    /// template parameters.
    ///
    /// \param buffer A pointer to a data buffer.
    /// \param size The number of bytes in the data buffer.
    void send(const uint8_t* buffer, size_t size) const
    {
        if(_stream == nullptr || buffer == nullptr || size == 0) return;

        uint8_t _encodeBuffer[EncoderType::getEncodedBufferSize(size)];

        size_t numEncoded = EncoderType::encode(buffer,
                                                size,
                                                _encodeBuffer);

        _stream->write(_encodeBuffer, numEncoded);
        _stream->write(PacketMarker);
    }

    /// \brief Set the function that will receive decoded packets.
    ///
    /// This function will be called when data is read from the serial stream
    /// connection and a packet is decoded. The decoded packet will be passed
    /// to the packet handler. The packet handler must have the form:
    ///
    /// The packet handler method usually has the form:
    ///
    ///     void onPacket(const uint8_t* buffer, size_t size)
    ///
    /// The packet handler would then be registered like:
    ///
    ///     myPacketSerial.setPacketHandler(&onPacket);
    ///
    /// \param onPacketFunction A pointer to the packet handler function.
    void setPacketHandler(PacketHandlerFunction onPacketFunction)
    {
        _onPacketFunction = onPacketFunction;
    }

private:
    PacketSerial_(const PacketSerial_&);
    PacketSerial_& operator = (const PacketSerial_&);

    uint8_t _receiveBuffer[BufferSize];
    size_t _receiveBufferIndex = 0;

    Stream* _stream = nullptr;

    PacketHandlerFunction _onPacketFunction = nullptr;

};


/// \brief A typedef for the default COBS PacketSerial class.
typedef PacketSerial_<COBS> PacketSerial;

/// \brief A typedef for a PacketSerial type with COBS encoding.
typedef PacketSerial_<COBS> COBSPacketSerial;

/// \brief A typedef for a PacketSerial type with SLIP encoding.
typedef PacketSerial_<SLIP, SLIP::END> SLIPPacketSerial;
