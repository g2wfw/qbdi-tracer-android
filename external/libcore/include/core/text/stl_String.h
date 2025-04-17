#pragma once

#include "../stl_core.h"

#if !defined (DOXYGEN) && (STL_MAC || STL_IOS)
// Annoyingly we can only forward-declare a typedef by forward-declaring the
// aliased type
#if __has_attribute(objc_bridge)
#define STL_CF_BRIDGED_TYPE(T) __attribute__ ((objc_bridge (T)))
#else
#define STL_CF_BRIDGED_TYPE(T)
#endif

typedef const struct STL_CF_BRIDGED_TYPE(NSString) __CFString * CFStringRef;

#undef STL_CF_BRIDGED_TYPE
#endif

#include "../logging/fmt/fmt.h"

namespace stl {
    class ByteArray;

    class StringArray;

    class ZeroCopyByteBuffer;
//==============================================================================



    class STL_API String final {
    public:
        //==============================================================================
        /** Creates an empty string.
            @see empty
        */
        String() noexcept;

        /** Creates a copy of another string. */
        String(const String &) noexcept;

        /** Move constructor */
        String(String &&) noexcept;


        String(const char *text);


        String(const char *text, size_t maxChars);


        String(const wchar_t *text);


        String(const wchar_t *text, size_t maxChars);

        //==============================================================================
        
        String(CharPointer_UTF8 text);

        
        String(CharPointer_UTF8 text, size_t maxChars);

        
        String(CharPointer_UTF8 start, CharPointer_UTF8 end);

        //==============================================================================
        
        String(CharPointer_UTF16 text);

        
        String(CharPointer_UTF16 text, size_t maxChars);

        
        String(CharPointer_UTF16 start, CharPointer_UTF16 end);

        //==============================================================================
        
        String(CharPointer_UTF32 text);

        
        String(CharPointer_UTF32 text, size_t maxChars);

        
        String(CharPointer_UTF32 start, CharPointer_UTF32 end);


        String(CharPointer_ASCII text);

        
        inline String(const std::string &str) : String(str.c_str()) {
        };

        inline String(std::string &string) : String(string.c_str()) {

        }

        String(const ByteArray &);

        
        String(StringRef);

        //==============================================================================
        
        static String charToString(stl_wchar character);

        /** Destructor. */
        ~String() noexcept;


        enum ConvertType {
            TYPE_OCT,
            TYPE_DEC,
            TYPE_HEX,
        };

        short toShort(ConvertType= TYPE_DEC) const;

        uint16_t toUShort(ConvertType= TYPE_DEC) const;

        int toInt(ConvertType= TYPE_DEC) const;

        uint32 toUInt(ConvertType= TYPE_DEC) const;

        long toLong(ConvertType= TYPE_DEC) const;

        unsigned long toULong(ConvertType= TYPE_DEC) const;

        long long toLongLong(ConvertType= TYPE_DEC) const;

        unsigned long long toULongLong(ConvertType= TYPE_DEC) const;

        float toFloat() const;

        double toDouble() const;


#if (STL_STRING_UTF_TYPE == 32)
        using CharPointerType = CharPointer_UTF32;
#elif (STL_STRING_UTF_TYPE == 16)
        using CharPointerType = CharPointer_UTF16;
#elif (DOXYGEN || STL_STRING_UTF_TYPE == 8)
        using CharPointerType = CharPointer_UTF8;
#else
#error "You must set the value of STL_STRING_UTF_TYPE to be either 8, 16, or 32!"
#endif

        //==============================================================================
        int hashCode() const noexcept;


        int64 hashCode64() const noexcept;


        size_t hash() const noexcept;


        int length() const noexcept;


        StringArray splitToArray(const char *prefix) const noexcept;
        //==============================================================================
        // Assignment and concatenation operators..

        
        String &operator=(const String &other) noexcept;

        
        String &operator=(String &&other) noexcept;

        /** Appends another string at the end of this one. */
        String &operator+=(const String &stringToAppend);

        /** Appends another string at the end of this one. */
        String &operator+=(const char *textToAppend);

        /** Appends another string at the end of this one. */
        String &operator+=(const wchar_t *textToAppend);

        /** Appends another string at the end of this one. */
        String &operator+=(StringRef textToAppend);

        /** Appends a decimal number at the end of this string. */
        String &operator+=(int numberToAppend);

        /** Appends a decimal number at the end of this string. */
        String &operator+=(long numberToAppend);

        /** Appends a decimal number at the end of this string. */
        String &operator+=(int64 numberToAppend);

        /** Appends a decimal number at the end of this string. */
        String &operator+=(uint64 numberToAppend);

        /** Appends a character at the end of this string. */
        String &operator+=(char characterToAppend);

        /** Appends a character at the end of this string. */
        String &operator+=(wchar_t characterToAppend);

#if !STL_NATIVE_WCHAR_IS_UTF32

        /** Appends a character at the end of this string. */
        String &operator+=(stl_wchar characterToAppend);

#endif

        std::string toString() const;

        std::string toStringV2() const {
            return toStdStringV2();
        }


        void append(const String &textToAppend, size_t maxCharsToTake = std::numeric_limits<size_t>::max());


        void append(const char ch);


        void appendCharPointer(CharPointerType startOfTextToAppend,
                               CharPointerType endOfTextToAppend);


        template<class CharPointer>
        void appendCharPointer(CharPointer startOfTextToAppend,
                               CharPointer endOfTextToAppend) {
            jassert (startOfTextToAppend.getAddress() != nullptr && endOfTextToAppend.getAddress() != nullptr);

            size_t extraBytesNeeded = 0, numChars = 1;

            for (auto t = startOfTextToAppend; t != endOfTextToAppend && !t.isEmpty(); ++numChars)
                extraBytesNeeded += CharPointerType::getBytesRequiredFor(t.getAndAdvance());

            if (extraBytesNeeded > 0) {
                auto byteOffsetOfNull = getByteOffsetOfEnd();

                preallocateBytes(byteOffsetOfNull + extraBytesNeeded);
                CharPointerType(addBytesToPointer(text.getAddress(), (int) byteOffsetOfNull))
                        .writeWithCharLimit(startOfTextToAppend, (int) numChars);
            }
        }

        /** Appends a string to the end of this one. */
        void appendCharPointer(CharPointerType textToAppend);

        /** Appends a string to the end of this one.

            @param textToAppend     the string to add
            @param maxCharsToTake   the maximum number of characters to take from the string passed in
        */
        template<class CharPointer>
        void appendCharPointer(CharPointer textToAppend, size_t maxCharsToTake) {
            if (textToAppend.getAddress() != nullptr) {
                size_t extraBytesNeeded = 0, numChars = 1;

                for (auto t = textToAppend; numChars <= maxCharsToTake && !t.isEmpty(); ++numChars)
                    extraBytesNeeded += CharPointerType::getBytesRequiredFor(t.getAndAdvance());

                if (extraBytesNeeded > 0) {
                    auto byteOffsetOfNull = getByteOffsetOfEnd();

                    preallocateBytes(byteOffsetOfNull + extraBytesNeeded);
                    CharPointerType(addBytesToPointer(text.getAddress(), (int) byteOffsetOfNull))
                            .writeWithCharLimit(textToAppend, (int) numChars);
                }
            }
        }

        /** Appends a string to the end of this one. */
        template<class CharPointer>
        void appendCharPointer(CharPointer textToAppend) {
            appendCharPointer(textToAppend, std::numeric_limits<size_t>::max());
        }

        //==============================================================================
        // Comparison methods..

        bool isEmpty() const noexcept { return text.isEmpty(); }


        bool isNotEmpty() const noexcept { return !text.isEmpty(); }

        
        void clear() noexcept;

        
        bool equalsIgnoreCase(const String &other) const noexcept;

        
        bool equalsIgnoreCase(StringRef other) const noexcept;

        
        bool equalsIgnoreCase(const wchar_t *other) const noexcept;

        
        bool equalsIgnoreCase(const char *other) const noexcept;


        int compare(const String &other) const noexcept;

        /** Case-sensitive comparison with another string.
            @returns     0 if the two strings are identical; negative if this string comes before
                         the other one alphabetically, or positive if it comes after it.
        */
        int compare(const char *other) const noexcept;

        /** Case-sensitive comparison with another string.
            @returns     0 if the two strings are identical; negative if this string comes before
                         the other one alphabetically, or positive if it comes after it.
        */
        int compare(const wchar_t *other) const noexcept;

        /** Case-insensitive comparison with another string.
            @returns     0 if the two strings are identical; negative if this string comes before
                         the other one alphabetically, or positive if it comes after it.
        */
        int compareIgnoreCase(const String &other) const noexcept;

        /** Compares two strings, taking into account textual characteristics like numbers and spaces.

            This comparison is case-insensitive and can detect words and embedded numbers in the
            strings, making it good for sorting human-readable lists of things like filenames.

            @returns     0 if the two strings are identical; negative if this string comes before
                         the other one alphabetically, or positive if it comes after it.
        */
        int compareNatural(StringRef other, bool isCaseSensitive = false) const noexcept;

        /** Tests whether the string begins with another string.
            If the parameter is an empty string, this will always return true.
            Uses a case-sensitive comparison.
        */
        bool startsWith(StringRef text) const noexcept;

        /** Tests whether the string begins with a particular character.
            If the character is 0, this will always return false.
            Uses a case-sensitive comparison.
        */
        bool startsWithChar(stl_wchar character) const noexcept;

        /** Tests whether the string begins with another string.
            If the parameter is an empty string, this will always return true.
            Uses a case-insensitive comparison.
        */
        bool startsWithIgnoreCase(StringRef text) const noexcept;

        /** Tests whether the string ends with another string.
            If the parameter is an empty string, this will always return true.
            Uses a case-sensitive comparison.
        */
        bool endsWith(StringRef text) const noexcept;

        /** Tests whether the string ends with a particular character.
            If the character is 0, this will always return false.
            Uses a case-sensitive comparison.
        */
        bool endsWithChar(stl_wchar character) const noexcept;

        [[deprecated]]
        std::unique_ptr<ZeroCopyByteBuffer> createZeroCopyByteBuffer() const;


        size_t size() const;

        bool endsWithIgnoreCase(StringRef text) const noexcept;


        bool contains(StringRef text) const noexcept;

        bool containsChar(stl_wchar character) const noexcept;

        
        bool containsIgnoreCase(StringRef text) const noexcept;

        bool containsWholeWord(StringRef wordToLookFor) const noexcept;

        bool containsWholeWordIgnoreCase(StringRef wordToLookFor) const noexcept;


        int indexOfWholeWord(StringRef wordToLookFor) const noexcept;

        int indexOfWholeWordIgnoreCase(StringRef wordToLookFor) const noexcept;

        bool containsAnyOf(StringRef charactersItMightContain) const noexcept;

        /** Looks for a set of characters in the string.
            Uses a case-sensitive comparison.

            @returns    Returns false if any of the characters in this string do not occur in
                        the parameter string. If this string is empty, the return value will
                        always be true.
        */
        bool containsOnly(StringRef charactersItMightContain) const noexcept;


        bool containsNonWhitespaceChars() const noexcept;


        bool matchesWildcard(StringRef wildcard, bool ignoreCase) const noexcept;

        //==============================================================================
        // Substring location methods..

        int indexOfChar(stl_wchar characterToLookFor) const noexcept;


        int indexOfChar(int startIndex, stl_wchar characterToLookFor) const noexcept;


        int indexOfAnyOf(StringRef charactersToLookFor,
                         int startIndex = 0,
                         bool ignoreCase = false) const noexcept;

        /** Searches for a substring within this string.
            Uses a case-sensitive comparison.
            @returns    the index of the first occurrence of this substring, or -1 if it's not found.
                        If textToLookFor is an empty string, this will always return 0.
        */
        int indexOf(StringRef textToLookFor) const noexcept;

        /** Searches for a substring within this string.
            Uses a case-sensitive comparison.
            @param startIndex       the index from which the search should proceed
            @param textToLookFor    the string to search for
            @returns                the index of the first occurrence of this substring, or -1 if it's not found.
                                    If textToLookFor is an empty string, this will always return -1.
        */
        int indexOf(int startIndex, StringRef textToLookFor) const noexcept;

        /** Searches for a substring within this string.
            Uses a case-insensitive comparison.
            @returns    the index of the first occurrence of this substring, or -1 if it's not found.
                        If textToLookFor is an empty string, this will always return 0.
        */
        int indexOfIgnoreCase(StringRef textToLookFor) const noexcept;

        /** Searches for a substring within this string.
            Uses a case-insensitive comparison.
            @param startIndex       the index from which the search should proceed
            @param textToLookFor    the string to search for
            @returns                the index of the first occurrence of this substring, or -1 if it's not found.
                                    If textToLookFor is an empty string, this will always return -1.
        */
        int indexOfIgnoreCase(int startIndex, StringRef textToLookFor) const noexcept;


        int lastIndexOfChar(stl_wchar character) const noexcept;


        int lastIndexOf(StringRef textToLookFor) const noexcept;

        /** Searches for a substring inside this string (working backwards from the end of the string).
            Uses a case-insensitive comparison.
            @returns    the index of the start of the last occurrence of the substring within this string, or -1
                        if it's not found. If textToLookFor is an empty string, this will always return -1.
        */
        int lastIndexOfIgnoreCase(StringRef textToLookFor) const noexcept;

        /** Returns the index of the last character in this string that matches one of the
            characters passed-in to this method.

            This scans the string backwards, starting from its end, and if it finds
            a character that appears in the string charactersToLookFor, it returns its index.

            If none of these characters are found, it returns -1.

            If ignoreCase is true, the comparison will be case-insensitive.

            @see lastIndexOf, indexOfAnyOf
        */
        int lastIndexOfAnyOf(StringRef charactersToLookFor,
                             bool ignoreCase = false) const noexcept;


        //==============================================================================
        // Substring extraction and manipulation methods..


        stl_wchar operator[](int index) const noexcept;

        /** Returns the final character of the string.
            If the string is empty this will return 0.
        */
        stl_wchar getLastCharacter() const noexcept;

        //==============================================================================
        /** Returns a subsection of the string.

            If the range specified is beyond the limits of the string, as much as
            possible is returned.

            @param startIndex   the index of the start of the substring needed
            @param endIndex     all characters from startIndex up to (but not including)
                                this index are returned
            @see fromFirstOccurrenceOf, dropLastCharacters, getLastCharacters, upToFirstOccurrenceOf
        */
        String substring(int startIndex, int endIndex) const;

        /** Returns a section of the string, starting from a given position.

            @param startIndex   the first character to include. If this is beyond the end
                                of the string, an empty string is returned. If it is zero or
                                less, the whole string is returned.
            @returns            the substring from startIndex up to the end of the string
            @see dropLastCharacters, getLastCharacters, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf
        */
        String substring(int startIndex) const;

        /** Returns a version of this string with a number of characters removed
            from the end.

            @param numberToDrop     the number of characters to drop from the end of the
                                    string. If this is greater than the length of the string,
                                    an empty string will be returned. If zero or less, the
                                    original string will be returned.
            @see substring, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf, getLastCharacter
        */
        String dropLastCharacters(int numberToDrop) const;

        /** Returns a number of characters from the end of the string.

            This returns the last numCharacters characters from the end of the string. If the
            string is shorter than numCharacters, the whole string is returned.

            @see substring, dropLastCharacters, getLastCharacter
        */
        String getLastCharacters(int numCharacters) const;

        //==============================================================================
        /** Returns a section of the string starting from a given substring.

            This will search for the first occurrence of the given substring, and
            return the section of the string starting from the point where this is
            found (optionally not including the substring itself).

            e.g. for the string "123456", fromFirstOccurrenceOf ("34", true) would return "3456", and
                                          fromFirstOccurrenceOf ("34", false) would return "56".

            If the substring isn't found, the method will return an empty string.

            If ignoreCase is true, the comparison will be case-insensitive.

            @see upToFirstOccurrenceOf, fromLastOccurrenceOf
        */
        String fromFirstOccurrenceOf(StringRef substringToStartFrom,
                                     bool includeSubStringInResult,
                                     bool ignoreCase) const;

        /** Returns a section of the string starting from the last occurrence of a given substring.

            Similar to fromFirstOccurrenceOf(), but using the last occurrence of the substring, and
            unlike fromFirstOccurrenceOf(), if the substring isn't found, this method will
            return the whole of the original string.

            @see fromFirstOccurrenceOf, upToLastOccurrenceOf
        */
        String fromLastOccurrenceOf(StringRef substringToFind,
                                    bool includeSubStringInResult,
                                    bool ignoreCase) const;

        /** Returns the start of this string, up to the first occurrence of a substring.

            This will search for the first occurrence of a given substring, and then
            return a copy of the string, up to the position of this substring,
            optionally including or excluding the substring itself in the result.

            e.g. for the string "123456", upTo ("34", false) would return "12", and
                                          upTo ("34", true) would return "1234".

            If the substring isn't found, this will return the whole of the original string.

            @see upToLastOccurrenceOf, fromFirstOccurrenceOf
        */
        String upToFirstOccurrenceOf(StringRef substringToEndWith,
                                     bool includeSubStringInResult,
                                     bool ignoreCase) const;

        /** Returns the start of this string, up to the last occurrence of a substring.

            Similar to upToFirstOccurrenceOf(), but this finds the last occurrence rather than the first.
            If the substring isn't found, this will return the whole of the original string.

            @see upToFirstOccurrenceOf, fromFirstOccurrenceOf
        */
        String upToLastOccurrenceOf(StringRef substringToFind,
                                    bool includeSubStringInResult,
                                    bool ignoreCase) const;

        //==============================================================================
        /** Returns a copy of this string with any whitespace characters removed from the start and end. */
        String trim() const;

        /** Returns a copy of this string with any whitespace characters removed from the start. */
        String trimStart() const;

        /** Returns a copy of this string with any whitespace characters removed from the end. */
        String trimEnd() const;

        /** Returns a copy of this string, having removed a specified set of characters from its start.
            Characters are removed from the start of the string until it finds one that is not in the
            specified set, and then it stops.
            @param charactersToTrim     the set of characters to remove.
            @see trim, trimStart, trimCharactersAtEnd
        */
        String trimCharactersAtStart(StringRef charactersToTrim) const;

        /** Returns a copy of this string, having removed a specified set of characters from its end.
            Characters are removed from the end of the string until it finds one that is not in the
            specified set, and then it stops.
            @param charactersToTrim     the set of characters to remove.
            @see trim, trimEnd, trimCharactersAtStart
        */
        String trimCharactersAtEnd(StringRef charactersToTrim) const;

        //==============================================================================
        /** Returns an upper-case version of this string. */
        String toUpperCase() const;

        /** Returns an lower-case version of this string. */
        String toLowerCase() const;

        //==============================================================================
        /** Replaces a sub-section of the string with another string.

            This will return a copy of this string, with a set of characters
            from startIndex to startIndex + numCharsToReplace removed, and with
            a new string inserted in their place.

            Note that this is a const method, and won't alter the string itself.

            @param startIndex               the first character to remove. If this is beyond the bounds of the string,
                                            it will be constrained to a valid range.
            @param numCharactersToReplace   the number of characters to remove. If zero or less, no
                                            characters will be taken out.
            @param stringToInsert           the new string to insert at startIndex after the characters have been
                                            removed.
        */
        String replaceSection(int startIndex,
                              int numCharactersToReplace,
                              StringRef stringToInsert) const;

        /** Replaces all occurrences of a substring with another string.

            Returns a copy of this string, with any occurrences of stringToReplace
            swapped for stringToInsertInstead.

            Note that this is a const method, and won't alter the string itself.
        */
        String replace(StringRef stringToReplace,
                       StringRef stringToInsertInstead,
                       bool ignoreCase = false) const;

        /** Replaces the first occurrence of a substring with another string.

            Returns a copy of this string, with the first occurrence of stringToReplace
            swapped for stringToInsertInstead.

            Note that this is a const method, and won't alter the string itself.
        */
        String replaceFirstOccurrenceOf(StringRef stringToReplace,
                                        StringRef stringToInsertInstead,
                                        bool ignoreCase = false) const;

        /** Returns a string with all occurrences of a character replaced with a different one. */
        String replaceCharacter(stl_wchar characterToReplace,
                                stl_wchar characterToInsertInstead) const;

        /** Replaces a set of characters with another set.

            Returns a string in which each character from charactersToReplace has been replaced
            by the character at the equivalent position in newCharacters (so the two strings
            passed in must be the same length).

            e.g. replaceCharacters ("abc", "def") replaces 'a' with 'd', 'b' with 'e', etc.

            Note that this is a const method, and won't affect the string itself.
        */
        String replaceCharacters(StringRef charactersToReplace,
                                 StringRef charactersToInsertInstead) const;


        String retainCharacters(StringRef charactersToRetain) const;


        String removeCharacters(StringRef charactersToRemove) const;

        /** Returns a section from the start of the string that only contains a certain set of characters.

            This returns the leftmost section of the string, up to (and not including) the
            first character that doesn't appear in the string passed in.
        */
        String initialSectionContainingOnly(StringRef permittedCharacters) const;

        /** Returns a section from the start of the string that only contains a certain set of characters.

            This returns the leftmost section of the string, up to (and not including) the
            first character that occurs in the string passed in. (If none of the specified
            characters are found in the string, the return value will just be the original string).
        */
        String initialSectionNotContaining(StringRef charactersToStopAt) const;

        //==============================================================================
        /** Checks whether the string might be in quotation marks.

            @returns    true if the string begins with a quote character (either a double or single quote).
                        It is also true if there is whitespace before the quote, but it doesn't check the end of the string.
            @see unquoted, quoted
        */
        bool isQuotedString() const;


        String unquoted() const;


        String quoted(stl_wchar quoteCharacter = '"') const;


        //==============================================================================
        /** Creates a string which is a version of a string repeated and joined together.

            @param stringToRepeat         the string to repeat
            @param numberOfTimesToRepeat  how many times to repeat it
        */
        static String repeatedString(StringRef stringToRepeat,
                                     int numberOfTimesToRepeat);

        /** Returns a copy of this string with the specified character repeatedly added to its
            beginning until the total length is at least the minimum length specified.
        */
        String paddedLeft(stl_wchar padCharacter, int minimumLength) const;

        /** Returns a copy of this string with the specified character repeatedly added to its
            end until the total length is at least the minimum length specified.
        */
        String paddedRight(stl_wchar padCharacter, int minimumLength) const;

        /** Creates a string from data in an unknown format.

            This looks at some binary data and tries to guess whether it's Unicode
            or 8-bit characters, then returns a string that represents it correctly.

            Should be able to handle Unicode endianness correctly, by looking at
            the first two bytes.
        */
        static String createStringFromData(const void *data, int size);


        template<typename... Args>
        static String formatted(const String &formatStr, Args... args) {
            return formattedRaw(formatStr.toRawUTF8(), args...);
        }

        /** Returns an iterator pointing at the beginning of the string. */
        CharPointerType begin() const { return getCharPointer(); }


        CharPointerType end() const { return begin().findTerminatingNull(); }

        //==============================================================================
        // Numeric conversions..

        /** Creates a string containing this signed 32-bit integer as a decimal number.
            @see getIntValue, getFloatValue, getDoubleValue, toHexString
        */
        explicit String(int decimalInteger);

        explicit String(unsigned int decimalInteger);

        explicit String(short decimalInteger);

        explicit String(unsigned short decimalInteger);


        explicit String(int64 largeIntegerValue);

        explicit String(uint64 largeIntegerValue);

        /** Creates a string containing this signed long integer as a decimal number.
            @see getIntValue, getFloatValue, getDoubleValue, toHexString
        */
        explicit String(long decimalInteger);

        /** Creates a string containing this unsigned long integer as a decimal number.
            @see getIntValue, getFloatValue, getDoubleValue, toHexString
        */
        explicit String(unsigned long decimalInteger);

        /** Creates a string representing this floating-point number.
            @param floatValue               the value to convert to a string
            @see getDoubleValue, getIntValue
        */
        explicit String(float floatValue);

        /** Creates a string representing this floating-point number.
            @param doubleValue              the value to convert to a string
            @see getFloatValue, getIntValue
        */
        explicit String(double doubleValue);

        String(float floatValue, int numberOfDecimalPlaces, bool useScientificNotation = false);

        String(double doubleValue, int numberOfDecimalPlaces, bool useScientificNotation = false);

#ifndef DOXYGEN

        // Automatically creating a String from a bool opens up lots of nasty type conversion edge cases.
        // If you want a String representation of a bool you can cast the bool to an int first.
        explicit String(bool) = delete;

#endif

        int getIntValue() const noexcept;

        /** Reads the value of the string as a decimal number (up to 64 bits in size).
            @returns the value of the string as a 64 bit signed base-10 integer.
        */
        int64 getLargeIntValue() const noexcept;


        int getTrailingIntValue() const noexcept;

        float getFloatValue() const noexcept;

        double getDoubleValue() const noexcept;


        int getHexValue32() const noexcept;


        int64 getHexValue64() const noexcept;

        /** Returns a string representing this numeric value in hexadecimal. */
        template<typename IntegerType>
        static String toHexString(IntegerType number) { return createHex(number); }


        static String toHexString(const void *data, int size, int groupSize = 1);

        /** Returns a string containing a decimal with a set number of significant figures.

            @param number                         the input number
            @param numberOfSignificantFigures     the number of significant figures to use
        */
        template<typename DecimalType>
        static String toDecimalStringWithSignificantFigures(DecimalType number, int numberOfSignificantFigures) {
            jassert (numberOfSignificantFigures > 0);

            if (exactlyEqual(number, DecimalType())) {
                if (numberOfSignificantFigures > 1) {
                    String result("0.0");

                    for (int i = 2; i < numberOfSignificantFigures; ++i)
                        result += "0";

                    return result;
                }

                return "0";
            }

            auto numDigitsBeforePoint = (int) std::ceil(std::log10(number < 0 ? -number : number));

            auto shift = numberOfSignificantFigures - numDigitsBeforePoint;
            auto factor = std::pow(10.0, shift);
            auto rounded = std::round(number * factor) / factor;

            std::stringstream ss;
            ss << std::fixed << std::setprecision(std::max(shift, 0)) << rounded;
            return ss.str();
        }

        //==============================================================================
        /** Returns the character pointer currently being used to store this string.

            Because it returns a reference to the string's internal data, the pointer
            that is returned must not be stored anywhere, as it can be deleted whenever the
            string changes.
        */
        CharPointerType getCharPointer() const noexcept { return text; }


        CharPointer_UTF8 toUTF8() const;


        const char *toRawUTF8() const;


        const char *c_str() const;


        CharPointer_UTF16 toUTF16() const;


        CharPointer_UTF32 toUTF32() const;


        const wchar_t *toWideCharPointer() const;

        /** */
        std::string toStdString() const;

        std::string toStdStringV2() const {
            return std::string(toRawUTF8());
        };

        std::string stringV2() const {
            return toStdStringV2();
        }

        std::string string() const { return toStdString(); };


        size_t countOfChar(const char c) const;

        size_t countOfWChar(const wchar_t c) const;


        static String fromUTF8(const char *utf8buffer, int bufferSizeBytes = -1);


        static String stringPrintf(const char *fmt, ...)__attribute__((__format__(__printf__, 1, 2)));

        template<typename... T>
        inline static String fmtPrintf(fmt::format_string<T...> fmt, T &&... args) {
            auto format = fmt::vformat(fmt, fmt::make_format_args(args...));
            return format;
        }

        static void StringAppendV(stl::String *dst, const char *format, va_list ap)
        __attribute__((__format__(__printf__, 2, 0)));


        size_t getNumBytesAsUTF8() const noexcept;


        size_t copyToUTF8(CharPointer_UTF8::CharType *destBuffer, size_t maxBufferSizeBytes) const noexcept;

        size_t copyToUTF16(CharPointer_UTF16::CharType *destBuffer, size_t maxBufferSizeBytes) const noexcept;

        size_t copyToUTF32(CharPointer_UTF32::CharType *destBuffer, size_t maxBufferSizeBytes) const noexcept;


        void preallocateBytes(size_t numBytesNeeded);

        /** Swaps the contents of this string with another one.
            This is a very fast operation, as no allocation or copying needs to be done.
        */
        void swapWith(String &other) noexcept;

        //==============================================================================
#if STL_MAC || STL_IOS || DOXYGEN
        /** OSX ONLY - Creates a String from an OSX CFString. */
        static String fromCFString (CFStringRef cfString);

        /** OSX ONLY - Converts this string to a CFString.
            Remember that you must use CFRelease() to free the returned string when you're
            finished with it.
        */
        CFStringRef toCFString() const;

        /** OSX ONLY - Returns a copy of this string in which any decomposed unicode characters have
            been converted to their precomposed equivalents. */
        String convertToPrecomposedUnicode() const;
#endif


        int getReferenceCount() const noexcept;

        //==============================================================================
#if STL_ALLOW_STATIC_NULL_VARIABLES && !defined (DOXYGEN)
        [[deprecated ("This was a static empty string object, but is now deprecated as it's too easy to accidentally "
                     "use it indirectly during a static constructor, leading to hard-to-find order-of-initialisation "
                     "problems. If you need an empty String object, just use String() or {}. For returning an empty "
                     "String from a function by reference, use a function-local static String object and return that.")]]
        static const String empty;
#endif
    private:
        //==============================================================================
        CharPointerType text;

        //==============================================================================
        struct PreallocationBytes {
            explicit PreallocationBytes(size_t) noexcept;

            size_t numBytes;
        };

        explicit String(const PreallocationBytes &); // This constructor preallocates a certain amount of memory
        size_t getByteOffsetOfEnd() const noexcept;

        // This private cast operator should prevent strings being accidentally cast
        // to bools (this is possible because the compiler can add an implicit cast
        // via a const char*)
        operator bool() const noexcept { return false; }

        //==============================================================================
        static String formattedRaw(const char *, ...);

        static String createHex(uint8);

        static String createHex(uint16);

        static String createHex(uint32);

        static String createHex(uint64);

        template<typename Type>
        static String createHex(Type n) {
            return createHex(static_cast<typename TypeHelpers::UnsignedTypeWithSize<(int) sizeof(n)>::type> (n));
        }
    };

//==============================================================================
/** Concatenates two strings. */
    String STL_CALLTYPE operator+(const char *string1, const String &string2);

/** Concatenates two strings. */
    String STL_CALLTYPE operator+(const wchar_t *string1, const String &string2);

/** Concatenates two strings. */
    String STL_CALLTYPE operator+(char string1, const String &string2);

/** Concatenates two strings. */
    String STL_CALLTYPE operator+(wchar_t string1, const String &string2);

#if !STL_NATIVE_WCHAR_IS_UTF32

    /** Concatenates two strings. */
    String STL_CALLTYPE operator+(stl_wchar string1, const String &string2);

#endif

/** Concatenates two strings. */
    String STL_CALLTYPE operator+(String string1, const String &string2);

/** Concatenates two strings. */
    String STL_CALLTYPE operator+(String string1, const char *string2);

/** Concatenates two strings. */
    String STL_CALLTYPE operator+(String string1, const wchar_t *string2);

/** Concatenates two strings. */
    String STL_CALLTYPE operator+(String string1, char characterToAppend);

/** Concatenates two strings. */
    String STL_CALLTYPE operator+(String string1, wchar_t characterToAppend);

#if !STL_NATIVE_WCHAR_IS_UTF32

    /** Concatenates two strings. */
    String STL_CALLTYPE operator+(String string1, stl_wchar characterToAppend);

#endif

//==============================================================================
/** Appends a character at the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, char characterToAppend);

/** Appends a character at the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, wchar_t characterToAppend);

#if !STL_NATIVE_WCHAR_IS_UTF32

    /** Appends a character at the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, stl_wchar characterToAppend);

#endif

/** Appends a string to the end of the first one. */
    String &STL_CALLTYPE operator<<(String &string1, const char *string2);

/** Appends a string to the end of the first one. */
    String &STL_CALLTYPE operator<<(String &string1, const wchar_t *string2);

/** Appends a string to the end of the first one. */
    String &STL_CALLTYPE operator<<(String &string1, const String &string2);

/** Appends a string to the end of the first one. */
    String &STL_CALLTYPE operator<<(String &string1, StringRef string2);

/** Appends a string to the end of the first one. */
    inline String &
    STL_CALLTYPE operator<<(String &string1, const std::string &string2) { return string1 += string2.c_str(); }

/** Appends a decimal number to the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, uint8 number);

/** Appends a decimal number to the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, short number);

/** Appends a decimal number to the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, int number);

/** Appends a decimal number to the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, long number);

/** Appends a decimal number to the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, unsigned long number);

/** Appends a decimal number to the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, int64 number);

/** Appends a decimal number to the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, uint64 number);

/** Appends a decimal number to the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, float number);

/** Appends a decimal number to the end of a string. */
    String &STL_CALLTYPE operator<<(String &string1, double number);

#ifndef DOXYGEN

// Automatically creating a String from a bool opens up lots of nasty type conversion edge cases.
// If you want a String representation of a bool you can cast the bool to an int first.
    String &STL_CALLTYPE operator<<(String &, bool) = delete;

#endif

    inline String STL_CALLTYPE operator+(String s1, const std::string &s2) { return s1 += s2.c_str(); }
//==============================================================================
/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator==(const String &string1, const String &string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator==(const String &string1, const char *string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator==(const String &string1, const wchar_t *string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator==(const String &string1, CharPointer_UTF8 string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator==(const String &string1, CharPointer_UTF16 string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator==(const String &string1, CharPointer_UTF32 string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator!=(const String &string1, const String &string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator!=(const String &string1, const char *string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator!=(const String &string1, const wchar_t *string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator!=(const String &string1, CharPointer_UTF8 string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator!=(const String &string1, CharPointer_UTF16 string2) noexcept;

/** Case-sensitive comparison of two strings. */
    bool STL_CALLTYPE operator!=(const String &string1, CharPointer_UTF32 string2) noexcept;

//==============================================================================

    template<class traits>
    std::basic_ostream<char, traits> &
    STL_CALLTYPE operator<<(std::basic_ostream<char, traits> &stream, const String &stringToWrite) {
        return stream << stringToWrite.toRawUTF8();
    }

    /** This operator allows you to write a juce String directly to std output streams.
        This is handy for writing strings to std::wcout, std::wcerr, etc.
    */
    template<class traits>
    std::basic_ostream<wchar_t, traits> &
    STL_CALLTYPE operator<<(std::basic_ostream<wchar_t, traits> &stream, const String &stringToWrite) {
        return stream << stringToWrite.toWideCharPointer();
    }

    /** Writes a string to an OutputStream as UTF8. */
    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, const String &stringToWrite);

    /** Writes a string to an OutputStream as UTF8. */
    OutputStream &STL_CALLTYPE operator<<(OutputStream &stream, StringRef stringToWrite);

    inline std::string stl::String::toString() const {
        return toStdString();
    }


} // namespace stl

//add format for String
template<>
struct stl::fmt::v9::formatter<stl::String> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const stl::String &data, FormatContext &ctx) {
        return fmt::v9::format_to(ctx.out(), "{}", data.toRawUTF8());
    }
};

#if defined(STL_MAC) or defined(__loongarch__)
namespace std {
    template <>
    struct hash<stl::String> {
        size_t operator()(const stl::String& s) const {
            // Implement a hash function for your String class
            // You can use std::hash on its internal string or create a custom hash algorithm
            return s.hashCode64(); // Assuming getInternalString() retrieves the internal string
        }
    };
}
#endif
#ifndef DOXYGEN
namespace std {
}
#endif
