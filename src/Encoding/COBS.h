// =============================================================================
//
// Copyright (c) 2010-2014 Christopher Baker <http://christopherbaker.net>
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


/// \brief A Consistent Overhead Byte Stuffing (COBS) Encoder.
///
/// Consistent Overhead Byte Stuffing (COBS) is an encoding that removes all 0
/// bytes from arbitrary binary data. The encoded data consists only of bytes
/// with values from 0x01 to 0xFF. This is useful for preparing data for
/// transmission over a serial link (RS-232 or RS-485 for example), as the 0
/// byte can be used to unambiguously indicate packet boundaries. COBS also has
/// the advantage of adding very little overhead (at least 1 byte, plus up to an
/// additional byte per 254 bytes of data). For messages smaller than 254 bytes,
/// the overhead is constant.
///
/// (via http://www.jacquesf.com/2011/03/consistent-overhead-byte-stuffing/)
///
/// \sa http://conferences.sigcomm.org/sigcomm/1997/papers/p062.pdf
/// \sa http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
/// \sa https://github.com/jacquesf/COBS-Consistent-Overhead-Byte-Stuffing
/// \sa http://www.jacquesf.com/2011/03/consistent-overhead-byte-stuffing/
class COBS 
{
public:
    /// \brief Encode a byte buffer with the COBS encoder.
    /// \param source The buffer to encode.
    /// \param size The size of the buffer to encode.
    /// \param destination The target buffer for the encoded bytes.
    /// \returns The number of bytes in the encoded buffer.
    /// \warning destination must have a minimum capacity of
    ///     (size + size / 254 + 1).
    static size_t encode(const uint8_t* source, size_t size, uint8_t* destination)
    {
        size_t read_index  = 0;
        size_t write_index = 1;
        size_t code_index  = 0;
        uint8_t code       = 1;

        while(read_index < size)
        {
            if(source[read_index] == 0)
            {
                destination[code_index] = code;
                code = 1;
                code_index = write_index++;
                read_index++;
            }
            else
            {
                destination[write_index++] = source[read_index++];
                code++;

                if(code == 0xFF)
                {
                    destination[code_index] = code;
                    code = 1;
                    code_index = write_index++;
                }
            }
        }

        destination[code_index] = code;

        return write_index;
    }

    /// \brief Decode a COBS-encoded buffer.
    /// \param source The COBS-encoded buffer to decode.
    /// \param size The size of the COBS-encoded buffer.
    /// \param destination The target buffer for the decoded bytes.
    /// \returns The number of bytes in the decoded buffer.
    /// \warning destination must have a minimum capacity of
    ///     size
    static size_t decode(const uint8_t* source, size_t size, uint8_t* destination)
    {
        size_t read_index  = 0;
        size_t write_index = 0;
        uint8_t code;
        uint8_t i;

        while(read_index < size)
        {
            code = source[read_index];

            if(read_index + code > size && code != 1)
            {
                return 0;
            }

            read_index++;

            for(i = 1; i < code; i++)
            {
                destination[write_index++] = source[read_index++];
            }

            if(code != 0xFF && read_index != size)
            {
                destination[write_index++] = '\0';
            }
        }
        
        return write_index;
    }

    /// \brief Get the maximum encoded buffer size needed for a given source size.
    /// \param sourceSize The size of the buffer to be encoded.
    /// \returns the maximum size of the required encoded buffer.
    static size_t getEncodedBufferSize(size_t sourceSize)
    {
        return sourceSize + sourceSize / 254 + 1;
    }

};
