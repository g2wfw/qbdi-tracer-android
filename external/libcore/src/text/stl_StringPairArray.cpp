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
#include "text/stl_StringPairArray.h"
#include <map>
#include <unordered_map>
#include "text/stl_String.h"



namespace stl
{

StringPairArray::StringPairArray (bool shouldIgnoreCase)  : ignoreCase (shouldIgnoreCase)
{
}

StringPairArray::StringPairArray (const StringPairArray& other)
    : keys (other.keys),
      values (other.values),
      ignoreCase (other.ignoreCase)
{
}

StringPairArray& StringPairArray::operator= (const StringPairArray& other)
{
    keys = other.keys;
    values = other.values;
    return *this;
}

bool StringPairArray::operator== (const StringPairArray& other) const
{
    auto num = size();

    if (num != other.size())
        return false;

    for (int i = 0; i < num; ++i)
    {
        if (keys[i] == other.keys[i]) // optimise for the case where the keys are in the same order
        {
            if (values[i] != other.values[i])
                return false;
        }
        else
        {
            // if we encounter keys that are in a different order, search remaining items by brute force..
            for (int j = i; j < num; ++j)
            {
                auto otherIndex = other.keys.indexOf (keys[j], other.ignoreCase);

                if (otherIndex < 0 || values[j] != other.values[otherIndex])
                    return false;
            }

            return true;
        }
    }

    return true;
}

bool StringPairArray::operator!= (const StringPairArray& other) const
{
    return ! operator== (other);
}

const String& StringPairArray::operator[] (StringRef key) const
{
    return values[keys.indexOf (key, ignoreCase)];
}

String StringPairArray::getValue (StringRef key, const String& defaultReturnValue) const
{
    auto i = keys.indexOf (key, ignoreCase);

    if (i >= 0)
        return values[i];

    return defaultReturnValue;
}

bool StringPairArray::containsKey (StringRef key) const noexcept
{
    return keys.contains (key, ignoreCase);
}

void StringPairArray::set (const String& key, const String& value)
{
    auto i = keys.indexOf (key, ignoreCase);

    if (i >= 0)
    {
        values.set (i, value);
    }
    else
    {
        keys.add (key);
        values.add (value);
    }
}

void StringPairArray::addArray (const StringPairArray& other)
{
    for (int i = 0; i < other.size(); ++i)
        set (other.keys[i], other.values[i]);
}

void StringPairArray::clear()
{
    keys.clear();
    values.clear();
}

void StringPairArray::remove (StringRef key)
{
    remove (keys.indexOf (key, ignoreCase));
}

void StringPairArray::remove (int index)
{
    keys.remove (index);
    values.remove (index);
}

void StringPairArray::setIgnoresCase (bool shouldIgnoreCase)
{
    ignoreCase = shouldIgnoreCase;
}

bool StringPairArray::getIgnoresCase() const noexcept
{
    return ignoreCase;
}

String StringPairArray::getDescription() const
{
    String s;

    for (int i = 0; i < keys.size(); ++i)
    {
        s << keys[i] << " = " << values[i];

        if (i < keys.size())
            s << ", ";
    }

    return s;
}

void StringPairArray::minimiseStorageOverheads()
{
    keys.minimiseStorageOverheads();
    values.minimiseStorageOverheads();
}

template <typename Map>
void StringPairArray::addMapImpl (const Map& toAdd)
{
    // If we just called `set` for each item in `toAdd`, that would
    // perform badly when adding to large StringPairArrays, as `set`
    // has to loop through the whole container looking for matching keys.
    // Instead, we use a temporary map to give us better lookup performance.
    std::map<String, int> contents;

    const auto normaliseKey = [this] (const String& key)
    {
        return ignoreCase ? key.toLowerCase() : key;
    };

    for (auto i = 0; i != size(); ++i)
        contents.emplace (normaliseKey (getAllKeys().getReference (i)), i);

    for (const auto& pair : toAdd)
    {
        const auto key = normaliseKey (pair.first);
        const auto it = contents.find (key);

        if (it != contents.cend())
        {
            values.getReference (it->second) = pair.second;
        }
        else
        {
            contents.emplace (key, static_cast<int> (contents.size()));
            keys.add (pair.first);
            values.add (pair.second);
        }
    }
}

void StringPairArray::addUnorderedMap (const std::unordered_map<String, String>& toAdd) { addMapImpl (toAdd); }
void StringPairArray::addMap (const std::map<String, String>& toAdd)                    { addMapImpl (toAdd); }


} // namespace stl
