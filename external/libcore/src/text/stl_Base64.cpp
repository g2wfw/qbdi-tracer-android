/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/
#include "stl_core.h"
#include "text/stl_Base64.h"
#include "streams/stl_OutputStream.h"
#include "streams/stl_MemoryOutputStream.h"
#include "text/stl_ByteArray.h"

namespace stl {

    bool Base64::convertToBase64(OutputStream &base64Result, const void *sourceData, size_t sourceDataSize) {
        static const char lookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        auto *source = static_cast<const uint8 *> (sourceData);

        while (sourceDataSize > 0) {
            char frame[4];
            auto byte0 = *source++;
            frame[0] = lookup[(byte0 & 0xfcu) >> 2];
            uint32 bits = (byte0 & 0x03u) << 4;

            if (sourceDataSize > 1) {
                auto byte1 = *source++;
                frame[1] = lookup[bits | ((byte1 & 0xf0u) >> 4)];
                bits = (byte1 & 0x0fu) << 2;

                if (sourceDataSize > 2) {
                    auto byte2 = *source++;
                    frame[2] = lookup[bits | ((byte2 & 0xc0u) >> 6)];
                    frame[3] = lookup[byte2 & 0x3fu];
                    sourceDataSize -= 3;
                } else {
                    frame[2] = lookup[bits];
                    frame[3] = '=';
                    sourceDataSize = 0;
                }
            } else {
                frame[1] = lookup[bits];
                frame[2] = '=';
                frame[3] = '=';
                sourceDataSize = 0;
            }

            if (!base64Result.write(frame, 4))
                return false;
        }

        return true;
    }

    bool Base64::convertFromBase64(OutputStream &binaryOutput, StringRef base64TextInput) {
        for (auto s = base64TextInput.text; !s.isEmpty();) {
            uint8 data[4];

            for (int i = 0; i < 4; ++i) {
                auto c = (uint32) s.getAndAdvance();

                if (c >= 'A' && c <= 'Z') c -= 'A';
                else if (c >= 'a' && c <= 'z') c -= 'a' - 26;
                else if (c >= '0' && c <= '9') c += 52 - '0';
                else if (c == '+') c = 62;
                else if (c == '/') c = 63;
                else if (c == '=') {
                    c = 64;
                    if (i <= 1) return false;
                } else return false;

                data[i] = (uint8) c;
            }

            binaryOutput.writeByte((char) ((data[0] << 2) | (data[1] >> 4)));

            if (data[2] < 64) {
                binaryOutput.writeByte((char) ((data[1] << 4) | (data[2] >> 2)));

                if (data[3] < 64)
                    binaryOutput.writeByte((char) ((data[2] << 6) | data[3]));
            }
        }

        return true;
    }

    String Base64::toBase64(const void *sourceData, size_t sourceDataSize) {
        MemoryOutputStream m((sourceDataSize * 4) / 3 + 3);
        [[maybe_unused]] bool ok = convertToBase64(m, sourceData, sourceDataSize);
        jassert (ok); // should always succeed for this simple case
        return m.toString();
    }

    String Base64::toBase64(const String &text) {
        return toBase64(text.toRawUTF8(), strlen(text.toRawUTF8()));
    }

    ByteArray Base64::convertFromBase64(const String &textToEncode) {
        stl::MemoryOutputStream memoryOutputStream;
        if (!convertFromBase64(memoryOutputStream, textToEncode)) {
            return {};
        }
        ByteArray byteArray(memoryOutputStream.getData(), memoryOutputStream.getDataSize());
        return byteArray;
    }

} // namespace stl
