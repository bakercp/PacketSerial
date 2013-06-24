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


#include "COBSSerial.h"


#if defined(USBCON)
COBSSerial::COBSSerial(Serial_& serial, 
                       size_t readBufferLength, 
                       size_t writeBufferLength) :
    _pSerial(&serial),
    _bIsUSBCON(true),
    _pReadBuffer(NULL),
    _pWriteBuffer(NULL),
    _status(BUFFER_FILLING)
{
    _pReadBuffer  = new COBSBuffer(readBufferLength);
    _pWriteBuffer = new COBSBuffer(writeBufferLength);
}
#endif

//------------------------------------------------------------------------------
COBSSerial::COBSSerial(HardwareSerial& serial, 
                       size_t readBufferLength,
                       size_t writeBufferLength) :
    _pSerial(&serial),
    _bIsUSBCON(false),
    _pReadBuffer(NULL),
    _pWriteBuffer(NULL),
    _status(BUFFER_FILLING)
{
    _pReadBuffer  = new COBSBuffer(readBufferLength);
    _pWriteBuffer = new COBSBuffer(writeBufferLength);
}

//------------------------------------------------------------------------------
COBSSerial::~COBSSerial()
{
    delete _pReadBuffer;
    _pReadBuffer = NULL;
    delete _pWriteBuffer;
    _pWriteBuffer = NULL;
}

//------------------------------------------------------------------------------    
void COBSSerial::begin(unsigned long baud)
{
    if(!_bIsUSBCON) 
    {
        ((HardwareSerial*)_pSerial)->begin(baud);
    } 
#if defined(USBCON)
    else 
    {
        ((Serial_*)_pSerial)->begin(baud);
    }
#endif
}

//------------------------------------------------------------------------------
void COBSSerial::end()
{
    if(!_bIsUSBCON) 
    {
        ((HardwareSerial*)_pSerial)->end();
    } 
#if defined(USBCON)
    else 
    {
        ((Serial_*)_pSerial)->end();
    }
#endif
}

//------------------------------------------------------------------------------
void COBSSerial::update()
{
    if(isPacketReady())
    {
        // the status was not cleared
        _status = BUFFER_FILLING | PACKET_MISSED;
    }

    while(_pSerial->available() > 0)
    {
        int byteIn = _pSerial->read();

        // check for packet boundary
        if(byteIn == DEFAULT_PACKET_BOUNDARY)
        {
            if(_pReadBuffer->empty())
            {
                _status |= PACKET_EMPTY;
            } 
            else
            {
                _status |= PACKET_READY;
            }
            break;
        }
        else
        {
            size_t nBytes = _pReadBuffer->write((uint8_t)byteIn);
            if(nBytes > 0)
            {
                _status |= BUFFER_FILLING;
            } 
            else
            {
                _status |= BUFFER_OVERFLOW;
            }
        }
    }
}

//------------------------------------------------------------------------------
int COBSSerial::status() const
{
    return _status;
}

//------------------------------------------------------------------------------
bool COBSSerial::isFilling() const 
{
    return _status & BUFFER_FILLING;
}

//------------------------------------------------------------------------------
bool COBSSerial::didOverflow() const
{
    return _status & BUFFER_OVERFLOW;
}

//------------------------------------------------------------------------------
bool COBSSerial::isPacketReady() const
{
    return _status & PACKET_READY;
}

//------------------------------------------------------------------------------
bool COBSSerial::wasPacketMissed() const 
{
    return _status & PACKET_MISSED;
}

//------------------------------------------------------------------------------
bool COBSSerial::isPacketEmpty() const 
{
    return _status & PACKET_EMPTY;
}

//------------------------------------------------------------------------------
size_t COBSSerial::readPacket(uint8_t* buffer, size_t size)
{
    // TODO: check the length of input for a worst case scenario
    _status = BUFFER_FILLING;

    return cobsDecode(_pReadBuffer->data(), _pReadBuffer->index(), buffer);
}

//------------------------------------------------------------------------------
size_t COBSSerial::writePacket(const uint8_t* buffer, size_t size)
{
    _pWriteBuffer->clear();

    // TODO: check the length of input for a worst case scenario
    size_t numBytes = cobsEncode(buffer, size, _pWriteBuffer->data());

    _pWriteBuffer->write(_pWriteBuffer->data(),_pWriteBuffer->index());

    return numBytes;
}

/// Stuffs "length" bytes of data at the location pointed to by
/// "input", writing the output to the location pointed to by
/// "output". Returns the number of bytes written to "output".
//------------------------------------------------------------------------------
size_t COBSSerial::cobsEncode(const uint8_t* input, 
                              size_t size, 
                              uint8_t* output)
{
    size_t  read_index  = 0;
    size_t  write_index = 1;
    size_t  code_index  = 0;
    uint8_t code        = 1;

    while(read_index < size)
    {
        if(input[read_index] == 0)
        {
            output[code_index] = code;
            code = 1;
            code_index = write_index++;
            read_index++;
        }
        else
        {
            output[write_index++] = input[read_index++];
            code++;
            if(code == 0xFF)
            {
                output[code_index] = code;
                code = 1;
                code_index = write_index++;
            }
        }
    }

    output[code_index] = code;

    return write_index;
}

/// Unstuffs "size" bytes of data at the location pointed to by
/// "input", writing the output * to the location pointed to by
/// "output". Returns the number of bytes written to "output" if
/// "input" was successfully unstuffed, and 0 if there was an
/// error unstuffing "input".
//------------------------------------------------------------------------------
size_t COBSSerial::cobsDecode(const uint8_t* input, 
                              size_t size, 
                              uint8_t* output)
{
    size_t  read_index  = 0;
    size_t  write_index = 0;
    uint8_t code;
    uint8_t i;

    while(read_index < size)
    {
        code = input[read_index];

        if(read_index + code > size && code != 1)
        {
            return 0;
        }

        read_index++;

        for(i = 1; i < code; i++)
        {
            output[write_index++] = input[read_index++];
        }
        if(code != 0xFF && read_index != size)
        {
            output[write_index++] = '\0';
        }
    }

    return write_index;
}
