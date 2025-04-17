/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/10 17:59
 * @version: 1.0
 * @description: 
*/

#include "text/stl_NumberParser.h"
#include "text/stl_Ascii.h"
#include <stdexcept>
#include "stl_core.h"
#include "text/stl_NumericString.h"
#include "text/stl_String.h"

namespace stl {

    int NumberParser::parse(const String &s, char thSep) {
        int result;
        if (tryParse(s, result, thSep))
            return result;
        else
            throw std::invalid_argument("Not a valid integer");
    }


    bool NumberParser::tryParse(const String &s, int &value, char thSep) {
        return strToInt(s.c_str(), value, NUM_BASE_DEC, thSep);
    }

    unsigned NumberParser::parseUnsigned(const String &s, char thSep) {
        unsigned result;
        if (tryParseUnsigned(s, result, thSep))
            return result;
        else
            throw std::invalid_argument("Not a valid unsigned integer");
    }


    bool NumberParser::tryParseUnsigned(const String &s, unsigned &value, char thSep) {
        return strToInt(s.c_str(), value, NUM_BASE_DEC, thSep);
    }


    unsigned NumberParser::parseHex(const String &s) {
        unsigned result;
        if (tryParseHex(s, result))
            return result;
        else
            throw std::invalid_argument("Not a valid hexadecimal integer");
    }


    bool NumberParser::tryParseHex(const String &s, unsigned &value) {
        int offset = 0;
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) offset = 2;
        return strToInt(s.c_str() + offset, value, NUM_BASE_HEX);
    }


    unsigned NumberParser::parseOct(const String &s) {
        unsigned result;
        if (tryParseOct(s, result))
            return result;
        else
            throw std::invalid_argument("Not a valid hexadecimal integer");
    }


    bool NumberParser::tryParseOct(const String &s, unsigned &value) {
        return strToInt(s.c_str(), value, NUM_BASE_OCT);
    }


    int64_t NumberParser::parse64(const String &s, char thSep) {
        ino64_t result;
        if (tryParse64(s, result, thSep))
            return result;
        else
            throw std::invalid_argument("Not a valid integer");
    }


    bool NumberParser::tryParse64(const String &s, ino64_t &value, char thSep) {
        return strToInt(s.c_str(), value, NUM_BASE_DEC, thSep);
    }


    uint64_t NumberParser::parseUnsigned64(const String &s, char thSep) {
        uint64_t result;
        if (tryParseUnsigned64(s, result, thSep))
            return result;
        else
            throw std::invalid_argument("Not a valid unsigned integer");
    }


    bool NumberParser::tryParseUnsigned64(const String &s, uint64_t &value, char thSep) {
        return strToInt(s.c_str(), value, NUM_BASE_DEC, thSep);
    }


    uint64_t NumberParser::parseHex64(const String &s) {
        uint64_t result;
        if (tryParseHex64(s, result))
            return result;
        else
            throw std::invalid_argument("Not a valid hexadecimal integer");
    }


    bool NumberParser::tryParseHex64(const String &s, uint64_t &value) {
        int offset = 0;
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) offset = 2;
        return strToInt(s.c_str() + offset, value, NUM_BASE_HEX);
    }


    uint64_t NumberParser::parseOct64(const String &s) {
        uint64_t result;
        if (tryParseOct64(s, result))
            return result;
        else
            throw std::invalid_argument("Not a valid hexadecimal integer");
    }


    bool NumberParser::tryParseOct64(const String &s, uint64_t &value) {
        return strToInt(s.c_str(), value, NUM_BASE_OCT);
    }


    double NumberParser::parseFloat(const String &s, char decSep, char thSep) {
        double result;
        if (tryParseFloat(s, result, decSep, thSep))
            return result;
        else
            throw std::invalid_argument("Not a valid floating-point number");
    }


    bool NumberParser::tryParseFloat(const String &s, double &value, char decSep, char thSep) {
        return strToDouble(s.c_str(), value, decSep, thSep);
    }


    bool NumberParser::parseBool(const String &s) {
        bool result;
        if (tryParseBool(s, result))
            return result;
        else
            throw std::invalid_argument("Not a valid bool number");
    }


    bool NumberParser::tryParseBool(const String &s, bool &value) {
        int n;
        if (NumberParser::tryParse(s, n)) {
            value = (n != 0);
            return true;
        }
        stl::String data = s;
        if (data.compareIgnoreCase("ture") == 0) {
            value = true;
            return true;
        } else if (data.compareIgnoreCase("yes") == 0) {
            value = true;
            return true;
        } else if (data.compareIgnoreCase("on") == 0) {
            value = true;
            return true;
        }
        if (data.compareIgnoreCase("false") == 0) {
            value = false;
            return true;
        } else if (data.compareIgnoreCase("no") == 0) {
            value = false;
            return true;
        } else if (data.compareIgnoreCase("off") == 0) {
            value = false;
            return true;
        }

        return false;
    }
}