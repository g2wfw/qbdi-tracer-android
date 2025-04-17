/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/10 17:57
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STL_NUMBERPARSER_H
#define MANXI_CORE_STL_NUMBERPARSER_H

#include <string>
#include <cstdint>
#include "stl_core.h"
#include "stl_String.h"

#ifdef STL_MAC
#define ino64_t ino_t
#endif
#ifdef __MINGW32__
#define ino64_t uint64_t
#endif

namespace stl {

    class STL_EXPORT NumberParser
        /// The NumberParser class provides static methods
        /// for parsing numbers out of strings.
        ///
        /// Note that leading or trailing whitespace is not allowed
        /// in the string. Poco::trim() or Poco::trimInPlace()
        /// can be used to remove leading or trailing whitespace.
    {
    public:
        static const unsigned short NUM_BASE_OCT = 010;
        static const unsigned short NUM_BASE_DEC = 10;
        static const unsigned short NUM_BASE_HEX = 0x10;

        static int parse(const String &s, char thousandSeparator = ',');
        /// Parses an integer value in decimal notation from the given string.
        /// Throws a SyntaxException if the string does not hold a number in decimal notation.

        static bool tryParse(const String &s, int &value, char thousandSeparator = ',');
        /// Parses an integer value in decimal notation from the given string.
        /// Returns true if a valid integer has been found, false otherwise.
        /// If parsing was not successful, value is undefined.

        static unsigned parseUnsigned(const String &s, char thousandSeparator = ',');
        /// Parses an unsigned integer value in decimal notation from the given string.
        /// Throws a SyntaxException if the string does not hold a number in decimal notation.

        static bool tryParseUnsigned(const String &s, unsigned &value, char thousandSeparator = ',');
        /// Parses an unsigned integer value in decimal notation from the given string.
        /// Returns true if a valid integer has been found, false otherwise.
        /// If parsing was not successful, value is undefined.

        static unsigned parseHex(const String &s);
        /// Parses an integer value in hexadecimal notation from the given string.
        /// Throws a SyntaxException if the string does not hold a number in
        /// hexadecimal notation.

        static bool tryParseHex(const String &s, unsigned &value);
        /// Parses an unsigned integer value in hexadecimal notation from the given string.
        /// Returns true if a valid integer has been found, false otherwise.
        /// If parsing was not successful, value is undefined.

        static unsigned parseOct(const String &s);
        /// Parses an integer value in octal notation from the given string.
        /// Throws a SyntaxException if the string does not hold a number in
        /// hexadecimal notation.

        static bool tryParseOct(const String &s, unsigned &value);
        /// Parses an unsigned integer value in octal notation from the given string.
        /// Returns true if a valid integer has been found, false otherwise.
        /// If parsing was not successful, value is undefined.



        static int64_t parse64(const String &s, char thousandSeparator = ',');
/// Parses a 64-bit integer value in decimal notation from the given string.
/// Throws a SyntaxException if the string does not hold a number in decimal notation.

        static bool tryParse64(const String &s, ino64_t &value, char thousandSeparator = ',');
/// Parses a 64-bit integer value in decimal notation from the given string.
/// Returns true if a valid integer has been found, false otherwise.
/// If parsing was not successful, value is undefined.

        static uint64_t parseUnsigned64(const String &s, char thousandSeparator = ',');
/// Parses an unsigned 64-bit integer value in decimal notation from the given string.
/// Throws a SyntaxException if the string does not hold a number in decimal notation.

        static bool tryParseUnsigned64(const String &s, uint64_t &value, char thousandSeparator = ',');
/// Parses an unsigned 64-bit integer value in decimal notation from the given string.
/// Returns true if a valid integer has been found, false otherwise.
/// If parsing was not successful, value is undefined.

        static uint64_t parseHex64(const String &s);
/// Parses a 64 bit-integer value in hexadecimal notation from the given string.
/// Throws a SyntaxException if the string does not hold a number in hexadecimal notation.

        static bool tryParseHex64(const String &s, uint64_t &value);
/// Parses an unsigned 64-bit integer value in hexadecimal notation from the given string.
/// Returns true if a valid integer has been found, false otherwise.
/// If parsing was not successful, value is undefined.

        static uint64_t parseOct64(const String &s);
/// Parses a 64 bit-integer value in octal notation from the given string.
/// Throws a SyntaxException if the string does not hold a number in hexadecimal notation.

        static bool tryParseOct64(const String &s, uint64_t &value);
/// Parses an unsigned 64-bit integer value in octal notation from the given string.
/// Returns true if a valid integer has been found, false otherwise.
/// If parsing was not successful, value is undefined.



        static double parseFloat(const String &s, char decimalSeparator = '.', char thousandSeparator = ',');
        /// Parses a double value in decimal floating point notation
        /// from the given string.
        /// Throws a SyntaxException if the string does not hold a floating-point
        /// number in decimal notation.

        static bool
        tryParseFloat(const String &s, double &value, char decimalSeparator = '.', char thousandSeparator = ',');
        /// Parses a double value in decimal floating point notation
        /// from the given string.
        /// Returns true if a valid floating point number has been found,
        /// false otherwise.
        /// If parsing was not successful, value is undefined.

        static bool parseBool(const String &s);
        /// Parses a bool value in decimal or string notation
        /// from the given string.
        /// Valid forms are: "0", "1", "true", "on", false", "yes", "no", "off".
        /// String forms are NOT case sensitive.
        /// Throws a SyntaxException if the string does not hold a valid bool number

        static bool tryParseBool(const String &s, bool &value);
        /// Parses a bool value in decimal or string notation
        /// from the given string.
        /// Valid forms are: "0", "1", "true", "on", false", "yes", "no", "off".
        /// String forms are NOT case sensitive.
        /// Returns true if a valid bool number has been found,
        /// false otherwise.
        /// If parsing was not successful, value is undefined.
    };
}
#endif //MANXI_CORE_STL_NUMBERPARSER_H
