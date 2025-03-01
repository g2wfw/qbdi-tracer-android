/*
 Copyright 2014 Smartsheet.com, Inc.
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 
 Portions Copyright (c) 2008-2010 Björn Höhrmann <bjoern@hoehrmann.de>.
 See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 USE OR OTHER DEALINGS IN THE SOFTWARE.

*/


#ifndef HEADER_UTF_UTIL_INCLUDED
#define HEADER_UTF_UTIL_INCLUDED

namespace smjni
{
    template<typename InIt, typename Out>
    Out utf32_to_utf16(InIt first, InIt last, Out dest)
    {
        while(first != last)
        {
            uint32_t c = *first++;
            
            if (c > 0x010FFFF || (c > 0x00D7FF && c < 0x00E000))
            {
                *dest++ = u'\uFFFD';
            }
            else if (c >= 0x010000)
            {
                c -= 0x010000;
                *dest++ = char16_t((c >> 10)    + 0x00D800);
                *dest++ = char16_t((c & 0x03FF) + 0x00DC00);
            }
            else
            {
                *dest++ = char16_t(c);
            }
        }
        
        return dest;
    }
    
    template<typename InIt, typename OutIt>
    OutIt utf16_to_utf32(InIt first, InIt last, OutIt dest)
    {
        while(first != last)
        {
            uint32_t value = *first++;
        resync:
            if (value >= 0xD800 && value <= 0xDBFF)
            {
                if (first == last)
                {
                    *dest++ = U'\uFFFD';
                    return dest;
                }
                
                uint32_t trail = *first++;
                if (trail < 0xDC00 || trail > 0xDFFF)
                {
                    *dest++ = U'\uFFFD';
                    value = trail;
                    goto resync;
                }
                else
                {
                    value = char32_t(((value - 0xD800) << 10) + (trail - 0xDC00) + 0x0010000);
                }
            }
            else if (value >= 0xDC00 && value <= 0xDFFF)
            {
                value = U'\uFFFD';
            }
            *dest++ = char32_t(value);
        }
        return dest;
    }


    class utf8_codepoint_decoder
    {
    public:
        constexpr void put(uint32_t byte) noexcept
        {
            uint32_t type = s_state_table[byte];

            m_value = (m_state != state_accept) ?
                    (byte & 0x3fu) | (m_value << 6) :
                    (0xff >> type) & (byte);

            m_state = s_state_table[256 + m_state + type];
        }
        
        constexpr bool done() const noexcept
            { return m_state == state_accept; }
        
        constexpr bool error() const noexcept
            { return m_state == state_reject; }
        
        constexpr uint32_t value() const noexcept
            { return m_value; }
    private:
        uint32_t m_state = state_accept;
        uint32_t m_value = 0;
        
        static constexpr uint32_t state_accept = 0;
        static constexpr uint32_t state_reject = 12;
        
        static constexpr const uint8_t s_state_table[] = {
            // The first part of the table maps bytes to character classes that
            // to reduce the size of the transition table and create bitmasks.
             0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
             1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
             8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

            // The second part is a transition table that maps a combination
            // of a state of the automaton and a character class to a state.
             0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
            12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
            12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
            12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
            12,36,12,12,12,12,12,12,12,12,12,12,
        };
    };

    
    template<typename InIt, typename Out>
    Out utf8_to_utf16(InIt first, InIt last, Out dest)
    {
        if (first == last)
            return dest;
        
        uint8_t byte = *first++;
        
        for( ; ; )
        {
            uint32_t value;
            
            bool repeat_prev = false;
            if (byte <= 0x7f)
            {
                value = byte;
            }
            else
            {
                utf8_codepoint_decoder decoder;
                uint32_t first_byte = true;
                for ( ; ; )
                {
                    decoder.put(byte);
                    
                    if (decoder.done())
                    {
                        value = decoder.value();
                        break;
                    }
                    
                    if (decoder.error())
                    {
                        value = U'\uFFFD';
                        repeat_prev = !first_byte;
                        break;
                    }
                    if (first == last)
                    {
                        value = u'\uFFFD';
                        break;
                    }
                    
                    byte = *first++;
                    first_byte = false;
                }
            }
                
            if (value >= 0x010000)
            {
                value -= 0x010000;
                *dest++ = char16_t((value >> 10)    + 0x00D800);
                *dest++ = char16_t((value & 0x03FF) + 0x00DC00);
            }
            else
            {
                *dest++ = char16_t(value);
            }
            
            if (repeat_prev)
                continue;
            
            if (first == last)
                break;
                
            byte = *first++;
        }
        
        return dest;
    }
    
    template<typename InIt, typename OutIt>
    OutIt utf16_to_utf8(InIt first, InIt last, OutIt dest)
    {
        while(first != last)
        {
            uint32_t value = *first++;
        resync:
            if (value >= 0xD800 && value <= 0xDBFF)
            {
                if (first == last)
                {
                    *dest++ = static_cast<uint8_t>('\xef');
                    *dest++ = static_cast<uint8_t>('\xbf');
                    *dest++ = static_cast<uint8_t>('\xbd');
                    return dest;
                }
                
                
                uint32_t trail = *first++;
                if (trail < 0xDC00 || trail > 0xDFFF)
                {
                    *dest++ = static_cast<uint8_t>('\xef');
                    *dest++ = static_cast<uint8_t>('\xbf');
                    *dest++ = static_cast<uint8_t>('\xbd');
                    value = trail;
                    goto resync;
                }
                else
                {
                    value = char32_t(((value - 0xD800) << 10) + (trail - 0xDC00) + 0x0010000);
                }
            }
            else if (value >= 0xDC00 && value <= 0xDFFF)
            {
                *dest++ = static_cast<uint8_t>('\xef');
                *dest++ = static_cast<uint8_t>('\xbf');
                *dest++ = static_cast<uint8_t>('\xbd');
                continue;
            }

            if (value <= 0x00007f)
            {
                *dest++ = static_cast<uint8_t>(value);
            }
            else if (value <= 0x0007FF)
            {
                *dest++ = static_cast<uint8_t>(0b11000000u | ((value >> 6) & 0b00011111u));
                *dest++ = static_cast<uint8_t>(0b10000000u | (value        & 0b00111111u));
            }
            else if (value <= 0x00FFFF)
            {
                *dest++ = static_cast<uint8_t>(0b11100000u | ((value >> 12) & 0b00001111u));
                *dest++ = static_cast<uint8_t>(0b10000000u | ((value >> 6)  & 0b00111111u));
                *dest++ = static_cast<uint8_t>(0b10000000u | (value         & 0b00111111u));
            }
            else
            {
                *dest++ = static_cast<uint8_t>(0b11110000u | ((value >> 18) & 0b00000111u));
                *dest++ = static_cast<uint8_t>(0b10000000u | ((value >> 12) & 0b00111111u));
                *dest++ = static_cast<uint8_t>(0b10000000u | ((value >> 6)  & 0b00111111u));
                *dest++ = static_cast<uint8_t>(0b10000000u | (value         & 0b00111111u));
            }
        }
        return dest;
    }
}


#endif //HEADER_UTF_UTIL_INCLUDED


