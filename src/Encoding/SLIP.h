// =============================================================================
//
// Copyright (c) 2010-2014 Christopher Baker <http://christopherbaker.net>
// 2016 Antoine Villeret added Christopher's ofx/IO/SLIPEncoding.h here
//
// see : https://github.com/bakercp/ofxIO/blob/master/libs/ofxIO/include/ofx/IO/SLIPEncoding.h
//
// Portions:
//  Copyright (c) 2011, Jacques Fortier. All rights reserved.
//  https://github.com/jacquesf/COBS-Consistent-Overhead-Byte-Stuffing
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



/// \brief A Serial Line IP (SLIP) Encoder.
///
/// Serial Line IP (SLIP) is a packet framing protocol: SLIP defines a
/// sequence of characters that frame IP packets on a serial line and
/// nothing more. It provides no addressing, packet type identification,
/// error detection/correction or compression mechanisms.  Because the
/// protocol does so little, though, it is usually very easy to
/// implement.
///
/// \sa http://tools.ietf.org/html/rfc1055
class SLIP
{
public:
    static size_t encode(const uint8_t* buffer, size_t size, uint8_t* encoded)
    {
        if (size == 0)
            return 0;

        size_t read_index  = 0;
        size_t write_index = 0;

        // double-ENDed, flush any data that may have accumulated due to line noise
        encoded[write_index++] = END;

        while (read_index < size)
        {
            if(buffer[read_index] == END)
            {
                encoded[write_index++] = ESC;
                encoded[write_index++] = ESC_END;
                read_index++;
            }
            else if(buffer[read_index] == ESC)
            {
                encoded[write_index++] = ESC;
                encoded[write_index++] = ESC_ESC;
                read_index++;
            }
            else
            {
                encoded[write_index++] = buffer[read_index++];
            }
        }

        encoded[write_index++] = END;

        return write_index;
    }

    static size_t decode(const uint8_t* buffer, size_t size, uint8_t* decoded)
    {
        if (size == 0)
            return 0;

        size_t read_index  = 0;
        size_t write_index = 0;

        while (read_index < size)
        {
            if (buffer[read_index] == END)
            {
                // flush or done
                read_index++;
            }
            else if (buffer[read_index] == ESC)
            {
                if (buffer[read_index+1] == ESC_END)
                {
                    decoded[write_index++] = END;
                    read_index += 2;
                }
                else if (buffer[read_index+1] == ESC_ESC)
                {
                    decoded[write_index++] = ESC;
                    read_index += 2;
                }
                else
                {
                    // considered a protocol violation
                }
            }
            else
            {
                decoded[write_index++] = buffer[read_index++];
            }
        }

        return write_index;
    }

    /// \brief Get the maximum encoded buffer size needed for a given source size.
    ///
    /// SLIP has a start and a end markers (192 and 219).
    /// Marker value is replaced by 2 bytes in the encoded buffer.
    /// So in the worst case of sending a buffer with only '192' or '219',
    /// the encoded buffer length will be 2 * buffer.size() + 2
    /// \param sourceSize The size of the buffer to be encoded.
    /// \returns the maximum size of the required encoded buffer.
    static size_t getEncodedBufferSize(size_t sourceSize)
    {
        return sourceSize * 2 + 2;
    }

    enum
    {
        END = 0300,
        ESC = 0333,
        ESC_END = 0334,
        ESC_ESC = 0335
    };

};
