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
#include "files/stl_FileOutputStream.h"
#include "stl_bit_utils.h"

#ifdef STL_MINGW
#include <windows.h>
#else

#include <sys/stat.h>

#endif
namespace stl {
    int64 stl_fileSetPosition(void *handle, int64 pos);

//==============================================================================
    FileOutputStream::FileOutputStream(const File &f, const size_t bufferSizeToUse)
            : file(f),
              bufferSize(bufferSizeToUse),
              buffer(jmax(bufferSizeToUse, (size_t) 16)) {
        openHandle();
    }

    FileOutputStream::~FileOutputStream() {
        flushBuffer();
        closeHandle();
    }

    int64 FileOutputStream::getPosition() {
        return currentPosition;
    }

    bool FileOutputStream::setPosition(int64 newPosition) {
        if (newPosition != currentPosition) {
            flushBuffer();
            currentPosition = stl_fileSetPosition(fileHandle, newPosition);
        }

        return newPosition == currentPosition;
    }

    bool FileOutputStream::flushBuffer() {
        bool ok = true;

        if (bytesInBuffer > 0) {
            ok = (writeInternal(buffer, bytesInBuffer) == (ssize_t) bytesInBuffer);
            bytesInBuffer = 0;
        }

        return ok;
    }

    void FileOutputStream::flush() {
        flushBuffer();
        flushInternal();
    }

    bool FileOutputStream::write(const void *const src, const size_t numBytes) {
        jassert (src != nullptr && ((ssize_t) numBytes) >= 0);

        if (!openedOk())
            return false;

        if (bytesInBuffer + numBytes < bufferSize) {
            memcpy(buffer + bytesInBuffer, src, numBytes);
            bytesInBuffer += numBytes;
            currentPosition += (int64) numBytes;
        } else {
            if (!flushBuffer())
                return false;

            if (numBytes < bufferSize) {
                memcpy(buffer + bytesInBuffer, src, numBytes);
                bytesInBuffer += numBytes;
                currentPosition += (int64) numBytes;
            } else {
                auto bytesWritten = writeInternal(src, numBytes);

                if (bytesWritten < 0)
                    return false;

                currentPosition += (int64) bytesWritten;
                return bytesWritten == (ssize_t) numBytes;
            }
        }

        return true;
    }

    bool FileOutputStream::writeRepeatedByte(uint8 byte, size_t numBytes) {
        jassert (((ssize_t) numBytes) >= 0);

        if (bytesInBuffer + numBytes < bufferSize) {
            memset(buffer + bytesInBuffer, byte, numBytes);
            bytesInBuffer += numBytes;
            currentPosition += (int64) numBytes;
            return true;
        }

        return OutputStream::writeRepeatedByte(byte, numBytes);
    }

    FileOutputStream::FileOutputStream(const std::string fileToWriteTo, size_t bufferSizeToUse) : file(fileToWriteTo),
                                                                                                  bufferSize(
                                                                                                          bufferSizeToUse),
                                                                                                  buffer(jmax(
                                                                                                          bufferSizeToUse,
                                                                                                          (size_t) 16)) {
        openHandle();
    }

    void FileOutputStream::alignTo(const size_t alignment) {
        flush();
        size_t file_length = 0;
#ifdef STL_MINGW
        DWORD fileSizeLow, fileSizeHigh;
        fileSizeLow = GetFileSize(fileHandle, &fileSizeHigh);
        file_length  = ((unsigned long long)fileSizeHigh << 32) | fileSizeLow;;
#else
        struct stat fileStat;
        fstat((int) (pointer_sized_int) fileHandle, &fileStat);
        file_length = fileStat.st_size;
#endif
        auto real_size = RoundUp(file_length, alignment);
        if (real_size - file_length > 0) {
            auto buffer_length = real_size - file_length;
            for (int i = 0; i < buffer_length; ++i) {
                writeByte('\0');
            }
            flush();
        }
    }

} // namespace stl
