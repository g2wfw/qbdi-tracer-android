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
#include "../stl_core.h"
namespace stl
{

//==============================================================================
/**
将指向以null结尾的ASCII字符串的指针换行，并提供
对数据进行操作的各种方法。
假定有效的ASCII字符串不包含127以上的任何字符。
@请参阅CharPointer_UTF8、CharPointer_UTP16、CharPointer_UTF32
@标签｛Core｝
*/
class STL_EXPORT CharPointer_ASCII  final
{
public:
    using CharType = char;

    inline explicit CharPointer_ASCII (const CharType* rawPointer) noexcept
        : data (const_cast<CharType*> (rawPointer))
    {
    }

    inline CharPointer_ASCII (const CharPointer_ASCII& other) = default;

    inline CharPointer_ASCII operator= (const CharPointer_ASCII other) noexcept
    {
        data = other.data;
        return *this;
    }

    inline CharPointer_ASCII operator= (const CharType* text) noexcept
    {
        data = const_cast<CharType*> (text);
        return *this;
    }

    /** 这是一个指针比较，它不比较实际文本。 */
    inline bool operator== (CharPointer_ASCII other) const noexcept     { return data == other.data; }
    inline bool operator!= (CharPointer_ASCII other) const noexcept     { return data != other.data; }
    inline bool operator<= (CharPointer_ASCII other) const noexcept     { return data <= other.data; }
    inline bool operator<  (CharPointer_ASCII other) const noexcept     { return data <  other.data; }
    inline bool operator>= (CharPointer_ASCII other) const noexcept     { return data >= other.data; }
    inline bool operator>  (CharPointer_ASCII other) const noexcept     { return data >  other.data; }

    /** 返回此指针所指向的地址。 */
    inline CharType* getAddress() const noexcept        { return data; }

    /**返回此指针所指向的地址。 */
    inline operator const CharType*() const noexcept    { return data; }

    /** 如果此指针指向空字符，则返回true。 */
    inline bool isEmpty() const noexcept                { return *data == 0; }

    /** 如果此指针未指向null字符，则返回true。 */
    inline bool isNotEmpty() const noexcept             { return *data != 0; }

    /** 返回此指针所指向的unicode字符。 */
    inline stl_wchar operator*() const noexcept        { return (stl_wchar) (uint8) *data; }

    /** 将此指针移动到字符串中的下一个字符。 */
    inline CharPointer_ASCII operator++() noexcept
    {
        ++data;
        return *this;
    }

    /** 将此指针移动到字符串中的上一个字符。 */
    inline CharPointer_ASCII operator--() noexcept
    {
        --data;
        return *this;
    }

    /** 返回此指针当前指向的字符，然后使指针前进到下一个字符。*/
    inline stl_wchar getAndAdvance() noexcept  { return (stl_wchar) (uint8) *data++; }

    /** 将此指针移动到字符串中的下一个字符。 */
    CharPointer_ASCII operator++ (int) noexcept
    {
        auto temp (*this);
        ++data;
        return temp;
    }

    /** 将此指针向前移动指定的字符数。 */
    inline void operator+= (const int numToSkip) noexcept
    {
        data += numToSkip;
    }

    inline void operator-= (const int numToSkip) noexcept
    {
        data -= numToSkip;
    }

    /** Returns the character at a given character index from the start of the string. */
    inline stl_wchar operator[] (const int characterIndex) const noexcept
    {
        return (stl_wchar) (uint8) data [characterIndex];
    }

    /** Returns a pointer which is moved forwards from this one by the specified number of characters. */
    CharPointer_ASCII operator+ (const int numToSkip) const noexcept
    {
        return CharPointer_ASCII (data + numToSkip);
    }

    /** Returns a pointer which is moved backwards from this one by the specified number of characters. */
    CharPointer_ASCII operator- (const int numToSkip) const noexcept
    {
        return CharPointer_ASCII (data - numToSkip);
    }

    /** Writes a unicode character to this string, and advances this pointer to point to the next position. */
    inline void write (const stl_wchar charToWrite) noexcept
    {
        *data++ = (char) charToWrite;
    }

    inline void replaceChar (const stl_wchar newChar) noexcept
    {
        *data = (char) newChar;
    }

    /** Writes a null character to this string (leaving the pointer's position unchanged). */
    inline void writeNull() const noexcept
    {
        *data = 0;
    }

    /** Returns the number of characters in this string. */
    size_t length() const noexcept
    {
        return (size_t) strlen (data);
    }

    /** Returns the number of characters in this string, or the given value, whichever is lower. */
    size_t lengthUpTo (const size_t maxCharsToCount) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, maxCharsToCount);
    }

    /** Returns the number of characters in this string, or up to the given end pointer, whichever is lower. */
    size_t lengthUpTo (const CharPointer_ASCII end) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, end);
    }

    /** Returns the number of bytes that are used to represent this string.
        This includes the terminating null character.
    */
    size_t sizeInBytes() const noexcept
    {
        return length() + 1;
    }

    /** Returns the number of bytes that would be needed to represent the given
        unicode character in this encoding format.
    */
    static size_t getBytesRequiredFor (const stl_wchar) noexcept
    {
        return 1;
    }

    /** Returns the number of bytes that would be needed to represent the given
        string in this encoding format.
        The value returned does NOT include the terminating null character.
    */
    template <class CharPointer>
    static size_t getBytesRequiredFor (const CharPointer text) noexcept
    {
        return text.length();
    }

    /** Returns a pointer to the null character that terminates this string. */
    CharPointer_ASCII findTerminatingNull() const noexcept
    {
        return CharPointer_ASCII (data + length());
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    template <typename CharPointer>
    void writeAll (const CharPointer src) noexcept
    {
        CharacterFunctions::copyAll (*this, src);
    }

    /** 将源字符串复制到此指针，并在指针前进的过程中使其前进。maxDestBytes参数指定可以写入的最大字节数到目的地缓冲区。
    */
    template <typename CharPointer>
    size_t writeWithDestByteLimit (const CharPointer src, const size_t maxDestBytes) noexcept
    {
        return CharacterFunctions::copyWithDestByteLimit (*this, src, maxDestBytes);
    }

    /** 将源字符串复制到此指针，并在指针前进的过程中使其前进。maxChars参数指定可以在停止之前写入目标缓冲区（包括终止null）。
    */
    template <typename CharPointer>
    void writeWithCharLimit (const CharPointer src, const int maxChars) noexcept
    {
        CharacterFunctions::copyWithCharLimit (*this, src, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compare (const CharPointer other) const noexcept
    {
        return CharacterFunctions::compare (*this, other);
    }

    /** Compares this string with another one. */
    int compare (const CharPointer_ASCII other) const noexcept
    {
        return strcmp (data, other.data);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareUpTo (const CharPointer other, const int maxChars) const noexcept
    {
        return CharacterFunctions::compareUpTo (*this, other, maxChars);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    int compareUpTo (const CharPointer_ASCII other, const int maxChars) const noexcept
    {
        return strncmp (data, other.data, (size_t) maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compareIgnoreCase (const CharPointer other) const
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    int compareIgnoreCase (const CharPointer_ASCII other) const
    {
       #if STL_MINGW || (STL_WINDOWS && STL_CLANG)
        return CharacterFunctions::compareIgnoreCase (*this, other);
       #elif STL_WINDOWS
        return stricmp (data, other.data);
       #else
        return strcasecmp (data, other.data);
       #endif
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareIgnoreCaseUpTo (const CharPointer other, const int maxChars) const noexcept
    {
        return CharacterFunctions::compareIgnoreCaseUpTo (*this, other, maxChars);
    }

    /** Returns the character index of a substring, or -1 if it isn't found. */
    template <typename CharPointer>
    int indexOf (const CharPointer stringToFind) const noexcept
    {
        return CharacterFunctions::indexOf (*this, stringToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const stl_wchar charToFind) const noexcept
    {
        int i = 0;

        while (data[i] != 0)
        {
            if (data[i] == (char) charToFind)
                return i;

            ++i;
        }

        return -1;
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const stl_wchar charToFind, const bool ignoreCase) const noexcept
    {
        return ignoreCase ? CharacterFunctions::indexOfCharIgnoreCase (*this, charToFind)
                          : CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Returns true if the first character of this string is whitespace. */
    bool isWhitespace() const                   { return CharacterFunctions::isWhitespace (*data) != 0; }
    /** Returns true if the first character of this string is a digit. */
    bool isDigit() const                        { return CharacterFunctions::isDigit (*data) != 0; }
    /** Returns true if the first character of this string is a letter. */
    bool isLetter() const                       { return CharacterFunctions::isLetter (*data) != 0; }
    /** Returns true if the first character of this string is a letter or digit. */
    bool isLetterOrDigit() const                { return CharacterFunctions::isLetterOrDigit (*data) != 0; }
    /** Returns true if the first character of this string is upper-case. */
    bool isUpperCase() const                    { return CharacterFunctions::isUpperCase ((stl_wchar) (uint8) *data) != 0; }
    /** Returns true if the first character of this string is lower-case. */
    bool isLowerCase() const                    { return CharacterFunctions::isLowerCase ((stl_wchar) (uint8) *data) != 0; }

    /** Returns an upper-case version of the first character of this string. */
    stl_wchar toUpperCase() const noexcept     { return CharacterFunctions::toUpperCase ((stl_wchar) (uint8) *data); }
    /** Returns a lower-case version of the first character of this string. */
    stl_wchar toLowerCase() const noexcept     { return CharacterFunctions::toLowerCase ((stl_wchar) (uint8) *data); }

    /** Parses this string as a 32-bit integer. */
    int getIntValue32() const noexcept          { return atoi (data); }

    /** Parses this string as a 64-bit integer. */
    int64 getIntValue64() const noexcept
    {
       #if STL_LINUX || STL_BSD || STL_ANDROID || STL_MINGW
        return atoll (data);
       #elif STL_WINDOWS
        return _atoi64 (data);
       #else
        return CharacterFunctions::getIntValue <int64, CharPointer_ASCII> (*this);
       #endif
    }

    /** Parses this string as a floating point double. */
    double getDoubleValue() const noexcept                      { return CharacterFunctions::getDoubleValue (*this); }

    /** Returns the first non-whitespace character in the string. */
    CharPointer_ASCII findEndOfWhitespace() const noexcept      { return CharacterFunctions::findEndOfWhitespace (*this); }

    /** Move this pointer to the first non-whitespace character in the string. */
    void incrementToEndOfWhitespace() noexcept                  { CharacterFunctions::incrementToEndOfWhitespace (*this); }

    /** Returns true if the given unicode character can be represented in this encoding. */
    static bool canRepresent (stl_wchar character) noexcept
    {
        return ((unsigned int) character) < (unsigned int) 128;
    }

    /** Returns true if this data contains a valid string in this encoding. */
    static bool isValidString (const CharType* dataToTest, int maxBytesToRead)
    {
        while (--maxBytesToRead >= 0)
        {
            if (((signed char) *dataToTest) <= 0)
                return *dataToTest == 0;

            ++dataToTest;
        }

        return true;
    }

private:
    CharType* data;
};

} // namespace stl
