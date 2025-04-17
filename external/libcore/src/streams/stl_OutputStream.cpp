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
#include "streams/stl_OutputStream.h"
#include "streams/stl_InputStream.h"
#include "text/stl_String.h"
#include "memory/stl_MemoryBlock.h"
#include "files/stl_FileInputStream.h"

namespace stl {

#if STL_DEBUG

    //==============================================================================
    struct DanglingStreamChecker
    {
        DanglingStreamChecker() = default;

        ~DanglingStreamChecker()
        {
            /*
                It's always a bad idea to leak any object, but if you're leaking output
                streams, then there's a good chance that you're failing to flush a file
                to disk properly, which could result in corrupted data and other similar
                nastiness..
            */
            jassert (activeStreams.size() == 0);

            // We need to flag when this helper struct has been destroyed to prevent some
            // nasty order-of-static-destruction issues
            hasBeenDestroyed = true;
        }

        Array<void*, CriticalSection> activeStreams;

        static bool hasBeenDestroyed;
    };

    bool DanglingStreamChecker::hasBeenDestroyed = false;
    static DanglingStreamChecker danglingStreamChecker;

#endif

//==============================================================================
    OutputStream::OutputStream()
            : newLineString(NewLine::getDefault()) {
#if STL_DEBUG
        if (! DanglingStreamChecker::hasBeenDestroyed)
            danglingStreamChecker.activeStreams.add (this);
#endif
    }

    OutputStream::~OutputStream() {
#if STL_DEBUG
        if (! DanglingStreamChecker::hasBeenDestroyed)
            danglingStreamChecker.activeStreams.removeFirstMatchingValue (this);
#endif
    }

//==============================================================================
    bool OutputStream::writeBool(bool b) {
        return writeByte(b ? (char) 1
                           : (char) 0);
    }

    bool OutputStream::writeByte(char byte) {
        return write(&byte, 1);
    }

    bool OutputStream::writeRepeatedByte(uint8 byte, size_t numTimesToRepeat) {
        for (size_t i = 0; i < numTimesToRepeat; ++i)
            if (!writeByte((char) byte))
                return false;

        return true;
    }

    bool OutputStream::writeShort(short value) {
        auto v = ByteOrder::swapIfBigEndian((uint16) value);
        return write(&v, 2);
    }

    bool OutputStream::writeShortBigEndian(short value) {
        auto v = ByteOrder::swapIfLittleEndian((uint16) value);
        return write(&v, 2);
    }

    bool OutputStream::writeInt(int value) {
        auto v = ByteOrder::swapIfBigEndian((uint32) value);
        return write(&v, 4);
    }

    bool OutputStream::writeIntBigEndian(int value) {
        auto v = ByteOrder::swapIfLittleEndian((uint32) value);
        return write(&v, 4);
    }

    bool OutputStream::writeCompressedInt(int value) {
        auto un = (value < 0) ? (unsigned int) -value
                              : (unsigned int) value;

        uint8 data[5];
        int num = 0;

        while (un > 0) {
            data[++num] = (uint8) un;
            un >>= 8;
        }

        data[0] = (uint8) num;

        if (value < 0)
            data[0] |= 0x80;

        return write(data, (size_t) num + 1);
    }

    bool OutputStream::writeInt64(int64 value) {
        auto v = ByteOrder::swapIfBigEndian((uint64) value);
        return write(&v, 8);
    }

    bool OutputStream::writeInt64BigEndian(int64 value) {
        auto v = ByteOrder::swapIfLittleEndian((uint64) value);
        return write(&v, 8);
    }

    bool OutputStream::writeFloat(float value) {
        union {
            int asInt;
            float asFloat;
        } n;
        n.asFloat = value;
        return writeInt(n.asInt);
    }

    bool OutputStream::writeFloatBigEndian(float value) {
        union {
            int asInt;
            float asFloat;
        } n;
        n.asFloat = value;
        return writeIntBigEndian(n.asInt);
    }

    bool OutputStream::writeDouble(double value) {
        union {
            int64 asInt;
            double asDouble;
        } n;
        n.asDouble = value;
        return writeInt64(n.asInt);
    }

    bool OutputStream::writeDoubleBigEndian(double value) {
        union {
            int64 asInt;
            double asDouble;
        } n;
        n.asDouble = value;
        return writeInt64BigEndian(n.asInt);
    }

    bool OutputStream::writeString(const String &text) {
        auto numBytes = text.getNumBytesAsUTF8() + 1;

#if (STL_STRING_UTF_TYPE == 8)
        return write(text.toRawUTF8(), numBytes);
#else
        // (This avoids using toUTF8() to prevent the memory bloat that it would leave behind
        // if lots of large, persistent strings were to be written to streams).
        HeapBlock<char> temp (numBytes);
        text.copyToUTF8 (temp, numBytes);
        return write (temp, numBytes);
#endif
    }

    bool OutputStream::writeText(const String &text) {
        return writeText(text, false, false, "");
    }

    bool OutputStream::writeText(const String &text, bool asUTF16, bool writeUTF16ByteOrderMark, const char *lf) {
        bool replaceLineFeedWithUnix = lf != nullptr && lf[0] == '\n' && lf[1] == 0;
        bool replaceLineFeedWithWindows = lf != nullptr && lf[0] == '\r' && lf[1] == '\n' && lf[2] == 0;

        // The line-feed passed in must be either nullptr, or "\n" or "\r\n"
        jassert (lf == nullptr || replaceLineFeedWithWindows || replaceLineFeedWithUnix);

        if (asUTF16) {
            if (writeUTF16ByteOrderMark)
                write("\x0ff\x0fe", 2);

            auto src = text.getCharPointer();
            bool lastCharWasReturn = false;

            for (;;) {
                auto c = src.getAndAdvance();

                if (c == 0)
                    break;

                if (replaceLineFeedWithWindows) {
                    if (c == '\n' && !lastCharWasReturn)
                        writeShort((short) '\r');

                    lastCharWasReturn = (c == L'\r');
                } else if (replaceLineFeedWithUnix && c == '\r') {
                    continue;
                }

                if (!writeShort((short) c))
                    return false;
            }
        } else {
            const char *src = text.toRawUTF8();

            if (replaceLineFeedWithWindows) {
                for (auto t = src;;) {
                    if (*t == '\n') {
                        if (t > src)
                            if (!write(src, (size_t) (t - src)))
                                return false;

                        if (!write("\r\n", 2))
                            return false;

                        src = t + 1;
                    } else if (*t == '\r') {
                        if (t[1] == '\n')
                            ++t;
                    } else if (*t == 0) {
                        if (t > src)
                            if (!write(src, (size_t) (t - src)))
                                return false;

                        break;
                    }

                    ++t;
                }
            } else if (replaceLineFeedWithUnix) {
                for (;;) {
                    auto c = *src++;

                    if (c == 0)
                        break;

                    if (c != '\r')
                        if (!writeByte(c))
                            return false;
                }
            } else {
                return write(src, text.getNumBytesAsUTF8());
            }
        }

        return true;
    }

    int64 OutputStream::writeFromInputStream(InputStream &source, int64 numBytesToWrite) {
        if (numBytesToWrite < 0)
            numBytesToWrite = std::numeric_limits<int64>::max();

        int64 numWritten = 0;

        while (numBytesToWrite > 0) {
            char buffer[8192];
            auto num = source.read(buffer, (int) jmin(numBytesToWrite, (int64) sizeof(buffer)));

            if (num <= 0)
                break;

            write(buffer, (size_t) num);

            numBytesToWrite -= num;
            numWritten += num;
        }

        return numWritten;
    }

//==============================================================================
    void OutputStream::setNewLineString(const String &newLineStringToUse) {
        newLineString = newLineStringToUse;
    }

    template<typename Type>
    static char *printDigits(char *t, Type v) noexcept {
        *--t = 0;

        do {
            *--t = static_cast<char> ('0' + (char) (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    // pass in a pointer to the END of a buffer..
    static char *numberToString(char *t, int64 n) noexcept {
        if (n >= 0)
            return printDigits(t, static_cast<uint64> (n));

        // NB: this needs to be careful not to call -std::numeric_limits<int64>::min(),
        // which has undefined behaviour
        t = printDigits(t, static_cast<uint64> (-(n + 1)) + 1);
        *--t = '-';
        return t;
    }

    static char *numberToString(char *t, uint64 v) noexcept {
        return printDigits(t, v);
    }

    static char *numberToString(char *t, int n) noexcept {
        if (n >= 0)
            return printDigits(t, static_cast<unsigned int> (n));

        // NB: this needs to be careful not to call -std::numeric_limits<int>::min(),
        // which has undefined behaviour
        t = printDigits(t, static_cast<unsigned int> (-(n + 1)) + 1);
        *--t = '-';
        return t;
    }

    static char *numberToString(char *t, unsigned int v) noexcept {
        return printDigits(t, v);
    }

    static char *numberToString(char *t, long n) noexcept {
        if (n >= 0)
            return printDigits(t, static_cast<unsigned long> (n));

        t = printDigits(t, static_cast<unsigned long> (-(n + 1)) + 1);
        *--t = '-';
        return t;
    }

    static char *numberToString(char *t, unsigned long v) noexcept {
        return printDigits(t, v);
    }

//==============================================================================
    template<typename IntegerType>
    static void writeIntToStream(OutputStream &stream, IntegerType number) {
        char buffer[32];
        char *end = buffer + numElementsInArray(buffer);
        const char *start = numberToString(end, number);
        stream.write(start, (size_t) (end - start - 1));
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const int number) {
        writeIntToStream(stream, number);
        return stream;
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const int64 number) {
        writeIntToStream(stream, number);
        return stream;
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const double number) {
        return stream << String(number);
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const char character) {
        stream.writeByte(character);
        return stream;
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const char *const text) {
        stream.write(text, strlen(text));
        return stream;
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const MemoryBlock &data) {
        if (!data.isEmpty())
            stream.write(data.getData(), data.getSize());

        return stream;
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const File &fileToRead) {
        FileInputStream in(fileToRead);

        if (in.openedOk())
            return stream << in;

        return stream;
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, InputStream &streamToRead) {
        stream.writeFromInputStream(streamToRead, -1);
        return stream;
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const NewLine &) {
        return stream << stream.getNewLineString();
    }

} // namespace stl
