#pragma once/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using STL, you agree to the terms of both the STL 6 End-User License
   Agreement and STL Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

namespace stl
{

/** @internal */
struct MimeTypeTable
{
/** @internal */
static void registerCustomMimeTypeForFileExtension (const String& mimeType, const String& fileExtension);

/** @internal */
static StringArray getMimeTypesForFileExtension (const String& fileExtension);

/** @internal */
static StringArray getFileExtensionsForMimeType (const String& mimeType);
};

} // namespace stl
