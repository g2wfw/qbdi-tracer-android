/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025  g2wfw
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "hex_dump.h"
static constexpr size_t kBitsPerByte = 8;
static constexpr size_t kBitsPerByteLog2 = 3;
static constexpr int kBitsPerIntPtrT = sizeof(intptr_t) * kBitsPerByte;

void HexDump::Dump(std::ostream& os) const {
    if (byte_count_ == 0) {
        return;
    }
    if (address_ == nullptr) {
        os << "00000000:";
        return;
    }
    static const char gHexDigit[] = "0123456789abcdef";
    const unsigned char* addr = reinterpret_cast<const unsigned char*>(address_);
    // 01234560: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff  0123456789abcdef
    char out[(kBitsPerIntPtrT / 4) + /* offset */
             1 + /* colon */
             (16 * 3) + /* 16 hex digits and space */
             2 + /* white space */
             16 + /* 16 characters*/
             1 /* \0 */];
    size_t offset; /* offset to show while printing */
    offset = reinterpret_cast<size_t>(addr);

    memset(out, ' ', sizeof(out) - 1);
    out[kBitsPerIntPtrT / 4] = ':';
    out[sizeof(out) - 1] = '\0';

    size_t byte_count = byte_count_;
    size_t gap = offset & 0x0f;
    while (byte_count > 0) {
        size_t line_offset = offset & ~0x0f;

        char* hex = out;
        char* asc = out + (kBitsPerIntPtrT / 4) + /* offset */ 1 + /* colon */
                    (16 * 3) + /* 16 hex digits and space */ 2 /* white space */;

        for (int i = 0; i < (kBitsPerIntPtrT / 4); i++) {
            *hex++ = gHexDigit[line_offset >> (kBitsPerIntPtrT - 4)];
            line_offset <<= 4;
        }
        hex++;
        hex++;

        size_t count = std::min(byte_count, 16 - gap);
        // CHECK_NE(count, 0U);
        // CHECK_LE(count + gap, 16U);

        if (gap) {
            /* only on first line */
            hex += gap * 3;
            asc += gap;
        }

        size_t i;
        for (i = gap; i < count + gap; i++) {
            *hex++ = gHexDigit[*addr >> 4];
            *hex++ = gHexDigit[*addr & 0x0f];
            hex++;
            if (*addr >= 0x20 && *addr < 0x7f /*isprint(*addr)*/) {
                *asc++ = *addr;
            } else {
                *asc++ = '.';
            }
            addr++;
        }
        for (; i < 16; i++) {
            /* erase extra stuff; only happens on last line */
            *hex++ = ' ';
            *hex++ = ' ';
            hex++;
            *asc++ = ' ';
        }
        os << out;
        gap = 0;
        byte_count -= count;
        offset += count;
        if (byte_count > 0) {
            os << "\n";
        }
    }
}