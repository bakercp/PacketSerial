// =============================================================================
//
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


#include "COBSBuffer.h"


//------------------------------------------------------------------------------
COBSBuffer::COBSBuffer(size_t size) : 
    _data(NULL), 
    _size(size), 
    _index(0)
{
    _data = new uint8_t[_size];
}

//------------------------------------------------------------------------------
COBSBuffer::~COBSBuffer() 
{
    delete[] _data;
    _data   = NULL;
    _size   = 0;
    _index  = 0;
}

//------------------------------------------------------------------------------
size_t COBSBuffer::write(uint8_t inByte)
{
    if(_index < _size) 
    {
        _data[_index] = inByte;
        _index++;
        return 1;
    } else {
        // buffer overrun
        _index = 0;
        return 0;
    }
}

//------------------------------------------------------------------------------
size_t COBSBuffer::write(const uint8_t* buffer, size_t size)
{
    size_t n = 0;
    while (size--) 
    {
        size_t numBytes = write(*buffer++);
        if(numBytes == 0)
        {
            return 0;
        } 
        else
        {
            n += numBytes; 
        }
    }
    
    return n;
}

//------------------------------------------------------------------------------
uint8_t* COBSBuffer::data()
{
    return _data;
}

//------------------------------------------------------------------------------
size_t COBSBuffer::size() const 
{
    return _size;
}

//------------------------------------------------------------------------------
size_t COBSBuffer::index() const
{
    return _index;
}

//------------------------------------------------------------------------------
bool COBSBuffer::empty() const
{
    return _index == 0;
}

//------------------------------------------------------------------------------
void COBSBuffer::clear() 
{
    _index = 0;
}
