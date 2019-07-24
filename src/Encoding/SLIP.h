//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
// Copyright (c) 2016 Antoine Villeret
//
// SPDX-License-Identifier: MIT
//


#pragma once


#include "Arduino.h"


/// \brief A Serial Line Internet Protocol (SLIP) Encoder.
///
/// Serial Line Internet Protocol (SLIP) is a packet framing protocol: SLIP 
/// defines a sequence of characters that frame IP packets on a serial line and 
/// nothing more. It provides no addressing, packet type identification, error 
/// detection, correction or compression mechanisms. Because the protocol does 
/// so little its implementation is trivial and fast.
///
/// \sa http://tools.ietf.org/html/rfc1055
class SLIP
{
public:
    /// \brief Encode a byte buffer with the SLIP encoder.
    /// \param buffer A pointer to the unencoded buffer to encode.
    /// \param size  The number of bytes in the \p buffer.
    /// \param encodedBuffer The buffer for the encoded bytes.
    /// \returns The number of bytes written to the \p encodedBuffer.
    /// \warning The encodedBuffer must have at least getEncodedBufferSize() 
    ///          allocated.
    static size_t encode(const uint8_t* buffer,
                         size_t size,
                         uint8_t* encodedBuffer)
    {
        if (size == 0)
            return 0;

        size_t read_index  = 0;
        size_t write_index = 0;

        // Double-ENDed, flush any data that may have accumulated due to line 
        // noise.
        encodedBuffer[write_index++] = END;

        while (read_index < size)
        {
            if(buffer[read_index] == END)
            {
                encodedBuffer[write_index++] = ESC;
                encodedBuffer[write_index++] = ESC_END;
                read_index++;
            }
            else if(buffer[read_index] == ESC)
            {
                encodedBuffer[write_index++] = ESC;
                encodedBuffer[write_index++] = ESC_ESC;
                read_index++;
            }
            else
            {
                encodedBuffer[write_index++] = buffer[read_index++];
            }
        }

        return write_index;
    }

    /// \brief Decode a SLIP-encoded buffer.
    /// \param encodedBuffer A pointer to the \p encodedBuffer to decode.
    /// \param size The number of bytes in the \p encodedBuffer.
    /// \param decodedBuffer The target buffer for the decoded bytes.
    /// \returns The number of bytes written to the \p decodedBuffer.
    /// \warning decodedBuffer must have a minimum capacity of size.
    static size_t decode(const uint8_t* encodedBuffer,
                         size_t size,
                         uint8_t* decodedBuffer)
    {
        if (size == 0)
            return 0;

        size_t read_index  = 0;
        size_t write_index = 0;

        while (read_index < size)
        {
            if (encodedBuffer[read_index] == END)
            {
                // flush or done
                read_index++;
            }
            else if (encodedBuffer[read_index] == ESC)
            {
                if (encodedBuffer[read_index+1] == ESC_END)
                {
                    decodedBuffer[write_index++] = END;
                    read_index += 2;
                }
                else if (encodedBuffer[read_index+1] == ESC_ESC)
                {
                    decodedBuffer[write_index++] = ESC;
                    read_index += 2;
                }
                else
                {
                    // This case is considered a protocol violation.
                }
            }
            else
            {
                decodedBuffer[write_index++] = encodedBuffer[read_index++];
            }
        }

        return write_index;
    }

    /// \brief Get the maximum encoded buffer size for an unencoded buffer size.
    ///
    /// SLIP has a start and end markers (192 and 219). Marker value is
    /// replaced by 2 bytes in the encoded buffer. So in the worst case of
    /// sending a buffer with only '192' or '219', the encoded buffer length
    /// will be 2 * buffer.size() + 2.
    ///
    /// \param unencodedBufferSize The size of the buffer to be encoded.
    /// \returns the maximum size of the required encoded buffer.
    static size_t getEncodedBufferSize(size_t unencodedBufferSize)
    {
        return unencodedBufferSize * 2 + 2;
    }

    /// \brief Key constants used in the SLIP protocol.
    enum
    {
        /// \brief The decimal END character (octal 0300).
        ///
        /// Indicates the end of a packet.
        END = 192, 

        /// \brief The decimal ESC character (octal 0333).
        ///
        /// Indicates byte stuffing.
        ESC = 219,

        /// \brief The decimal ESC_END character (octal 0334).
        ///
        /// ESC ESC_END means END data byte.
        ESC_END = 220,

        /// \brief The decimal ESC_ESC character (ocatal 0335).
        ///
        /// ESC ESC_ESC means ESC data byte.
        ESC_ESC = 221
    };

};
