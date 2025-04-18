#pragma once/*
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

#include "stl_InputStream.h"
#include "../maths/stl_MathsFunctions.h"
#include "../text/stl_String.h"
#include "../memory/stl_OptionalScopedPointer.h"
#include "../maths/stl_Range.h"
#include "../memory/stl_HeapBlock.h"
#include "../system/stl_PlatformDefs.h"

namespace stl {

//==============================================================================
/** Wraps another input stream, and reads from it using an intermediate buffer

    If you're using an input stream such as a file input stream, and making lots of
    small read accesses to it, it's probably sensible to wrap it in one of these,
    so that the source stream gets accessed in larger chunk sizes, meaning less
    work for the underlying stream.

    @tags{Core}
*/
    class STL_API BufferedInputStream : public InputStream {
    public:
        //==============================================================================
        /** Creates a BufferedInputStream from an input source.

            @param sourceStream                 the source stream to read from
            @param bufferSize                   the size of reservoir to use to buffer the source
            @param deleteSourceWhenDestroyed    whether the sourceStream that is passed in should be
                                                deleted by this object when it is itself deleted.
        */
        BufferedInputStream(InputStream *sourceStream,
                            int bufferSize,
                            bool deleteSourceWhenDestroyed);

        /** Creates a BufferedInputStream from an input source.

            @param sourceStream     the source stream to read from - the source stream  must not
                                    be deleted until this object has been destroyed.
            @param bufferSize       the size of reservoir to use to buffer the source
        */
        BufferedInputStream(InputStream &sourceStream, int bufferSize);

        /** Destructor.

            This may also delete the source stream, if that option was chosen when the
            buffered stream was created.
        */
        ~BufferedInputStream() override;


        //==============================================================================
        /** 返回调用readByte（）将读取的下一个字节 */
        char peekByte();

        int64 getTotalLength() override;

        int64 getPosition() override;

        bool setPosition(int64 newPosition) override;

        int read(void *destBuffer, int maxBytesToRead) override;

        String readString() override;

        bool isExhausted() override;


    private:
        //==============================================================================
        OptionalScopedPointer<InputStream> source;
        Range<int64> bufferedRange;
        int64 position, bufferLength, lastReadPos = 0, bufferOverlap = 128;
        HeapBlock<char> buffer;

        bool ensureBuffered();

        STL_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferedInputStream)
    };

} // namespace stl
