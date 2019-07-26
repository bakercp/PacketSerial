//
// Copyright (c) 2013 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier: MIT
//


#pragma once


#include <Arduino.h>
#include "Encoding/COBS.h"
#include "Encoding/SLIP.h"


/// \brief A template class enabling packet-based Serial communication.
///
/// Typically one of the typedefined versions are used, for example,
/// `COBSPacketSerial` or `SLIPPacketSerial`.
///
/// The template parameters allow the user to define their own packet encoder /
/// decoder, custom packet marker and receive buffer size.
///
/// \tparam EncoderType The static packet encoder class name.
/// \tparam PacketMarker The byte value used to mark the packet boundary.
/// \tparam BufferSize The number of bytes allocated for the receive buffer.
template<typename EncoderType, uint8_t PacketMarker = 0, size_t ReceiveBufferSize = 256>
class PacketSerial_
{
public:
    /// \brief A typedef describing the packet handler method.
    ///
    /// The packet handler method usually has the form:
    ///
    ///     void onPacketReceived(const uint8_t* buffer, size_t size);
    ///
    /// where buffer is a pointer to the incoming buffer array, and size is the
    /// number of bytes in the incoming buffer.
    typedef void (*PacketHandlerFunction)(const uint8_t* buffer, size_t size);

    /// \brief A typedef describing the packet handler method.
    ///
    /// The packet handler method usually has the form:
    ///
    ///     void onPacketReceived(void* sender, const uint8_t* buffer, size_t size);
    ///
    /// where sender is a pointer to the PacketSerial_ instance that recieved
    /// the buffer,  buffer is a pointer to the incoming buffer array, and size
    /// is the number of bytes in the incoming buffer.
    typedef void (*PacketHandlerFunctionWithSender)(const void* sender, const uint8_t* buffer, size_t size);

    /// \brief Construct a default PacketSerial_ device.
    PacketSerial_():
        _receiveBufferIndex(0),
        _stream(nullptr),
        _onPacketFunction(nullptr),
        _onPacketFunctionWithSender(nullptr)
    {
    }

    /// \brief Destroy the PacketSerial_ device.
    ~PacketSerial_()
    {
    }

    /// \brief Begin a default serial connection with the given speed.
    ///
    /// The default Serial port `Serial` and default config `SERIAL_8N1` will be
    /// used. For example:
    ///
    ///     PacketSerial myPacketSerial;
    ///
    ///     void setup()
    ///     {
    ///         myPacketSerial.begin(9600);
    ///     }
    ///
    /// This is a convenience method. For more complex Serial port
    /// configurations, use the `setStream()` function to set an arbitrary
    /// Arduino Stream.
    ///
    /// \param speed The serial data transmission speed in bits / second (baud).
    /// \sa https://www.arduino.cc/en/Serial/Begin
    void begin(unsigned long speed)
    {
        Serial.begin(speed);
        #if ARDUINO >= 100 && !defined(CORE_TEENSY)
        while (!Serial) {;}
        #endif
        setStream(&Serial);
    }

    /// \brief Deprecated. Use setStream() to configure a non-default port.
    /// \param speed The serial data transmission speed in bits / second (baud).
    /// \param port The Serial port number (e.g. 0 is Serial, 1 is Serial1).
    /// \deprecated Use setStream() to configure a non-default port.
    void begin(unsigned long speed, size_t port) __attribute__ ((deprecated))
    {
        switch(port)
        {
        #if defined(UBRR1H)
            case 1:
                Serial1.begin(speed);
                #if ARDUINO >= 100 && !defined(CORE_TEENSY)
                while (!Serial1) {;}
                #endif
                setStream(&Serial1);
                break;
        #endif
        #if defined(UBRR2H)
            case 2:
                Serial2.begin(speed);
                #if ARDUINO >= 100 && !defined(CORE_TEENSY)
                while (!Serial1) {;}
                #endif
                setStream(&Serial2);
                break;
        #endif
        #if defined(UBRR3H)
            case 3:
                Serial3.begin(speed);
                #if ARDUINO >= 100 && !defined(CORE_TEENSY)
                while (!Serial3) {;}
                #endif
                setStream(&Serial3);
                break;
        #endif
            default:
                begin(speed);
        }
    }

    /// \brief Deprecated. Use setStream() to configure a non-default port.
    /// \param stream A pointer to an Arduino `Stream`.
    /// \deprecated Use setStream() to configure a non-default port.
    void begin(Stream* stream) __attribute__ ((deprecated))
    {
        _stream = stream;
    }

    /// \brief Attach PacketSerial to an existing Arduino `Stream`.
    ///
    /// This `Stream` could be a standard `Serial` `Stream` with a non-default
    /// configuration such as:
    ///
    ///     PacketSerial myPacketSerial;
    ///
    ///     void setup()
    ///     {
    ///         Serial.begin(300, SERIAL_7N1);
    ///         myPacketSerial.setStream(&Serial);
    ///     }
    ///
    /// Or it might be a `SoftwareSerial` `Stream` such as:
    ///
    ///     PacketSerial myPacketSerial;
    ///     SoftwareSerial mySoftwareSerial(10, 11);
    ///
    ///     void setup()
    ///     {
    ///         mySoftwareSerial.begin(38400);
    ///         myPacketSerial.setStream(&mySoftwareSerial);
    ///     }
    ///
    /// Any class that implements the `Stream` interface should work, which
    /// includes some network objects.
    ///
    /// \param stream A pointer to an Arduino `Stream`.
    void setStream(Stream* stream)
    {
        _stream = stream;
    }

    /// \brief Get a pointer to the current stream.
    /// \warning Reading from or writing to the stream managed by PacketSerial_
    ///          may break the packet-serial protocol if not done so with care. 
    ///          Access to the stream is allowed because PacketSerial_ never
    ///          takes ownership of the stream and thus does not have exclusive
    ///          access to the stream anyway.
    /// \returns a non-const pointer to the stream, or nullptr if unset.
    Stream* getStream()
    {
        return _stream;
    }

    /// \brief Get a pointer to the current stream.
    /// \warning Reading from or writing to the stream managed by PacketSerial_
    ///          may break the packet-serial protocol if not done so with care. 
    ///          Access to the stream is allowed because PacketSerial_ never
    ///          takes ownership of the stream and thus does not have exclusive
    ///          access to the stream anyway.
    /// \returns a const pointer to the stream, or nullptr if unset.
    const Stream* getStream() const
    {
        return _stream;
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
                if (_onPacketFunction || _onPacketFunctionWithSender)
                {
                    uint8_t _decodeBuffer[_receiveBufferIndex];

                    size_t numDecoded = EncoderType::decode(_receiveBuffer,
                                                            _receiveBufferIndex,
                                                            _decodeBuffer);

                    if (_onPacketFunction)
                    {
                        _onPacketFunction(_decodeBuffer, numDecoded);
                    }
                    else if (_onPacketFunctionWithSender)
                    {
                        _onPacketFunctionWithSender(this, _decodeBuffer, numDecoded);
                    }
                }

                _receiveBufferIndex = 0;
                _recieveBufferOverflow = false;
            }
            else
            {
                if ((_receiveBufferIndex + 1) < ReceiveBufferSize)
                {
                    _receiveBuffer[_receiveBufferIndex++] = data;
                }
                else
                {
                    // The buffer will be in an overflowed state if we write
                    // so set a buffer overflowed flag.
                    _recieveBufferOverflow = true;
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
    ///     // Make an array.
    ///     uint8_t myPacket[2] = { 255, 10 };
    ///
    ///     // Send the array.
    ///     myPacketSerial.send(myPacket, 2);
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
    ///     void onPacketReceived(const uint8_t* buffer, size_t size);
    ///
    /// The packet handler would then be registered like this:
    ///
    ///     myPacketSerial.setPacketHandler(&onPacketReceived);
    ///
    /// Setting a packet handler will remove all other packet handlers.
    ///
    /// \param onPacketFunction A pointer to the packet handler function.
    void setPacketHandler(PacketHandlerFunction onPacketFunction)
    {
        _onPacketFunction = onPacketFunction;
        _onPacketFunctionWithSender = nullptr;
    }

    /// \brief Set the function that will receive decoded packets.
    ///
    /// This function will be called when data is read from the serial stream
    /// connection and a packet is decoded. The decoded packet will be passed
    /// to the packet handler. The packet handler must have the form:
    ///
    /// The packet handler method usually has the form:
    ///
    ///     void onPacketReceived(const void* sender, const uint8_t* buffer, size_t size);
    ///
    /// To determine the sender, compare the pointer to the known possible
    /// PacketSerial senders.
    ///
    ///     void onPacketReceived(void* sender, const uint8_t* buffer, size_t size)
    ///     {
    ///         if (sender == &myPacketSerial)
    ///         {
    ///             // Do something with the packet from myPacketSerial.
    ///         }
    ///         else if (sender == &myOtherPacketSerial)
    ///         {
    ///             // Do something with the packet from myOtherPacketSerial.
    ///         }
    ///     }
    ///
    /// The packet handler would then be registered like this:
    ///
    ///     myPacketSerial.setPacketHandler(&onPacketReceived);
    ///
    /// Setting a packet handler will remove all other packet handlers.
    ///
    /// \param onPacketFunctionWithSender A pointer to the packet handler function.
    void setPacketHandler(PacketHandlerFunctionWithSender onPacketFunctionWithSender)
    {
        _onPacketFunction = nullptr;
        _onPacketFunctionWithSender = onPacketFunctionWithSender;
    }

    /// \brief Check to see if the receive buffer overflowed.
    ///
    /// This must be called often, directly after the `update()` function.
    ///
    ///     void loop()
    ///     {
    ///         // Other program code.
    ///         myPacketSerial.update();
    ///
    ///         // Check for a receive buffer overflow.
    ///         if (myPacketSerial.overflow())
    ///         {
    ///             // Send an alert via a pin (e.g. make an overflow LED) or return a
    ///             // user-defined packet to the sender.
    ///             //
    ///             // Ultimately you may need to just increase your recieve buffer via the
    ///             // template parameters.
    ///         }
    ///     }
    ///
    /// The state is reset every time a new packet marker is received NOT when 
    /// overflow() method is called.
    ///
    /// \returns true if the receive buffer overflowed.
    bool overflow() const
    {
        return _recieveBufferOverflow;
    }

private:
    PacketSerial_(const PacketSerial_&);
    PacketSerial_& operator = (const PacketSerial_&);

    bool _recieveBufferOverflow = false;

    uint8_t _receiveBuffer[ReceiveBufferSize];
    size_t _receiveBufferIndex = 0;

    Stream* _stream = nullptr;

    PacketHandlerFunction _onPacketFunction = nullptr;
    PacketHandlerFunctionWithSender _onPacketFunctionWithSender = nullptr;
};


/// \brief A typedef for the default COBS PacketSerial class.
typedef PacketSerial_<COBS> PacketSerial;

/// \brief A typedef for a PacketSerial type with COBS encoding.
typedef PacketSerial_<COBS> COBSPacketSerial;

/// \brief A typedef for a PacketSerial type with SLIP encoding.
typedef PacketSerial_<SLIP, SLIP::END> SLIPPacketSerial;
