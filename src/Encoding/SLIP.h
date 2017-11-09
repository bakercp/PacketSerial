//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
// Copyright (c) 2016 Antoine Villeret
//
// SPDX-License-Identifier:	MIT
//


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
    /// \brief Encode the given buffer.
    /// \param buffer A pointer to the buffer to be encoded.
    /// \param size The size of the buffer pointed to by \p buffer.
    /// \param encoded A pointer to the target encoded buffer.  This buffer must have sufficient memory allocated.
    /// \returns The number of bytes written to the encoded \p buffer.
    /// \warning The encoded buffer must have at least getEncodedBufferSize() allocated.
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

        return write_index;
    }

    /// \brief Decode a SLIP-encoded buffer.
    /// \param source The SLIP-encoded buffer to decode.
    /// \param size The size of the SLIP-encoded buffer.
    /// \param destination The target buffer for the decoded bytes.
    /// \returns The number of bytes in the decoded buffer.
    /// \warning destination must have a minimum capacity of size.
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

    /// \brief Key constants used in the SLIP protocol.
    enum
    {
        END = 0300,
        ESC = 0333,
        ESC_END = 0334,
        ESC_ESC = 0335
    };

};
