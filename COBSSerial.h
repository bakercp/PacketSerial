// =============================================================================
// COBS Implementation
// Copyright 2011, Jacques Fortier. All rights reserved.
// Library
// Copyright (c) 2013 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#pragma once


#include "Arduino.h"
#include "COBSBuffer.h"


class COBSSerial {
public:
    enum BufferStatus {
        BUFFER_FILLING  = 0x01,
        BUFFER_OVERFLOW = 0x02,
        PACKET_READY    = 0x04,
        PACKET_MISSED   = 0x08,
        PACKET_EMPTY    = 0x10
    };

#if defined(USBCON)
    COBSSerial(Serial_& serial, 
               size_t readBufferLength  = DEFAULT_BUFFER_LENGTH, 
               size_t writeBufferLength = DEFAULT_BUFFER_LENGTH);
#endif

    COBSSerial(HardwareSerial& serial, 
               size_t readBufferLength  = DEFAULT_BUFFER_LENGTH, 
               size_t writeBufferLength = DEFAULT_BUFFER_LENGTH);

    ~COBSSerial();

    void begin(unsigned long baud);
    void end();

    void update();

    int status() const;

    bool isFilling() const;
    bool didOverflow() const;
    bool isPacketReady() const;
    bool wasPacketMissed() const;
    bool isPacketEmpty() const;

    size_t readPacket(uint8_t* buffer, size_t size);
    size_t writePacket(const uint8_t* buffer, size_t size);

protected:
    static size_t cobsEncode(const uint8_t* input, size_t size, uint8_t* output);
    static size_t cobsDecode(const uint8_t* input, size_t size, uint8_t* output);

private:
    enum {
        DEFAULT_PACKET_BOUNDARY = 0,
        DEFAULT_BUFFER_LENGTH = 1024
    };

    Stream* _pSerial;
    bool _bIsUSBCON;

    COBSBuffer* _pReadBuffer;
    COBSBuffer* _pWriteBuffer;

    int _status;

};








