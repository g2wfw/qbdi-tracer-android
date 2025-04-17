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
#include "text/stl_ByteArray.h"
#include "text/stl_StringTokenizer.h"
#include "text/stl_NumberParser.h"
#include "text/stl_NewLine.h"
#include "streams/stl_InputStream.h"
#include "streams/stl_OutputStream.h"
#include "memory/stl_HeapBlock.h"
#include "text/stl_String.h"
#include "text/stl_StringArray.h"
#include "streams/stl_ZeroCopyByteBuffer.h"

namespace stl {

    STL_BEGIN_IGNORE_WARNINGS_MSVC (4514 4996)

    NewLine newLine;

#if defined (STL_STRINGS_ARE_UNICODE) && !STL_STRINGS_ARE_UNICODE
#error "STL_STRINGS_ARE_UNICODE is deprecated! All strings are now unicode by default."
#endif

#if STL_NATIVE_WCHAR_IS_UTF8
    using CharPointer_wchar_t = CharPointer_UTF8;
#elif STL_NATIVE_WCHAR_IS_UTF16
    using CharPointer_wchar_t = CharPointer_UTF16;
#else
    using CharPointer_wchar_t = CharPointer_UTF32;
#endif

    static CharPointer_wchar_t castToCharPointer_wchar_t(const void *t) noexcept {
        return CharPointer_wchar_t(static_cast<const CharPointer_wchar_t::CharType *> (t));
    }

//==============================================================================
    struct StringHolder {
        using CharPointerType = String::CharPointerType;
        using CharType = String::CharPointerType::CharType;

        std::atomic<int> refCount{0};
        size_t allocatedNumBytes = sizeof(CharType);
        CharType text[1]{0};
    };

    constexpr StringHolder emptyString;

//==============================================================================
    class StringHolderUtils {
    public:
        using CharPointerType = StringHolder::CharPointerType;
        using CharType = StringHolder::CharType;

        static CharPointerType createUninitialisedBytes(size_t numBytes) {
            numBytes = (numBytes + 3) & ~(size_t) 3;
            auto *bytes = new char[sizeof(StringHolder) - sizeof(CharType) + numBytes];
            auto s = unalignedPointerCast<StringHolder *>(bytes);
            s->refCount = 0;
            s->allocatedNumBytes = numBytes;
            return CharPointerType(unalignedPointerCast<CharType *>(bytes + offsetof (StringHolder, text)));
        }

        template<class CharPointer>
        static CharPointerType createFromCharPointer(const CharPointer text) {
            if (text.getAddress() == nullptr || text.isEmpty())
                return CharPointerType(emptyString.text);

            auto bytesNeeded = sizeof(CharType) + CharPointerType::getBytesRequiredFor(text);
            auto dest = createUninitialisedBytes(bytesNeeded);
            CharPointerType(dest).writeAll(text);
            return dest;
        }

        template<class CharPointer>
        static CharPointerType createFromCharPointer(const CharPointer text, size_t maxChars) {
            if (text.getAddress() == nullptr || text.isEmpty() || maxChars == 0)
                return CharPointerType(emptyString.text);

            auto end = text;
            size_t numChars = 0;
            size_t bytesNeeded = sizeof(CharType);

            while (numChars < maxChars && !end.isEmpty()) {
                bytesNeeded += CharPointerType::getBytesRequiredFor(end.getAndAdvance());
                ++numChars;
            }

            auto dest = createUninitialisedBytes(bytesNeeded);
            CharPointerType(dest).writeWithCharLimit(text, (int) numChars + 1);
            return dest;
        }

        template<class CharPointer>
        static CharPointerType createFromCharPointer(const CharPointer start, const CharPointer end) {
            if (start.getAddress() == nullptr || start.isEmpty())
                return CharPointerType(emptyString.text);

            auto e = start;
            int numChars = 0;
            auto bytesNeeded = sizeof(CharType);

            while (e < end && !e.isEmpty()) {
                bytesNeeded += CharPointerType::getBytesRequiredFor(e.getAndAdvance());
                ++numChars;
            }

            auto dest = createUninitialisedBytes(bytesNeeded);
            CharPointerType(dest).writeWithCharLimit(start, numChars + 1);
            return dest;
        }

        static CharPointerType createFromCharPointer(const CharPointerType start, const CharPointerType end) {
            if (start.getAddress() == nullptr || start.isEmpty())
                return CharPointerType(emptyString.text);

            auto numBytes = (size_t) (reinterpret_cast<const char *> (end.getAddress())
                                      - reinterpret_cast<const char *> (start.getAddress()));
            auto dest = createUninitialisedBytes(numBytes + sizeof(CharType));
            memcpy(dest.getAddress(), start, numBytes);
            dest.getAddress()[numBytes / sizeof(CharType)] = 0;
            return dest;
        }

        static CharPointerType createFromFixedLength(const char *const src, const size_t numChars) {
            auto dest = createUninitialisedBytes(numChars * sizeof(CharType) + sizeof(CharType));
            CharPointerType(dest).writeWithCharLimit(CharPointer_UTF8(src), (int) (numChars + 1));
            return dest;
        }

        //==============================================================================
        static void retain(const CharPointerType text) noexcept {
            auto *b = bufferFromText(text);

            if (!isEmptyString(b))
                ++(b->refCount);
        }

        static void release(StringHolder *const b) noexcept {
            if (!isEmptyString(b))
                if (--(b->refCount) == -1)
                    delete[] reinterpret_cast<char *> (b);
        }

        static void release(const CharPointerType text) noexcept {
            release(bufferFromText(text));
        }

        static int getReferenceCount(const CharPointerType text) noexcept {
            return bufferFromText(text)->refCount + 1;
        }

        //==============================================================================
        static CharPointerType makeUniqueWithByteSize(const CharPointerType text, size_t numBytes) {
            auto *b = bufferFromText(text);

            if (isEmptyString(b)) {
                auto newText = createUninitialisedBytes(numBytes);
                newText.writeNull();
                return newText;
            }

            if (b->allocatedNumBytes >= numBytes && b->refCount <= 0)
                return text;

            auto newText = createUninitialisedBytes(jmax(b->allocatedNumBytes, numBytes));
            memcpy(newText.getAddress(), text.getAddress(), b->allocatedNumBytes);
            release(b);

            return newText;
        }

        static size_t getAllocatedNumBytes(const CharPointerType text) noexcept {
            return bufferFromText(text)->allocatedNumBytes;
        }

    private:
        StringHolderUtils() = delete;

        ~StringHolderUtils() = delete;

        static StringHolder *bufferFromText(const CharPointerType charPtr) noexcept {
            return unalignedPointerCast<StringHolder *>(
                    unalignedPointerCast<char *>(charPtr.getAddress()) - offsetof (StringHolder, text));
        }

        static bool isEmptyString(StringHolder *other) {
            return other == &emptyString;
        }

        void compileTimeChecks() {
            // Let me know if any of these assertions fail on your system!
#if STL_NATIVE_WCHAR_IS_UTF8
            static_assert (sizeof (wchar_t) == 1, "STL_NATIVE_WCHAR_IS_* macro has incorrect value");
#elif STL_NATIVE_WCHAR_IS_UTF16
            static_assert(sizeof(wchar_t) == 2, "STL_NATIVE_WCHAR_IS_* macro has incorrect value");
#elif STL_NATIVE_WCHAR_IS_UTF32
            static_assert(sizeof(wchar_t) == 4, "STL_NATIVE_WCHAR_IS_* macro has incorrect value");
#else
#error "native wchar_t size is unknown"
#endif
        }
    };

//==============================================================================
    String::String() noexcept: text(emptyString.text) {
    }

    String::~String() noexcept {
        StringHolderUtils::release(text);
    }

    String::String(const String &other) noexcept: text(other.text) {
        StringHolderUtils::retain(text);
    }

    void String::swapWith(String &other) noexcept {
        std::swap(text, other.text);
    }

    void String::clear() noexcept {
        StringHolderUtils::release(text);
        text = emptyString.text;
    }

    String &String::operator=(const String &other) noexcept {
        StringHolderUtils::retain(other.text);
        StringHolderUtils::release(text.atomicSwap(other.text));
        return *this;
    }

    String::String(String &&other) noexcept: text(other.text) {
        other.text = emptyString.text;
    }

    String &String::operator=(String &&other) noexcept {
        std::swap(text, other.text);
        return *this;
    }

    inline String::PreallocationBytes::PreallocationBytes(const size_t num) noexcept: numBytes(num) {}

    String::String(const PreallocationBytes &preallocationSize)
            : text(StringHolderUtils::createUninitialisedBytes(
            preallocationSize.numBytes + sizeof(CharPointerType::CharType))) {
    }

    void String::preallocateBytes(const size_t numBytesNeeded) {
        text = StringHolderUtils::makeUniqueWithByteSize(text, numBytesNeeded + sizeof(CharPointerType::CharType));
    }

    int String::getReferenceCount() const noexcept {
        return StringHolderUtils::getReferenceCount(text);
    }

//==============================================================================
    String::String(const char *const t)
            : text(StringHolderUtils::createFromCharPointer(CharPointer_UTF8(t))) {

    }

    void String::append(const char ch) {
        const char asString[] = {ch, 0};
        operator+=(asString);
    }

    String::String(const char *const t, const size_t maxChars)
            : text(StringHolderUtils::createFromCharPointer(CharPointer_UTF8(t), maxChars)) {
    }

    String::String(const wchar_t *const t) : text(
            StringHolderUtils::createFromCharPointer(castToCharPointer_wchar_t(t))) {}

    String::String(const CharPointer_UTF8 t) : text(StringHolderUtils::createFromCharPointer(t)) {}

    String::String(const CharPointer_UTF16 t) : text(StringHolderUtils::createFromCharPointer(t)) {}

    String::String(const CharPointer_UTF32 t) : text(StringHolderUtils::createFromCharPointer(t)) {}

    String::String(const CharPointer_ASCII t) : text(StringHolderUtils::createFromCharPointer(t)) {}

    String::String(CharPointer_UTF8 t, size_t maxChars) : text(StringHolderUtils::createFromCharPointer(t, maxChars)) {}

    String::String(CharPointer_UTF16 t, size_t maxChars) : text(
            StringHolderUtils::createFromCharPointer(t, maxChars)) {}

    String::String(CharPointer_UTF32 t, size_t maxChars) : text(
            StringHolderUtils::createFromCharPointer(t, maxChars)) {}

    String::String(const wchar_t *t, size_t maxChars) : text(
            StringHolderUtils::createFromCharPointer(castToCharPointer_wchar_t(t), maxChars)) {}

    String::String(CharPointer_UTF8 start, CharPointer_UTF8 end) : text(
            StringHolderUtils::createFromCharPointer(start, end)) {}

    String::String(CharPointer_UTF16 start, CharPointer_UTF16 end) : text(
            StringHolderUtils::createFromCharPointer(start, end)) {}

    String::String(CharPointer_UTF32 start, CharPointer_UTF32 end) : text(
            StringHolderUtils::createFromCharPointer(start, end)) {}

    String::String(const ByteArray &s) : text(emptyString.text) {
        auto fromData = String::createStringFromData(s.data(), s.size());
        StringHolderUtils::retain(fromData.text);
        this->text = fromData.text;
    }

    String::String(StringRef s) : text(StringHolderUtils::createFromCharPointer(s.text)) {}

    String String::charToString(stl_wchar character) {
        String result(PreallocationBytes(CharPointerType::getBytesRequiredFor(character)));
        CharPointerType t(result.text);
        t.write(character);
        t.writeNull();
        return result;
    }




//==============================================================================
    namespace NumberToStringConverters {
        enum {
            charsNeededForInt = 32,
            charsNeededForDouble = 48
        };

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

        struct StackArrayStream final : public std::basic_streambuf<char, std::char_traits<char>> {
            explicit StackArrayStream(char *d) {
                static const std::locale classicLocale(std::locale::classic());
                imbue(classicLocale);
                setp(d, d + charsNeededForDouble);
            }

            size_t writeDouble(double n, int numDecPlaces, bool useScientificNotation) {
                {
                    std::ostream o(this);

                    if (numDecPlaces > 0) {
                        o.setf(useScientificNotation ? std::ios_base::scientific : std::ios_base::fixed);
                        o.precision((std::streamsize) numDecPlaces);
                    }

                    o << n;
                }

                return (size_t) (pptr() - pbase());
            }
        };

        static char *
        doubleToString(char *buffer, double n, int numDecPlaces, bool useScientificNotation, size_t &len) noexcept {
            StackArrayStream strm(buffer);
            len = strm.writeDouble(n, numDecPlaces, useScientificNotation);
            jassert (len <= charsNeededForDouble);
            return buffer;
        }

        template<typename IntegerType>
        static String::CharPointerType createFromInteger(IntegerType number) {
            char buffer[charsNeededForInt];
            auto *end = buffer + numElementsInArray(buffer);
            auto *start = numberToString(end, number);
            return StringHolderUtils::createFromFixedLength(start, (size_t) (end - start - 1));
        }

        static String::CharPointerType
        createFromDouble(double number, int numberOfDecimalPlaces, bool useScientificNotation) {
            char buffer[charsNeededForDouble];
            size_t len;
            auto start = doubleToString(buffer, number, numberOfDecimalPlaces, useScientificNotation, len);
            return StringHolderUtils::createFromFixedLength(start, len);
        }
    }

//==============================================================================
    String::String(int number) : text(NumberToStringConverters::createFromInteger(number)) {}

    String::String(unsigned int number) : text(NumberToStringConverters::createFromInteger(number)) {}

    String::String(short number) : text(NumberToStringConverters::createFromInteger((int) number)) {}

    String::String(unsigned short number) : text(NumberToStringConverters::createFromInteger((unsigned int) number)) {}

    String::String(int64 number) : text(NumberToStringConverters::createFromInteger(number)) {}

    String::String(uint64 number) : text(NumberToStringConverters::createFromInteger(number)) {}

    String::String(long number) : text(NumberToStringConverters::createFromInteger(number)) {}

    String::String(unsigned long number) : text(NumberToStringConverters::createFromInteger(number)) {}

    String::String(float number) : text(NumberToStringConverters::createFromDouble((double) number, 0, false)) {}

    String::String(double number) : text(NumberToStringConverters::createFromDouble(number, 0, false)) {}

    String::String(float number, int numberOfDecimalPlaces, bool useScientificNotation) : text(
            NumberToStringConverters::createFromDouble((double) number, numberOfDecimalPlaces,
                                                       useScientificNotation)) {}

    String::String(double number, int numberOfDecimalPlaces, bool useScientificNotation) : text(
            NumberToStringConverters::createFromDouble(number, numberOfDecimalPlaces, useScientificNotation)) {}

//==============================================================================
    int String::length() const noexcept {
        return (int) text.length();
    }

    static size_t findByteOffsetOfEnd(String::CharPointerType text) noexcept {
        return (size_t) (((char *) text.findTerminatingNull().getAddress()) - (char *) text.getAddress());
    }

    size_t String::getByteOffsetOfEnd() const noexcept {
        return findByteOffsetOfEnd(text);
    }

    stl_wchar String::operator[](int index) const noexcept {
        jassert (index == 0 || (index > 0 && index <= (int) text.lengthUpTo((size_t) index + 1)));
        return text[index];
    }

    template<typename Type>
    struct HashGenerator {
        template<typename CharPointer>
        static Type calculate(CharPointer t) noexcept {
            Type result = {};

            while (!t.isEmpty())
                result = ((Type) multiplier) * result + (Type) t.getAndAdvance();

            return result;
        }

        enum {
            multiplier = sizeof(Type) > 4 ? 101 : 31
        };
    };

    int String::hashCode() const noexcept { return (int) HashGenerator<uint32>::calculate(text); }

    int64 String::hashCode64() const noexcept { return (int64) HashGenerator<uint64>::calculate(text); }

    size_t String::hash() const noexcept { return HashGenerator<size_t>::calculate(text); }

//==============================================================================
    bool STL_CALLTYPE operator==(const String &s1, const String &s2) noexcept { return s1.compare(s2) == 0; }

    bool STL_CALLTYPE operator!=(const String &s1, const String &s2) noexcept { return s1.compare(s2) != 0; }

    bool STL_CALLTYPE operator==(const String &s1, const char *s2) noexcept { return s1.compare(s2) == 0; }

    bool STL_CALLTYPE operator!=(const String &s1, const char *s2) noexcept { return s1.compare(s2) != 0; }

    bool STL_CALLTYPE operator==(const String &s1, const wchar_t *s2) noexcept { return s1.compare(s2) == 0; }

    bool STL_CALLTYPE operator!=(const String &s1, const wchar_t *s2) noexcept { return s1.compare(s2) != 0; }

    bool STL_CALLTYPE operator==(const String &s1, StringRef s2) noexcept {
        return s1.getCharPointer().compare(s2.text) == 0;
    }

    bool STL_CALLTYPE operator!=(const String &s1, StringRef s2) noexcept {
        return s1.getCharPointer().compare(s2.text) != 0;
    }

    bool STL_CALLTYPE operator<(const String &s1, StringRef s2) noexcept {
        return s1.getCharPointer().compare(s2.text) < 0;
    }

    bool STL_CALLTYPE operator<=(const String &s1, StringRef s2) noexcept {
        return s1.getCharPointer().compare(s2.text) <= 0;
    }

    bool STL_CALLTYPE operator>(const String &s1, StringRef s2) noexcept {
        return s1.getCharPointer().compare(s2.text) > 0;
    }

    bool STL_CALLTYPE operator>=(const String &s1, StringRef s2) noexcept {
        return s1.getCharPointer().compare(s2.text) >= 0;
    }

    bool STL_CALLTYPE operator==(const String &s1, const CharPointer_UTF8 s2) noexcept {
        return s1.getCharPointer().compare(s2) == 0;
    }

    bool STL_CALLTYPE operator!=(const String &s1, const CharPointer_UTF8 s2) noexcept {
        return s1.getCharPointer().compare(s2) != 0;
    }

    bool STL_CALLTYPE operator==(const String &s1, const CharPointer_UTF16 s2) noexcept {
        return s1.getCharPointer().compare(s2) == 0;
    }

    bool STL_CALLTYPE operator!=(const String &s1, const CharPointer_UTF16 s2) noexcept {
        return s1.getCharPointer().compare(s2) != 0;
    }

    bool STL_CALLTYPE operator==(const String &s1, const CharPointer_UTF32 s2) noexcept {
        return s1.getCharPointer().compare(s2) == 0;
    }

    bool STL_CALLTYPE operator!=(const String &s1, const CharPointer_UTF32 s2) noexcept {
        return s1.getCharPointer().compare(s2) != 0;
    }

    bool String::equalsIgnoreCase(const wchar_t *const t) const noexcept {
        return t != nullptr ? text.compareIgnoreCase(castToCharPointer_wchar_t(t)) == 0
                            : isEmpty();
    }

    bool String::equalsIgnoreCase(const char *const t) const noexcept {
        return t != nullptr ? text.compareIgnoreCase(CharPointer_UTF8(t)) == 0
                            : isEmpty();
    }

    bool String::equalsIgnoreCase(StringRef t) const noexcept {
        return text.compareIgnoreCase(t.text) == 0;
    }

    bool String::equalsIgnoreCase(const String &other) const noexcept {
        return text == other.text
               || text.compareIgnoreCase(other.text) == 0;
    }

    int String::compare(const String &other) const noexcept {
        return (text == other.text) ? 0 : text.compare(other.text);
    }

    int String::compare(const char *const other) const noexcept { return text.compare(CharPointer_UTF8(other)); }

    int String::compare(const wchar_t *const other) const noexcept {
        return text.compare(castToCharPointer_wchar_t(other));
    }

    int String::compareIgnoreCase(const String &other) const noexcept {
        return (text == other.text) ? 0 : text.compareIgnoreCase(other.text);
    }

    static int stringCompareRight(String::CharPointerType s1, String::CharPointerType s2) noexcept {
        for (int bias = 0;;) {
            auto c1 = s1.getAndAdvance();
            bool isDigit1 = CharacterFunctions::isDigit(c1);

            auto c2 = s2.getAndAdvance();
            bool isDigit2 = CharacterFunctions::isDigit(c2);

            if (!(isDigit1 || isDigit2)) return bias;
            if (!isDigit1) return -1;
            if (!isDigit2) return 1;

            if (c1 != c2 && bias == 0)
                bias = c1 < c2 ? -1 : 1;

            jassert (c1 != 0 && c2 != 0);
        }
    }

    static int stringCompareLeft(String::CharPointerType s1, String::CharPointerType s2) noexcept {
        for (;;) {
            auto c1 = s1.getAndAdvance();
            bool isDigit1 = CharacterFunctions::isDigit(c1);

            auto c2 = s2.getAndAdvance();
            bool isDigit2 = CharacterFunctions::isDigit(c2);

            if (!(isDigit1 || isDigit2)) return 0;
            if (!isDigit1) return -1;
            if (!isDigit2) return 1;
            if (c1 < c2) return -1;
            if (c1 > c2) return 1;
        }
    }

    static int
    naturalStringCompare(String::CharPointerType s1, String::CharPointerType s2, bool isCaseSensitive) noexcept {
        bool firstLoop = true;

        for (;;) {
            const bool hasSpace1 = s1.isWhitespace();
            const bool hasSpace2 = s2.isWhitespace();

            if ((!firstLoop) && (hasSpace1 ^ hasSpace2)) {
                if (s1.isEmpty()) return -1;
                if (s2.isEmpty()) return 1;

                return hasSpace2 ? 1 : -1;
            }

            firstLoop = false;

            if (hasSpace1) s1 = s1.findEndOfWhitespace();
            if (hasSpace2) s2 = s2.findEndOfWhitespace();

            if (s1.isDigit() && s2.isDigit()) {
                auto result = (*s1 == '0' || *s2 == '0') ? stringCompareLeft(s1, s2)
                                                         : stringCompareRight(s1, s2);

                if (result != 0)
                    return result;
            }

            auto c1 = s1.getAndAdvance();
            auto c2 = s2.getAndAdvance();

            if (c1 != c2 && !isCaseSensitive) {
                c1 = CharacterFunctions::toUpperCase(c1);
                c2 = CharacterFunctions::toUpperCase(c2);
            }

            if (c1 == c2) {
                if (c1 == 0)
                    return 0;
            } else {
                const bool isAlphaNum1 = CharacterFunctions::isLetterOrDigit(c1);
                const bool isAlphaNum2 = CharacterFunctions::isLetterOrDigit(c2);

                if (isAlphaNum2 && !isAlphaNum1) return -1;
                if (isAlphaNum1 && !isAlphaNum2) return 1;

                return c1 < c2 ? -1 : 1;
            }

            jassert (c1 != 0 && c2 != 0);
        }
    }

    int String::compareNatural(StringRef other, bool isCaseSensitive) const noexcept {
        return naturalStringCompare(getCharPointer(), other.text, isCaseSensitive);
    }

//==============================================================================
    void String::append(const String &textToAppend, size_t maxCharsToTake) {
        appendCharPointer(this == &textToAppend ? String(textToAppend).text
                                                : textToAppend.text, maxCharsToTake);
    }

    void String::appendCharPointer(const CharPointerType textToAppend) {
        appendCharPointer(textToAppend, textToAppend.findTerminatingNull());
    }

    void String::appendCharPointer(const CharPointerType startOfTextToAppend,
                                   const CharPointerType endOfTextToAppend) {
        jassert (startOfTextToAppend.getAddress() != nullptr && endOfTextToAppend.getAddress() != nullptr);

        auto extraBytesNeeded = getAddressDifference(endOfTextToAppend.getAddress(),
                                                     startOfTextToAppend.getAddress());
        jassert (extraBytesNeeded >= 0);

        if (extraBytesNeeded > 0) {
            auto byteOffsetOfNull = getByteOffsetOfEnd();
            preallocateBytes((size_t) extraBytesNeeded + byteOffsetOfNull);

            auto *newStringStart = addBytesToPointer(text.getAddress(), (int) byteOffsetOfNull);
            memcpy(newStringStart, startOfTextToAppend.getAddress(), (size_t) extraBytesNeeded);
            CharPointerType(addBytesToPointer(newStringStart, extraBytesNeeded)).writeNull();
        }
    }

    String &String::operator+=(const wchar_t *t) {
        appendCharPointer(castToCharPointer_wchar_t(t));
        return *this;
    }

    String &String::operator+=(const char *t) {
        appendCharPointer(CharPointer_UTF8(t)); // (using UTF8 here triggers a faster code-path than ascii)
        return *this;
    }

    String &String::operator+=(const String &other) {
        if (isEmpty())
            return operator=(other);

        if (this == &other)
            return operator+=(String(*this));

        appendCharPointer(other.text);
        return *this;
    }

    String &String::operator+=(StringRef other) {
        return operator+=(String(other));
    }

    String &String::operator+=(char ch) {
        const char asString[] = {ch, 0};
        return operator+=(asString);
    }

    String &String::operator+=(wchar_t ch) {
        const wchar_t asString[] = {ch, 0};
        return operator+=(asString);
    }

#if !STL_NATIVE_WCHAR_IS_UTF32

    String &String::operator+=(stl_wchar ch) {
        const stl_wchar asString[] = {ch, 0};
        appendCharPointer(CharPointer_UTF32(asString));
        return *this;
    }

#endif

    namespace StringHelpers {
        template<typename T>
        inline String &operationAddAssign(String &str, const T number) {
            char buffer[(sizeof(T) * 8) / 2];
            auto *end = buffer + numElementsInArray(buffer);
            auto *start = NumberToStringConverters::numberToString(end, number);

#if STL_STRING_UTF_TYPE == 8
            str.appendCharPointer(String::CharPointerType(start), String::CharPointerType(end));
#else
            str.appendCharPointer (CharPointer_ASCII (start), CharPointer_ASCII (end));
#endif

            return str;
        }
    }

    String &String::operator+=(const int number) { return StringHelpers::operationAddAssign<int>(*this, number); }

    String &String::operator+=(const long number) { return StringHelpers::operationAddAssign<long>(*this, number); }

    String &String::operator+=(const int64 number) { return StringHelpers::operationAddAssign<int64>(*this, number); }

    String &String::operator+=(const uint64 number) { return StringHelpers::operationAddAssign<uint64>(*this, number); }

//==============================================================================
    String STL_CALLTYPE operator+(const char *s1, const String &s2) {
        String s(s1);
        return s += s2;
    }

    String STL_CALLTYPE operator+(const wchar_t *s1, const String &s2) {
        String s(s1);
        return s += s2;
    }

    String STL_CALLTYPE operator+(char s1, const String &s2) {
        return String::charToString((stl_wchar) (uint8) s1) + s2;
    }

    String STL_CALLTYPE operator+(wchar_t s1, const String &s2) { return String::charToString(s1) + s2; }

    String STL_CALLTYPE operator+(String s1, const String &s2) { return s1 += s2; }

    String STL_CALLTYPE operator+(String s1, const char *s2) { return s1 += s2; }

    String STL_CALLTYPE operator+(String s1, const wchar_t *s2) { return s1 += s2; }


    String STL_CALLTYPE operator+(String s1, char s2) { return s1 += s2; }

    String STL_CALLTYPE operator+(String s1, wchar_t s2) { return s1 += s2; }

#if !STL_NATIVE_WCHAR_IS_UTF32

    String STL_CALLTYPE operator+(stl_wchar s1, const String &s2) { return String::charToString(s1) + s2; }

    String STL_CALLTYPE operator+(String s1, stl_wchar s2) { return s1 += s2; }

    String &STL_CALLTYPE operator<<(String &s1, stl_wchar s2) { return s1 += s2; }

#endif

    String &STL_CALLTYPE operator<<(String &s1, char s2) { return s1 += s2; }

    String &STL_CALLTYPE operator<<(String &s1, wchar_t s2) { return s1 += s2; }

    String &STL_CALLTYPE operator<<(String &s1, const char *s2) { return s1 += s2; }

    String &STL_CALLTYPE operator<<(String &s1, const wchar_t *s2) { return s1 += s2; }

    String &STL_CALLTYPE operator<<(String &s1, const String &s2) { return s1 += s2; }

    String &STL_CALLTYPE operator<<(String &s1, StringRef s2) { return s1 += s2; }


    String &STL_CALLTYPE operator<<(String &s1, uint8 number) { return s1 += (int) number; }

    String &STL_CALLTYPE operator<<(String &s1, short number) { return s1 += (int) number; }

    String &STL_CALLTYPE operator<<(String &s1, int number) { return s1 += number; }

    String &STL_CALLTYPE operator<<(String &s1, long number) { return s1 += String(number); }

    String &STL_CALLTYPE operator<<(String &s1, unsigned long number) { return s1 += String(number); }

    String &STL_CALLTYPE operator<<(String &s1, int64 number) { return s1 += String(number); }

    String &STL_CALLTYPE operator<<(String &s1, uint64 number) { return s1 += String(number); }

    String &STL_CALLTYPE operator<<(String &s1, float number) { return s1 += String(number); }

    String &STL_CALLTYPE operator<<(String &s1, double number) { return s1 += String(number); }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const String &text) {
        return operator<<(stream, StringRef(text));
    }

    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, StringRef text) {
        auto numBytes = CharPointer_UTF8::getBytesRequiredFor(text.text);

#if (STL_STRING_UTF_TYPE == 8)
        stream.write(text.text.getAddress(), numBytes);
#else
                                                                                                                                // (This avoids using toUTF8() to prevent the memory bloat that it would leave behind
    // if lots of large, persistent strings were to be written to streams).
    HeapBlock<char> temp (numBytes + 1);
    CharPointer_UTF8 (temp).writeAll (text.text);
    stream.write (temp, numBytes);
#endif

        return stream;
    }

//==============================================================================
    int String::indexOfChar(stl_wchar character) const noexcept {
        return text.indexOf(character);
    }

    int String::indexOfChar(int startIndex, stl_wchar character) const noexcept {
        auto t = text;

        for (int i = 0; !t.isEmpty(); ++i) {
            if (i >= startIndex) {
                if (t.getAndAdvance() == character)
                    return i;
            } else {
                ++t;
            }
        }

        return -1;
    }

    int String::lastIndexOfChar(stl_wchar character) const noexcept {
        auto t = text;
        int last = -1;

        for (int i = 0; !t.isEmpty(); ++i)
            if (t.getAndAdvance() == character)
                last = i;

        return last;
    }

    int String::indexOfAnyOf(StringRef charactersToLookFor, int startIndex, bool ignoreCase) const noexcept {
        auto t = text;

        for (int i = 0; !t.isEmpty(); ++i) {
            if (i >= startIndex) {
                if (charactersToLookFor.text.indexOf(t.getAndAdvance(), ignoreCase) >= 0)
                    return i;
            } else {
                ++t;
            }
        }

        return -1;
    }

    int String::indexOf(StringRef other) const noexcept {
        return other.isEmpty() ? 0 : text.indexOf(other.text);
    }

    int String::indexOfIgnoreCase(StringRef other) const noexcept {
        return other.isEmpty() ? 0 : CharacterFunctions::indexOfIgnoreCase(text, other.text);
    }

    int String::indexOf(int startIndex, StringRef other) const noexcept {
        if (other.isEmpty())
            return -1;

        auto t = text;

        for (int i = startIndex; --i >= 0;) {
            if (t.isEmpty())
                return -1;

            ++t;
        }

        auto found = t.indexOf(other.text);
        return found >= 0 ? found + startIndex : found;
    }

    int String::indexOfIgnoreCase(const int startIndex, StringRef other) const noexcept {
        if (other.isEmpty())
            return -1;

        auto t = text;

        for (int i = startIndex; --i >= 0;) {
            if (t.isEmpty())
                return -1;

            ++t;
        }

        auto found = CharacterFunctions::indexOfIgnoreCase(t, other.text);
        return found >= 0 ? found + startIndex : found;
    }

    int String::lastIndexOf(StringRef other) const noexcept {
        if (other.isNotEmpty()) {
            auto len = other.length();
            int i = length() - len;

            if (i >= 0) {
                for (auto n = text + i; i >= 0; --i) {
                    if (n.compareUpTo(other.text, len) == 0)
                        return i;

                    --n;
                }
            }
        }

        return -1;
    }

    int String::lastIndexOfIgnoreCase(StringRef other) const noexcept {
        if (other.isNotEmpty()) {
            auto len = other.length();
            int i = length() - len;

            if (i >= 0) {
                for (auto n = text + i; i >= 0; --i) {
                    if (n.compareIgnoreCaseUpTo(other.text, len) == 0)
                        return i;

                    --n;
                }
            }
        }

        return -1;
    }

    int String::lastIndexOfAnyOf(StringRef charactersToLookFor, const bool ignoreCase) const noexcept {
        auto t = text;
        int last = -1;

        for (int i = 0; !t.isEmpty(); ++i)
            if (charactersToLookFor.text.indexOf(t.getAndAdvance(), ignoreCase) >= 0)
                last = i;

        return last;
    }

    bool String::contains(StringRef other) const noexcept {
        return indexOf(other) >= 0;
    }

    bool String::containsChar(const stl_wchar character) const noexcept {
        return text.indexOf(character) >= 0;
    }

    bool String::containsIgnoreCase(StringRef t) const noexcept {
        return indexOfIgnoreCase(t) >= 0;
    }

    int String::indexOfWholeWord(StringRef word) const noexcept {
        if (word.isNotEmpty()) {
            auto t = text;
            auto wordLen = word.length();
            auto end = (int) t.length() - wordLen;

            for (int i = 0; i <= end; ++i) {
                if (t.compareUpTo(word.text, wordLen) == 0
                    && (i == 0 || !(t - 1).isLetterOrDigit())
                    && !(t + wordLen).isLetterOrDigit())
                    return i;

                ++t;
            }
        }

        return -1;
    }

    int String::indexOfWholeWordIgnoreCase(StringRef word) const noexcept {
        if (word.isNotEmpty()) {
            auto t = text;
            auto wordLen = word.length();
            auto end = (int) t.length() - wordLen;

            for (int i = 0; i <= end; ++i) {
                if (t.compareIgnoreCaseUpTo(word.text, wordLen) == 0
                    && (i == 0 || !(t - 1).isLetterOrDigit())
                    && !(t + wordLen).isLetterOrDigit())
                    return i;

                ++t;
            }
        }

        return -1;
    }

    bool String::containsWholeWord(StringRef wordToLookFor) const noexcept {
        return indexOfWholeWord(wordToLookFor) >= 0;
    }

    bool String::containsWholeWordIgnoreCase(StringRef wordToLookFor) const noexcept {
        return indexOfWholeWordIgnoreCase(wordToLookFor) >= 0;
    }

//==============================================================================
    template<typename CharPointer>
    struct WildCardMatcher {
        static bool matches(CharPointer wildcard, CharPointer test, const bool ignoreCase) noexcept {
            for (;;) {
                auto wc = wildcard.getAndAdvance();

                if (wc == '*')
                    return wildcard.isEmpty() || matchesAnywhere(wildcard, test, ignoreCase);

                if (!characterMatches(wc, test.getAndAdvance(), ignoreCase))
                    return false;

                if (wc == 0)
                    return true;
            }
        }

        static bool characterMatches(const stl_wchar wc, const stl_wchar tc, const bool ignoreCase) noexcept {
            return (wc == tc) || (wc == '?' && tc != 0)
                   || (ignoreCase && CharacterFunctions::toLowerCase(wc) == CharacterFunctions::toLowerCase(tc));
        }

        static bool matchesAnywhere(const CharPointer wildcard, CharPointer test, const bool ignoreCase) noexcept {
            for (; !test.isEmpty(); ++test)
                if (matches(wildcard, test, ignoreCase))
                    return true;

            return false;
        }
    };

    bool String::matchesWildcard(StringRef wildcard, const bool ignoreCase) const noexcept {
        return WildCardMatcher<CharPointerType>::matches(wildcard.text, text, ignoreCase);
    }

//==============================================================================
    String String::repeatedString(StringRef stringToRepeat, int numberOfTimesToRepeat) {
        if (numberOfTimesToRepeat <= 0)
            return {};

        String result(PreallocationBytes (findByteOffsetOfEnd(stringToRepeat)
        *(size_t) numberOfTimesToRepeat));
        auto n = result.text;

        while (--numberOfTimesToRepeat >= 0)
            n.writeAll(stringToRepeat.text);

        return result;
    }

    String String::paddedLeft(const stl_wchar padCharacter, int minimumLength) const {
        jassert (padCharacter != 0);

        auto extraChars = minimumLength;
        auto end = text;

        while (!end.isEmpty()) {
            --extraChars;
            ++end;
        }

        if (extraChars <= 0 || padCharacter == 0)
            return *this;

        auto currentByteSize = (size_t) (((char *) end.getAddress()) - (char *) text.getAddress());
        String result(PreallocationBytes(
                currentByteSize + (size_t) extraChars * CharPointerType::getBytesRequiredFor(padCharacter)));
        auto n = result.text;

        while (--extraChars >= 0)
            n.write(padCharacter);

        n.writeAll(text);
        return result;
    }

    String String::paddedRight(const stl_wchar padCharacter, int minimumLength) const {
        jassert (padCharacter != 0);

        auto extraChars = minimumLength;
        CharPointerType end(text);

        while (!end.isEmpty()) {
            --extraChars;
            ++end;
        }

        if (extraChars <= 0 || padCharacter == 0)
            return *this;

        auto currentByteSize = (size_t) (((char *) end.getAddress()) - (char *) text.getAddress());
        String result(PreallocationBytes(
                currentByteSize + (size_t) extraChars * CharPointerType::getBytesRequiredFor(padCharacter)));
        auto n = result.text;

        n.writeAll(text);

        while (--extraChars >= 0)
            n.write(padCharacter);

        n.writeNull();
        return result;
    }

//==============================================================================
    String String::replaceSection(int index, int numCharsToReplace, StringRef stringToInsert) const {
        if (index < 0) {
            // a negative index to replace from?
            jassertfalse;
            index = 0;
        }

        if (numCharsToReplace < 0) {
            // replacing a negative number of characters?
            numCharsToReplace = 0;
            jassertfalse;
        }

        auto insertPoint = text;

        for (int i = 0; i < index; ++i) {
            if (insertPoint.isEmpty()) {
                // replacing beyond the end of the string?
                jassertfalse;
                return *this + stringToInsert;
            }

            ++insertPoint;
        }

        auto startOfRemainder = insertPoint;

        for (int i = 0; i < numCharsToReplace && !startOfRemainder.isEmpty(); ++i)
            ++startOfRemainder;

        if (insertPoint == text && startOfRemainder.isEmpty())
            return stringToInsert.text;

        auto initialBytes = (size_t) (((char *) insertPoint.getAddress()) - (char *) text.getAddress());
        auto newStringBytes = findByteOffsetOfEnd(stringToInsert);
        auto remainderBytes = (size_t) (((char *) startOfRemainder.findTerminatingNull().getAddress()) -
                                        (char *) startOfRemainder.getAddress());

        auto newTotalBytes = initialBytes + newStringBytes + remainderBytes;

        if (newTotalBytes <= 0)
            return {};

        String result(PreallocationBytes ((size_t) newTotalBytes));

        auto *dest = (char *) result.text.getAddress();
        memcpy(dest, text.getAddress(), initialBytes);
        dest += initialBytes;
        memcpy(dest, stringToInsert.text.getAddress(), newStringBytes);
        dest += newStringBytes;
        memcpy(dest, startOfRemainder.getAddress(), remainderBytes);
        dest += remainderBytes;
        CharPointerType(unalignedPointerCast<CharPointerType::CharType *>(dest)).writeNull();

        return result;
    }

    String String::replace(StringRef stringToReplace, StringRef stringToInsert, const bool ignoreCase) const {
        auto stringToReplaceLen = stringToReplace.length();
        auto stringToInsertLen = stringToInsert.length();

        int i = 0;
        String result(*this);

        while ((i = (ignoreCase ? result.indexOfIgnoreCase(i, stringToReplace)
                                : result.indexOf(i, stringToReplace))) >= 0) {
            result = result.replaceSection(i, stringToReplaceLen, stringToInsert);
            i += stringToInsertLen;
        }

        return result;
    }

    String
    String::replaceFirstOccurrenceOf(StringRef stringToReplace, StringRef stringToInsert, const bool ignoreCase) const {
        auto stringToReplaceLen = stringToReplace.length();
        auto index = ignoreCase ? indexOfIgnoreCase(stringToReplace)
                                : indexOf(stringToReplace);

        if (index >= 0)
            return replaceSection(index, stringToReplaceLen, stringToInsert);

        return *this;
    }

    struct StringCreationHelper {
        StringCreationHelper(size_t initialBytes) : allocatedBytes(initialBytes) {
            result.preallocateBytes(allocatedBytes);
            dest = result.getCharPointer();
        }

        StringCreationHelper(const String::CharPointerType s)
                : source(s), allocatedBytes(StringHolderUtils::getAllocatedNumBytes(s)) {
            result.preallocateBytes(allocatedBytes);
            dest = result.getCharPointer();
        }

        void write(stl_wchar c) {
            bytesWritten += String::CharPointerType::getBytesRequiredFor(c);

            if (bytesWritten > allocatedBytes) {
                allocatedBytes += jmax((size_t) 8, allocatedBytes / 16);
                auto destOffset = (size_t) (((char *) dest.getAddress()) -
                                            (char *) result.getCharPointer().getAddress());
                result.preallocateBytes(allocatedBytes);
                dest = addBytesToPointer(result.getCharPointer().getAddress(), (int) destOffset);
            }

            dest.write(c);
        }

        String result;
        String::CharPointerType source{nullptr}, dest{nullptr};
        size_t allocatedBytes, bytesWritten = 0;
    };

    String String::replaceCharacter(const stl_wchar charToReplace, const stl_wchar charToInsert) const {
        if (!containsChar(charToReplace))
            return *this;

        StringCreationHelper builder(text);

        for (;;) {
            auto c = builder.source.getAndAdvance();

            if (c == charToReplace)
                c = charToInsert;

            builder.write(c);

            if (c == 0)
                break;
        }

        return std::move(builder.result);
    }

    String String::replaceCharacters(StringRef charactersToReplace, StringRef charactersToInsertInstead) const {
        // Each character in the first string must have a matching one in the
        // second, so the two strings must be the same length.
        jassert (charactersToReplace.length() == charactersToInsertInstead.length());

        StringCreationHelper builder(text);

        for (;;) {
            auto c = builder.source.getAndAdvance();
            auto index = charactersToReplace.text.indexOf(c);

            if (index >= 0)
                c = charactersToInsertInstead[index];

            builder.write(c);

            if (c == 0)
                break;
        }

        return std::move(builder.result);
    }

//==============================================================================
    bool String::startsWith(StringRef other) const noexcept {
        return text.compareUpTo(other.text, other.length()) == 0;
    }

    bool String::startsWithIgnoreCase(StringRef other) const noexcept {
        return text.compareIgnoreCaseUpTo(other.text, other.length()) == 0;
    }

    bool String::startsWithChar(const stl_wchar character) const noexcept {
        jassert (character != 0); // strings can't contain a null character!

        return *text == character;
    }

    bool String::endsWithChar(const stl_wchar character) const noexcept {
        jassert (character != 0); // strings can't contain a null character!

        if (text.isEmpty())
            return false;

        auto t = text.findTerminatingNull();
        return *--t == character;
    }

    bool String::endsWith(StringRef other) const noexcept {
        auto end = text.findTerminatingNull();
        auto otherEnd = other.text.findTerminatingNull();

        while (end > text && otherEnd > other.text) {
            --end;
            --otherEnd;

            if (*end != *otherEnd)
                return false;
        }

        return otherEnd == other.text;
    }

    bool String::endsWithIgnoreCase(StringRef other) const noexcept {
        auto end = text.findTerminatingNull();
        auto otherEnd = other.text.findTerminatingNull();

        while (end > text && otherEnd > other.text) {
            --end;
            --otherEnd;

            if (end.toLowerCase() != otherEnd.toLowerCase())
                return false;
        }

        return otherEnd == other.text;
    }

//==============================================================================
    String String::toUpperCase() const {
        StringCreationHelper builder(text);

        for (;;) {
            auto c = builder.source.toUpperCase();
            builder.write(c);

            if (c == 0)
                break;

            ++(builder.source);
        }

        return std::move(builder.result);
    }

    String String::toLowerCase() const {
        StringCreationHelper builder(text);

        for (;;) {
            auto c = builder.source.toLowerCase();
            builder.write(c);

            if (c == 0)
                break;

            ++(builder.source);
        }

        return std::move(builder.result);
    }

//==============================================================================
    stl_wchar String::getLastCharacter() const noexcept {
        return isEmpty() ? stl_wchar() : text[length() - 1];
    }

    String String::substring(int start, const int end) const {
        if (start < 0)
            start = 0;

        if (end <= start)
            return {};

        int i = 0;
        auto t1 = text;

        while (i < start) {
            if (t1.isEmpty())
                return {};

            ++i;
            ++t1;
        }

        auto t2 = t1;

        while (i < end) {
            if (t2.isEmpty()) {
                if (start == 0)
                    return *this;

                break;
            }

            ++i;
            ++t2;
        }

        return String(t1, t2);
    }

    String String::substring(int start) const {
        if (start <= 0)
            return *this;

        auto t = text;

        while (--start >= 0) {
            if (t.isEmpty())
                return {};

            ++t;
        }

        return String(t);
    }

    String String::dropLastCharacters(const int numberToDrop) const {
        return String(text, (size_t) jmax(0, length() - numberToDrop));
    }

    String String::getLastCharacters(const int numCharacters) const {
        return String(text + jmax(0, length() - jmax(0, numCharacters)));
    }

    String String::fromFirstOccurrenceOf(StringRef sub, bool includeSubString, bool ignoreCase) const {
        auto i = ignoreCase ? indexOfIgnoreCase(sub)
                            : indexOf(sub);
        if (i < 0)
            return {};

        return substring(includeSubString ? i : i + sub.length());
    }

    String String::fromLastOccurrenceOf(StringRef sub, bool includeSubString, bool ignoreCase) const {
        auto i = ignoreCase ? lastIndexOfIgnoreCase(sub)
                            : lastIndexOf(sub);
        if (i < 0)
            return *this;

        return substring(includeSubString ? i : i + sub.length());
    }

    String String::upToFirstOccurrenceOf(StringRef sub, bool includeSubString, bool ignoreCase) const {
        auto i = ignoreCase ? indexOfIgnoreCase(sub)
                            : indexOf(sub);
        if (i < 0)
            return *this;

        return substring(0, includeSubString ? i + sub.length() : i);
    }

    String String::upToLastOccurrenceOf(StringRef sub, bool includeSubString, bool ignoreCase) const {
        auto i = ignoreCase ? lastIndexOfIgnoreCase(sub)
                            : lastIndexOf(sub);
        if (i < 0)
            return *this;

        return substring(0, includeSubString ? i + sub.length() : i);
    }

    static bool isQuoteCharacter(stl_wchar c) noexcept {
        return c == '"' || c == '\'';
    }

    bool String::isQuotedString() const {
        return isQuoteCharacter(*text.findEndOfWhitespace());
    }

    String String::unquoted() const {
        if (!isQuoteCharacter(*text))
            return *this;

        auto len = length();
        return substring(1, len - (isQuoteCharacter(text[len - 1]) ? 1 : 0));
    }

    String String::quoted(stl_wchar quoteCharacter) const {
        if (isEmpty())
            return charToString(quoteCharacter) + quoteCharacter;

        String t(*this);

        if (!t.startsWithChar(quoteCharacter))
            t = charToString(quoteCharacter) + t;

        if (!t.endsWithChar(quoteCharacter))
            t += quoteCharacter;

        return t;
    }

//==============================================================================
    static String::CharPointerType findTrimmedEnd(const String::CharPointerType start,
                                                  String::CharPointerType end) {
        while (end > start) {
            if (!(--end).isWhitespace()) {
                ++end;
                break;
            }
        }

        return end;
    }

    String String::trim() const {
        if (isNotEmpty()) {
            auto start = text.findEndOfWhitespace();
            auto end = start.findTerminatingNull();
            auto trimmedEnd = findTrimmedEnd(start, end);

            if (trimmedEnd <= start)
                return {};

            if (text < start || trimmedEnd < end)
                return String(start, trimmedEnd);
        }

        return *this;
    }

    String String::trimStart() const {
        if (isNotEmpty()) {
            auto t = text.findEndOfWhitespace();

            if (t != text)
                return String(t);
        }

        return *this;
    }

    String String::trimEnd() const {
        if (isNotEmpty()) {
            auto end = text.findTerminatingNull();
            auto trimmedEnd = findTrimmedEnd(text, end);

            if (trimmedEnd < end)
                return String(text, trimmedEnd);
        }

        return *this;
    }

    String String::trimCharactersAtStart(StringRef charactersToTrim) const {
        auto t = text;

        while (charactersToTrim.text.indexOf(*t) >= 0)
            ++t;

        return t == text ? *this : String(t);
    }

    String String::trimCharactersAtEnd(StringRef charactersToTrim) const {
        if (isNotEmpty()) {
            auto end = text.findTerminatingNull();
            auto trimmedEnd = end;

            while (trimmedEnd > text) {
                if (charactersToTrim.text.indexOf(*--trimmedEnd) < 0) {
                    ++trimmedEnd;
                    break;
                }
            }

            if (trimmedEnd < end)
                return String(text, trimmedEnd);
        }

        return *this;
    }

//==============================================================================
    String String::retainCharacters(StringRef charactersToRetain) const {
        if (isEmpty())
            return {};

        StringCreationHelper builder(text);

        for (;;) {
            auto c = builder.source.getAndAdvance();

            if (charactersToRetain.text.indexOf(c) >= 0)
                builder.write(c);

            if (c == 0)
                break;
        }

        builder.write(0);
        return std::move(builder.result);
    }

    String String::removeCharacters(StringRef charactersToRemove) const {
        if (isEmpty())
            return {};

        StringCreationHelper builder(text);

        for (;;) {
            auto c = builder.source.getAndAdvance();

            if (charactersToRemove.text.indexOf(c) < 0)
                builder.write(c);

            if (c == 0)
                break;
        }

        return std::move(builder.result);
    }

    String String::initialSectionContainingOnly(StringRef permittedCharacters) const {
        for (auto t = text; !t.isEmpty(); ++t)
            if (permittedCharacters.text.indexOf(*t) < 0)
                return String(text, t);

        return *this;
    }

    String String::initialSectionNotContaining(StringRef charactersToStopAt) const {
        for (auto t = text; !t.isEmpty(); ++t)
            if (charactersToStopAt.text.indexOf(*t) >= 0)
                return String(text, t);

        return *this;
    }

    bool String::containsOnly(StringRef chars) const noexcept {
        for (auto t = text; !t.isEmpty();)
            if (chars.text.indexOf(t.getAndAdvance()) < 0)
                return false;

        return true;
    }

    bool String::containsAnyOf(StringRef chars) const noexcept {
        for (auto t = text; !t.isEmpty();)
            if (chars.text.indexOf(t.getAndAdvance()) >= 0)
                return true;

        return false;
    }

    bool String::containsNonWhitespaceChars() const noexcept {
        for (auto t = text; !t.isEmpty(); ++t)
            if (!t.isWhitespace())
                return true;

        return false;
    }

    String String::formattedRaw(const char *pf, ...) {
        size_t bufferSize = 256;

        for (;;) {
            va_list args;
            va_start (args, pf);

#if STL_WINDOWS
            STL_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
#endif

#if STL_ANDROID
                                                                                                                                    HeapBlock<char> temp (bufferSize);
        int num = (int) vsnprintf (temp.get(), bufferSize - 1, pf, args);
        if (num >= static_cast<int> (bufferSize))
            num = -1;
#else
            String wideCharVersion(pf);
            HeapBlock<wchar_t> temp(bufferSize);
            const int num = (int)
#if STL_WINDOWS
                    _vsnwprintf
#else
                    vswprintf
#endif
                            (temp.get(), bufferSize - 1, wideCharVersion.toWideCharPointer(), args);
#endif

#if STL_WINDOWS
            STL_END_IGNORE_WARNINGS_GCC_LIKE
#endif
            va_end (args);

            if (num > 0)
                return String(temp.get());

            bufferSize += 256;

            if (num == 0 ||
                bufferSize > 65536) // the upper limit is a sanity check to avoid situations where vprintf repeatedly
                break;                          // returns -1 because of an error rather than because it needs more space.
        }

        return {};
    }

//==============================================================================
    int String::getIntValue() const noexcept { return text.getIntValue32(); }

    int64 String::getLargeIntValue() const noexcept { return text.getIntValue64(); }

    float String::getFloatValue() const noexcept { return (float) getDoubleValue(); }

    double String::getDoubleValue() const noexcept { return text.getDoubleValue(); }

    int String::getTrailingIntValue() const noexcept {
        int n = 0;
        int mult = 1;
        auto t = text.findTerminatingNull();

        while (--t >= text) {
            if (!t.isDigit()) {
                if (*t == '-')
                    n = -n;

                break;
            }

            n += (int) (((stl_wchar) mult) * (*t - '0'));
            mult *= 10;
        }

        return n;
    }

    static const char hexDigits[] = "0123456789abcdef";

    template<typename Type>
    static String hexToString(Type v) {
        String::CharPointerType::CharType buffer[32];
        auto *end = buffer + numElementsInArray(buffer) - 1;
        auto *t = end;
        *t = 0;

        do {
            *--t = hexDigits[(int) (v & 15)];
            v = static_cast<Type> (v >> 4);

        } while (v != 0);

        return String(String::CharPointerType(t),
                      String::CharPointerType(end));
    }

    String String::createHex(uint8 n) { return hexToString(n); }

    String String::createHex(uint16 n) { return hexToString(n); }

    String String::createHex(uint32 n) { return hexToString(n); }

    String String::createHex(uint64 n) { return hexToString(n); }

    String String::toHexString(const void *const d, const int size, const int groupSize) {
        if (size <= 0)
            return {};

        int numChars = (size * 2) + 2;
        if (groupSize > 0)
            numChars += size / groupSize;

        String s(PreallocationBytes ((size_t) numChars *
        sizeof(CharPointerType::CharType)));

        auto *data = static_cast<const unsigned char *> (d);
        auto dest = s.text;

        for (int i = 0; i < size; ++i) {
            const unsigned char nextByte = *data++;
            dest.write((stl_wchar) hexDigits[nextByte >> 4]);
            dest.write((stl_wchar) hexDigits[nextByte & 0xf]);

            if (groupSize > 0 && (i % groupSize) == (groupSize - 1) && i < (size - 1))
                dest.write((stl_wchar) ' ');
        }

        dest.writeNull();
        return s;
    }

    int String::getHexValue32() const noexcept { return (int32) CharacterFunctions::HexParser<uint32>::parse(text); }

    int64 String::getHexValue64() const noexcept { return (int64) CharacterFunctions::HexParser<uint64>::parse(text); }

//==============================================================================
    static String getStringFromWindows1252Codepage(const char *data, size_t num) {
        HeapBlock<stl_wchar> unicode(num + 1);

        for (size_t i = 0; i < num; ++i)
            unicode[i] = CharacterFunctions::getUnicodeCharFromWindows1252Codepage((uint8) data[i]);

        unicode[num] = 0;
        return CharPointer_UTF32(unicode);
    }

    String String::createStringFromData(const void *const unknownData, int size) {
        auto *data = static_cast<const uint8 *> (unknownData);

        if (size <= 0 || data == nullptr)
            return {};

        if (size == 1)
            return charToString((stl_wchar) data[0]);

        if (CharPointer_UTF16::isByteOrderMarkBigEndian(data)
            || CharPointer_UTF16::isByteOrderMarkLittleEndian(data)) {
            const int numChars = size / 2 - 1;

            StringCreationHelper builder((size_t) numChars);

            auto src = unalignedPointerCast<const uint16 *>(data + 2);

            if (CharPointer_UTF16::isByteOrderMarkBigEndian(data)) {
                for (int i = 0; i < numChars; ++i)
                    builder.write((stl_wchar) ByteOrder::swapIfLittleEndian(src[i]));
            } else {
                for (int i = 0; i < numChars; ++i)
                    builder.write((stl_wchar) ByteOrder::swapIfBigEndian(src[i]));
            }

            builder.write(0);
            return std::move(builder.result);
        }

        auto *start = (const char *) data;

        if (size >= 3 && CharPointer_UTF8::isByteOrderMark(data)) {
            start += 3;
            size -= 3;
        }

        if (CharPointer_UTF8::isValidString(start, size))
            return String(CharPointer_UTF8(start),
                          CharPointer_UTF8(start + size));

        return getStringFromWindows1252Codepage(start, (size_t) size);
    }

//==============================================================================
    static const stl_wchar emptyChar = 0;

    template<class CharPointerType_Src, class CharPointerType_Dest>
    struct StringEncodingConverter {
        static CharPointerType_Dest convert(const String &s) {
            auto &source = const_cast<String &> (s);

            using DestChar = typename CharPointerType_Dest::CharType;

            if (source.isEmpty())
                return CharPointerType_Dest(reinterpret_cast<const DestChar *> (&emptyChar));

            CharPointerType_Src text(source.getCharPointer());
            auto extraBytesNeeded =
                    CharPointerType_Dest::getBytesRequiredFor(text) + sizeof(typename CharPointerType_Dest::CharType);
            auto endOffset = (text.sizeInBytes() + 3) & ~3u; // the new string must be word-aligned or many Windows
            // functions will fail to read it correctly!
            source.preallocateBytes(endOffset + extraBytesNeeded);
            text = source.getCharPointer();

            void *const newSpace = addBytesToPointer(text.getAddress(), (int) endOffset);
            const CharPointerType_Dest extraSpace(static_cast<DestChar *> (newSpace));

#if STL_DEBUG // (This just avoids spurious warnings from valgrind about the uninitialised bytes at the end of the buffer..)
                                                                                                                                    auto bytesToClear = (size_t) jmin ((int) extraBytesNeeded, 4);
        zeromem (addBytesToPointer (newSpace, extraBytesNeeded - bytesToClear), bytesToClear);
#endif

            CharPointerType_Dest(extraSpace).writeAll(text);
            return extraSpace;
        }
    };

    template<>
    struct StringEncodingConverter<CharPointer_UTF8, CharPointer_UTF8> {
        static CharPointer_UTF8 convert(const String &source) noexcept {
            return CharPointer_UTF8(
                    unalignedPointerCast<CharPointer_UTF8::CharType *>(source.getCharPointer().getAddress()));
        }
    };

    template<>
    struct StringEncodingConverter<CharPointer_UTF16, CharPointer_UTF16> {
        static CharPointer_UTF16 convert(const String &source) noexcept {
            return CharPointer_UTF16(
                    unalignedPointerCast<CharPointer_UTF16::CharType *>(source.getCharPointer().getAddress()));
        }
    };

    template<>
    struct StringEncodingConverter<CharPointer_UTF32, CharPointer_UTF32> {
        static CharPointer_UTF32 convert(const String &source) noexcept {
            return CharPointer_UTF32(
                    unalignedPointerCast<CharPointer_UTF32::CharType *>(source.getCharPointer().getAddress()));
        }
    };

    CharPointer_UTF8
    String::toUTF8() const { return StringEncodingConverter<CharPointerType, CharPointer_UTF8>::convert(*this); }

    CharPointer_UTF16
    String::toUTF16() const { return StringEncodingConverter<CharPointerType, CharPointer_UTF16>::convert(*this); }

    CharPointer_UTF32
    String::toUTF32() const { return StringEncodingConverter<CharPointerType, CharPointer_UTF32>::convert(*this); }

    const char *String::toRawUTF8() const {
        return toUTF8().getAddress();
    }

    const wchar_t *String::toWideCharPointer() const {
        return StringEncodingConverter<CharPointerType, CharPointer_wchar_t>::convert(*this).getAddress();
    }

    std::string String::toStdString() const {
        return std::string(toRawUTF8());
    }

//==============================================================================
    template<class CharPointerType_Src, class CharPointerType_Dest>
    struct StringCopier {
        static size_t
        copyToBuffer(const CharPointerType_Src source, typename CharPointerType_Dest::CharType *const buffer,
                     const size_t maxBufferSizeBytes) {
            jassert (((ssize_t) maxBufferSizeBytes) >= 0); // keep this value positive!

            if (buffer == nullptr)
                return CharPointerType_Dest::getBytesRequiredFor(source) +
                       sizeof(typename CharPointerType_Dest::CharType);

            return CharPointerType_Dest(buffer).writeWithDestByteLimit(source, maxBufferSizeBytes);
        }
    };

    size_t String::copyToUTF8(CharPointer_UTF8::CharType *const buffer, size_t maxBufferSizeBytes) const noexcept {
        return StringCopier<CharPointerType, CharPointer_UTF8>::copyToBuffer(text, buffer, maxBufferSizeBytes);
    }

    size_t String::copyToUTF16(CharPointer_UTF16::CharType *const buffer, size_t maxBufferSizeBytes) const noexcept {
        return StringCopier<CharPointerType, CharPointer_UTF16>::copyToBuffer(text, buffer, maxBufferSizeBytes);
    }

    size_t String::copyToUTF32(CharPointer_UTF32::CharType *const buffer, size_t maxBufferSizeBytes) const noexcept {
        return StringCopier<CharPointerType, CharPointer_UTF32>::copyToBuffer(text, buffer, maxBufferSizeBytes);
    }

//==============================================================================
    size_t String::getNumBytesAsUTF8() const noexcept {
        return CharPointer_UTF8::getBytesRequiredFor(text);
    }

    String String::fromUTF8(const char *const buffer, int bufferSizeBytes) {
        if (buffer != nullptr) {
            if (bufferSizeBytes < 0)
                return String(CharPointer_UTF8(buffer));

            if (bufferSizeBytes > 0) {
                jassert (CharPointer_UTF8::isValidString(buffer, bufferSizeBytes));
                return String(CharPointer_UTF8(buffer), CharPointer_UTF8(buffer + bufferSizeBytes));
            }
        }

        return {};
    }

    const char *String::c_str() const {
        return toRawUTF8();
    }


    size_t String::countOfChar(const char c) const {
        //统计c 出现的次数
        auto s = toStdString();
        return std::count(s.begin(), s.end(), c);
    }

    size_t String::countOfWChar(const wchar_t c) const {
        std::wstring s = toWideCharPointer();
        return std::count(s.begin(), s.end(), c);
    }

    short String::toShort(String::ConvertType type) const {
        switch (type) {
            case TYPE_OCT:
                return (short) NumberParser::parseOct(toStdString());
            case TYPE_DEC:
                return (short) NumberParser::parse(toStdString());
                //break;
            case TYPE_HEX:
                return (short) NumberParser::parseHex(toStdString());
        }
        return 0;
    }

    uint16_t String::toUShort(String::ConvertType type) const {
        switch (type) {
            case TYPE_OCT:
                return (uint16_t) NumberParser::parseOct(toStdString());
            case TYPE_DEC:
                return (uint16_t) NumberParser::parse(toStdString());
                //break;
            case TYPE_HEX:
                return (uint16_t) NumberParser::parseHex(toStdString());
        }
        return 0;
    }

    int String::toInt(String::ConvertType type) const {
        switch (type) {
            case TYPE_OCT:
                return (int) NumberParser::parseOct(*this);
            case TYPE_DEC:
                return NumberParser::parse(*this);
                //break;
            case TYPE_HEX:
                return (int) NumberParser::parseHex(toStdString());
        }
        return 0;
    }

    uint32_t String::toUInt(String::ConvertType type) const {
        switch (type) {
            case TYPE_OCT:
                return (uint32_t) NumberParser::parseOct(toStdString());
            case TYPE_DEC:
                return NumberParser::parse(toStdString());
                //break;
            case TYPE_HEX:
                return (uint32_t) NumberParser::parseHex(toStdString());
        }
        return 0;
    }

    long String::toLong(String::ConvertType type) const {
        switch (type) {
            case TYPE_OCT:
                return (long) NumberParser::parseOct64(toStdString());
            case TYPE_DEC:
                return NumberParser::parse64(toStdString());
                //break;
            case TYPE_HEX:
                return (long) NumberParser::parseHex64(toStdString());
        }
        return 0;
    }

    unsigned long String::toULong(String::ConvertType type) const {
        switch (type) {
            case TYPE_OCT:
                return (unsigned long) NumberParser::parseOct64(toStdString());
            case TYPE_DEC:
                return NumberParser::parse64(toStdString());
                //break;
            case TYPE_HEX:
                return (unsigned long) NumberParser::parseHex64(toStdString());
        }
        return 0;
    }

    long long String::toLongLong(String::ConvertType type) const {
        switch (type) {
            case TYPE_OCT:
                return (long long) NumberParser::parseOct64(toStdString());
            case TYPE_DEC:
                return NumberParser::parse64(toStdString());
                //break;
            case TYPE_HEX:
                return (long long) NumberParser::parseHex64(toStdString());
        }
        return 0;
    }

    unsigned long long String::toULongLong(String::ConvertType type) const {
        switch (type) {
            case TYPE_OCT:
                return (unsigned long long) NumberParser::parseOct64(toStdString());
            case TYPE_DEC:
                return NumberParser::parse64(toStdString());
                //break;
            case TYPE_HEX:
                return (unsigned long long) NumberParser::parseHex64(toStdString());
        }
        return 0;
    }

    float String::toFloat() const {
        return (float) NumberParser::parseFloat(toStdString());
    }

    double String::toDouble() const {
        return NumberParser::parseFloat(toStdString());
    }

    String String::stringPrintf(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        stl::String result;
        StringAppendV(&result, fmt, ap);
        va_end(ap);
        return result;
    }

    void String::StringAppendV(stl::String *dst, const char *format, va_list ap) {
        // First try with a small fixed size buffer
        char space[1024];

        // It's possible for methods that use a va_list to invalidate
        // the data in it upon use.  The fix is to make a copy
        // of the structure before using it and use that copy instead.
        va_list backup_ap;
        va_copy(backup_ap, ap);
        int result = vsnprintf(space, sizeof(space), format, backup_ap);
        va_end(backup_ap);

        if (result < static_cast<int>(sizeof(space))) {
            if (result >= 0) {
                // Normal case -- everything fit.
                dst->append(space, result);
                return;
            }

            if (result < 0) {
                // Just an error.
                return;
            }
        }
        // Increase the buffer size to the size requested by vsnprintf,
        // plus one for the closing \0.
        int length = result + 1;
        char *buf = new char[length];

        // Restore the va_list before we use it again
        va_copy(backup_ap, ap);
        result = vsnprintf(buf, length, format, backup_ap);
        va_end(backup_ap);

        if (result >= 0 && result < length) {
            // It fit
            dst->append(buf, result);
        }
        delete[] buf;
    }


    size_t String::size() const {
        return text.sizeInBytes() - 1;
    }

    std::unique_ptr<ZeroCopyByteBuffer> String::createZeroCopyByteBuffer() const {
        if (text.getAddress() == nullptr) {
            return nullptr;
        }
        auto bytebuffer = std::make_unique<ZeroCopyByteBuffer>((uint8_t *) text.getAddress(), size());
        return bytebuffer;
    }

    StringArray String::splitToArray(const char *prefix) const noexcept {
        StringTokenizer tokenizer(*this, prefix);
        StringArray stringArray;

        for (const auto &item: tokenizer) {
            if (!item.empty()) {
                stringArray.add(item);
            }
        }
        if (stringArray.isEmpty()) {
            stringArray.add(*this);
        }
        return stringArray;
    }

    STL_END_IGNORE_WARNINGS_MSVC

//==============================================================================
    StringRef::StringRef() noexcept: text(unalignedPointerCast<const String::CharPointerType::CharType *>("\0\0\0")) {
    }

    StringRef::StringRef(const char *stringLiteral) noexcept
#if STL_STRING_UTF_TYPE != 8
    : text (nullptr), stringCopy (stringLiteral)
#else
            : text(stringLiteral)
#endif
    {
#if STL_STRING_UTF_TYPE != 8
        text = stringCopy.getCharPointer();
#endif

        jassert (stringLiteral != nullptr); // This must be a valid string literal, not a null pointer!!

#if STL_NATIVE_WCHAR_IS_UTF8
                                                                                                                                /*  If you get an assertion here, then you're trying to create a string from 8-bit data
        that contains values greater than 127. These can NOT be correctly converted to unicode
        because there's no way for the String class to know what encoding was used to
        create them. The source data could be UTF-8, ASCII or one of many local code-pages.

        To get around this problem, you must be more explicit when you pass an ambiguous 8-bit
        string to the StringRef class - so for example if your source data is actually UTF-8,
        you'd call StringRef (CharPointer_UTF8 ("my utf8 string..")), and it would be able to
        correctly convert the multi-byte characters to unicode. It's *highly* recommended that
        you use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent these strings in a way that isn't dependent on
        the compiler, source code editor and platform.
    */
    jassert (CharPointer_ASCII::isValidString (stringLiteral, std::numeric_limits<int>::max()));
#endif
    }

    StringRef::StringRef(String::CharPointerType stringLiteral) noexcept: text(stringLiteral) {
        jassert (stringLiteral.getAddress() != nullptr); // This must be a valid string literal, not a null pointer!!
    }

    StringRef::StringRef(const String &string) noexcept: text(string.getCharPointer()) {}


//==============================================================================
    static String reduceLengthOfFloatString(const String &input) {
        const auto start = input.getCharPointer();
        const auto end = start + (int) input.length();
        auto trimStart = end;
        auto trimEnd = trimStart;
        auto exponentTrimStart = end;
        auto exponentTrimEnd = exponentTrimStart;

        decltype(*start) currentChar = '\0';

        for (auto c = end - 1; c > start; --c) {
            currentChar = *c;

            if (currentChar == '0' && c + 1 == trimStart) {
                --trimStart;
            } else if (currentChar == '.') {
                if (trimStart == c + 1 && trimStart != end && *trimStart == '0')
                    ++trimStart;

                break;
            } else if (currentChar == 'e' || currentChar == 'E') {
                auto cNext = c + 1;

                if (cNext != end) {
                    if (*cNext == '-')
                        ++cNext;

                    exponentTrimStart = cNext;

                    if (cNext != end && *cNext == '+')
                        ++cNext;

                    exponentTrimEnd = cNext;
                }

                while (cNext != end && *cNext++ == '0')
                    exponentTrimEnd = cNext;

                if (exponentTrimEnd == end)
                    exponentTrimStart = c;

                trimStart = c;
                trimEnd = trimStart;
            }
        }

        if ((trimStart != trimEnd && currentChar == '.') || exponentTrimStart != exponentTrimEnd) {
            if (trimStart == trimEnd)
                return String(start, exponentTrimStart) + String(exponentTrimEnd, end);

            if (exponentTrimStart == exponentTrimEnd)
                return String(start, trimStart) + String(trimEnd, end);

            if (trimEnd == exponentTrimStart)
                return String(start, trimStart) + String(exponentTrimEnd, end);

            return String(start, trimStart) + String(trimEnd, exponentTrimStart) + String(exponentTrimEnd, end);
        }

        return input;
    }

    String serialiseDouble(double input) {
        auto absInput = std::abs(input);

        if (absInput >= 1.0e6 || absInput <= 1.0e-5)
            return reduceLengthOfFloatString({input, 15, true});

        int intInput = (int) input;

        if (exactlyEqual((double) intInput, input))
            return {input, 1};

        auto numberOfDecimalPlaces = [absInput] {
            if (absInput < 1.0) {
                if (absInput >= 1.0e-3) {
                    if (absInput >= 1.0e-1) return 16;
                    if (absInput >= 1.0e-2) return 17;
                    return 18;
                }

                if (absInput >= 1.0e-4) return 19;
                return 20;
            }

            if (absInput < 1.0e3) {
                if (absInput < 1.0e1) return 15;
                if (absInput < 1.0e2) return 14;
                return 13;
            }

            if (absInput < 1.0e4) return 12;
            if (absInput < 1.0e5) return 11;
            return 10;
        }();

        return reduceLengthOfFloatString(String(input, numberOfDecimalPlaces));
    }

//==============================================================================
#if STL_ALLOW_STATIC_NULL_VARIABLES

                                                                                                                            STL_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
STL_BEGIN_IGNORE_WARNINGS_MSVC (4996)

const String String::empty;

STL_END_IGNORE_WARNINGS_GCC_LIKE
STL_END_IGNORE_WARNINGS_MSVC

#endif


} // namespace stl
