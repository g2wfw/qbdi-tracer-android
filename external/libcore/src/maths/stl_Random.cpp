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
#include "maths/stl_Random.h"
#include "time/stl_Time.h"

namespace stl
{

Random::Random (int64 seedValue) noexcept  : seed (seedValue)
{
}

Random::Random()  : seed (1)
{
    setSeedRandomly();
}

void Random::setSeed (const int64 newSeed) noexcept
{
    if (this == &getSystemRandom())
    {
        // Resetting the system Random risks messing up
        // STL's internal state. If you need a predictable
        // stream of random numbers you should use a local
        // Random object.
        jassertfalse;
        return;
    }

    seed = newSeed;
}

void Random::combineSeed (const int64 seedValue) noexcept
{
    seed ^= nextInt64() ^ seedValue;
}

void Random::setSeedRandomly()
{
    static std::atomic<int64> globalSeed { 0 };

    combineSeed (globalSeed ^ (int64) (pointer_sized_int) this);
    combineSeed (Time::getMillisecondCounter());
    combineSeed (Time::getHighResolutionTicks());
    combineSeed (Time::getHighResolutionTicksPerSecond());
    combineSeed (Time::currentTimeMillis());
    globalSeed ^= seed;
}

Random& Random::getSystemRandom() noexcept
{
    static Random sysRand;
    return sysRand;
}

//==============================================================================
int Random::nextInt() noexcept
{
    seed = (int64) (((((uint64) seed) * 0x5deece66dLL) + 11) & 0xffffffffffffLL);

    return (int) (seed >> 16);
}

int Random::nextInt (const int maxValue) noexcept
{
    jassert (maxValue > 0);
    return (int) ((((unsigned int) nextInt()) * (uint64) maxValue) >> 32);
}

int Random::nextInt (Range<int> range) noexcept
{
    return range.getStart() + nextInt (range.getLength());
}

int64 Random::nextInt64() noexcept
{
    return (int64) ((((uint64) (unsigned int) nextInt()) << 32) | (uint64) (unsigned int) nextInt());
}

bool Random::nextBool() noexcept
{
    return (nextInt() & 0x40000000) != 0;
}

float Random::nextFloat() noexcept
{
    auto result = static_cast<float> (static_cast<uint32> (nextInt()))
                  / (static_cast<float> (std::numeric_limits<uint32>::max()) + 1.0f);
    return jmin (result, 1.0f - std::numeric_limits<float>::epsilon());
}

double Random::nextDouble() noexcept
{
    return static_cast<uint32> (nextInt()) / (std::numeric_limits<uint32>::max() + 1.0);
}

BigInteger Random::nextLargeNumber (const BigInteger& maximumValue)
{
    BigInteger n;

    do
    {
        fillBitsRandomly (n, 0, maximumValue.getHighestBit() + 1);
    }
    while (n >= maximumValue);

    return n;
}

void Random::fillBitsRandomly (void* const buffer, size_t bytes)
{
    int* d = static_cast<int*> (buffer);

    for (; bytes >= sizeof (int); bytes -= sizeof (int))
        *d++ = nextInt();

    if (bytes > 0)
    {
        const int lastBytes = nextInt();
        memcpy (d, &lastBytes, bytes);
    }
}

void Random::fillBitsRandomly (BigInteger& arrayToChange, int startBit, int numBits)
{
    arrayToChange.setBit (startBit + numBits - 1, true);  // to force the array to pre-allocate space

    while ((startBit & 31) != 0 && numBits > 0)
    {
        arrayToChange.setBit (startBit++, nextBool());
        --numBits;
    }

    while (numBits >= 32)
    {
        arrayToChange.setBitRangeAsInt (startBit, 32, (unsigned int) nextInt());
        startBit += 32;
        numBits -= 32;
    }

    while (--numBits >= 0)
        arrayToChange.setBit (startBit + numBits, nextBool());
}



} // namespace stl
