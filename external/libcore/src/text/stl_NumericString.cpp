/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/10 18:13
 * @version: 1.0
 * @description: 
*/
#include "text/stl_NumericString.h"
#include <memory>

#define double_conversion poco_double_conversion
#define UNIMPLEMENTED poco_bugcheck

#include "double-conversion.h"
#include "cached-powers.cc"
#include "bignum-dtoa.cc"
#include "bignum.cc"
#include "fast-dtoa.cc"
#include "fixed-dtoa.cc"
#include "strtod.cc"
#include "double-to-string.cc"
#include "string-to-double.cc"
#include "text/stl_Ascii.h"

namespace {
    void pad(std::string &str, int precision, int width, char prefix = ' ', char decSep = '.')
    /// Pads the string with prefix space and postfix 0.
    /// Alternative prefix (e.g. zero instead of space) can be supplied by caller.
    /// Used only internally.
    {

        std::string::size_type decSepPos = str.find(decSep);
        if (decSepPos == std::string::npos) {
            str.append(1, decSep);
            decSepPos = str.size() - 1;
        }

        std::string::size_type frac = str.length() - decSepPos - 1;

        std::string::size_type ePos = str.find_first_of("eE");
        std::unique_ptr<std::string> eStr;
        if (ePos != std::string::npos) {
            eStr.reset(new std::string(str.substr(ePos, std::string::npos)));
            frac -= eStr->length();
            str = str.substr(0, str.length() - eStr->length());
        }

        if (frac != precision) {
            if (frac < precision) {
                str.append(precision - frac, '0');
            } else if ((frac > precision) && (decSepPos != std::string::npos)) {
                int pos = static_cast<int>(decSepPos) + 1 + precision;
                if (str[pos] >= '5') // we must round up
                {
                    char carry = 0;
                    if (str[--pos] == '9') {
                        str[pos] = '0';
                        carry = 1;
                    } else {
                        ++str[pos];
                        carry = 0;
                    }
                    while (--pos >= 0) {
                        if (str[pos] == decSep) continue;
                        if (carry) {
                            if ((str[pos] + carry) <= '9') {
                                ++str[pos];
                                carry = 0;
                            } else {
                                str[pos] = '0';
                                carry = 1;
                            }
                        }
                    }
                    if (carry) str.insert(str.begin(), 1, '1');
                }
                str = str.substr(0, decSepPos + 1 + precision);
            }
        }

        if (eStr.get()) str += *eStr;

        if (width && (str.length() < width)) str.insert(str.begin(), width - str.length(), prefix);
    }


    void insertThousandSep(std::string &str, char thSep, char decSep = '.')
    /// Inserts thousand separators.
    /// Used only internally.
    {

        if (str.size() == 0) return;

        std::string::size_type exPos = str.find('e');
        if (exPos == std::string::npos) exPos = str.find('E');
        std::string::size_type decPos = str.find(decSep);
        // there's no rinsert, using forward iterator to go backwards
        std::string::iterator it = str.end();
        if (exPos != std::string::npos) it -= str.size() - exPos;

        if (decPos != std::string::npos) {
            while (it != str.begin()) {
                --it;
                if (*it == decSep) break;
            }
        }
        int thCount = 0;
        if (it == str.end()) --it;
        for (; it != str.begin();) {
            std::string::iterator pos = it;
            std::string::value_type chr = *it;
            std::string::value_type prevChr = *--it;

            if (!std::isdigit(chr)) continue;

            if (++thCount == 3 && std::isdigit(prevChr))
                it = str.insert(pos, thSep);

            if (thCount == 3) thCount = 0;
        }
    }


} // namespace

namespace stl {
    template<class S>
    static S &trimInPlace(S &str)
    /// Removes all leading and trailing whitespace in str.
    {
        std::ptrdiff_t first = 0;
        std::ptrdiff_t last = static_cast<std::ptrdiff_t>(str.size()) - 1;

        while (first <= last && Ascii::isSpace(str[first])) ++first;
        while (last >= first && Ascii::isSpace(str[last])) --last;

        if (last >= 0) {
            str.resize(last + 1);
            str.erase(0, first);
        }
        return str;
    }


    template<class S>
    S &replaceInPlace(S &str, const S &from, const S &to, typename S::size_type start = 0) {
        poco_assert(from.size() > 0);

        S result;
        typename S::size_type pos = 0;
        result.append(str, 0, start);
        do {
            pos = str.find(from, start);
            if (pos != S::npos) {
                result.append(str, start, pos - start);
                result.append(to);
                start = pos + from.length();
            } else result.append(str, start, str.size() - start);
        } while (pos != S::npos);
        str.swap(result);
        return str;
    }


    template<class S>
    S &replaceInPlace(S &str, const typename S::value_type *from, const typename S::value_type *to,
                      typename S::size_type start = 0) {
        poco_assert(*from);

        S result;
        typename S::size_type pos = 0;
        typename S::size_type fromLen = std::strlen(from);
        result.append(str, 0, start);
        do {
            pos = str.find(from, start);
            if (pos != S::npos) {
                result.append(str, start, pos - start);
                result.append(to);
                start = pos + fromLen;
            } else result.append(str, start, str.size() - start);
        } while (pos != S::npos);
        str.swap(result);
        return str;
    }


    template<class S>
    S &replaceInPlace(S &str, const typename S::value_type from, const typename S::value_type to = 0,
                      typename S::size_type start = 0) {
        if (from == to) return str;

        typename S::size_type pos = 0;
        do {
            pos = str.find(from, start);
            if (pos != S::npos) {
                if (to) str[pos] = to;
                else str.erase(pos, 1);
            }
        } while (pos != S::npos);

        return str;
    }


    void floatToStr(char *buffer, int bufferSize, float value, int lowDec, int highDec) {
        using namespace double_conversion;

        StringBuilder builder(buffer, bufferSize);
        int flags = DoubleToStringConverter::UNIQUE_ZERO |
                    DoubleToStringConverter::EMIT_POSITIVE_EXPONENT_SIGN;
        DoubleToStringConverter dc(flags, POCO_FLT_INF, POCO_FLT_NAN, POCO_FLT_EXP, lowDec, highDec, 0, 0);
        dc.ToShortestSingle(value, &builder);
        builder.Finalize();
    }


    void floatToFixedStr(char *buffer, int bufferSize, float value, int precision) {
        using namespace double_conversion;

        StringBuilder builder(buffer, bufferSize);
        int flags = DoubleToStringConverter::UNIQUE_ZERO |
                    DoubleToStringConverter::EMIT_POSITIVE_EXPONENT_SIGN;
        DoubleToStringConverter dc(flags, POCO_FLT_INF, POCO_FLT_NAN, POCO_FLT_EXP,
                                   -std::numeric_limits<float>::digits10, std::numeric_limits<float>::digits10, 0, 0);
        dc.ToFixed(value, precision, &builder);
        builder.Finalize();
    }


    String floatToStr(String &str_, float value, int precision, int width, char thSep, char decSep) {
        if (!decSep) decSep = '.';
        if (precision == 0) value = std::floor(value);
        auto str = str_.toStdString();
        char buffer[POCO_MAX_FLT_STRING_LEN];
        floatToStr(buffer, POCO_MAX_FLT_STRING_LEN, value);
        str = buffer;

        if (decSep && (decSep != '.') && (str.find('.') != std::string::npos))
            replaceInPlace(str, '.', decSep);

        if (thSep) insertThousandSep(str, thSep, decSep);
        if (precision > 0 || width) pad(str, precision, width, ' ', decSep ? decSep : '.');
        return str;
    }


    String floatToFixedStr(String &str_, float value, int precision, int width, char thSep, char decSep) {
        if (!decSep) decSep = '.';
        if (precision == 0) value = std::floor(value);
        auto str=str_.toStdString();
        char buffer[POCO_MAX_FLT_STRING_LEN];
        floatToFixedStr(buffer, POCO_MAX_FLT_STRING_LEN, value, precision);
        str = buffer;

        if (decSep && (decSep != '.') && (str.find('.') != std::string::npos))
            replaceInPlace(str, '.', decSep);

        if (thSep) insertThousandSep(str, thSep, decSep);
        if (precision > 0 || width) pad(str, precision, width, ' ', decSep ? decSep : '.');
        return str;
    }


    void doubleToStr(char *buffer, int bufferSize, double value, int lowDec, int highDec) {
        using namespace double_conversion;

        StringBuilder builder(buffer, bufferSize);
        int flags = DoubleToStringConverter::UNIQUE_ZERO |
                    DoubleToStringConverter::EMIT_POSITIVE_EXPONENT_SIGN;
        DoubleToStringConverter dc(flags, POCO_FLT_INF, POCO_FLT_NAN, POCO_FLT_EXP, lowDec, highDec, 0, 0);
        dc.ToShortest(value, &builder);
        builder.Finalize();
    }


    void doubleToFixedStr(char *buffer, int bufferSize, double value, int precision) {
        using namespace double_conversion;

        StringBuilder builder(buffer, bufferSize);
        int flags = DoubleToStringConverter::UNIQUE_ZERO |
                    DoubleToStringConverter::EMIT_POSITIVE_EXPONENT_SIGN;
        DoubleToStringConverter dc(flags, POCO_FLT_INF, POCO_FLT_NAN, POCO_FLT_EXP,
                                   -std::numeric_limits<double>::digits10, std::numeric_limits<double>::digits10, 0, 0);
        dc.ToFixed(value, precision, &builder);
        builder.Finalize();
    }


    String doubleToStr(String &str_, double value, int precision, int width, char thSep, char decSep) {
        if (!decSep) decSep = '.';
        if (precision == 0) value = std::floor(value);
        auto str = str_.toStdString();
        char buffer[POCO_MAX_FLT_STRING_LEN];
        doubleToStr(buffer, POCO_MAX_FLT_STRING_LEN, value);

        str = buffer;

        if (decSep && (decSep != '.') && (str.find('.') != std::string::npos))
            replaceInPlace(str, '.', decSep);

        if (thSep) insertThousandSep(str, thSep, decSep);
        if (precision > 0 || width) pad(str, precision, width, ' ', decSep ? decSep : '.');
        return str;
    }


    String doubleToFixedStr(String &str_, double value, int precision, int width, char thSep, char decSep) {
        if (!decSep) decSep = '.';
        if (precision == 0) value = std::floor(value);
        auto str = str_.toStdString();
        char buffer[POCO_MAX_FLT_STRING_LEN];
        doubleToFixedStr(buffer, POCO_MAX_FLT_STRING_LEN, value, precision);

        str = buffer;

        if (decSep && (decSep != '.') && (str.find('.') != std::string::npos))
            replaceInPlace(str, '.', decSep);

        if (thSep) insertThousandSep(str, thSep, decSep);
        if (precision > 0 || width) pad(str, precision, width, ' ', decSep ? decSep : '.');
        String res(str);
        return res;
    }


    float strToFloat(const char *str, const char *inf, const char *nan) {
        using namespace double_conversion;

        int processed;
        int flags = StringToDoubleConverter::ALLOW_LEADING_SPACES |
                    StringToDoubleConverter::ALLOW_TRAILING_SPACES;
        StringToDoubleConverter converter(flags, 0.0, Single::NaN(), inf, nan);
        float result = converter.StringToFloat(str, static_cast<int>(strlen(str)), &processed);
        return result;
    }


    double strToDouble(const char *str, const char *inf, const char *nan) {
        using namespace double_conversion;
        int processed;
        int flags = StringToDoubleConverter::ALLOW_LEADING_SPACES |
                    StringToDoubleConverter::ALLOW_TRAILING_SPACES;
        StringToDoubleConverter converter(flags, 0.0, Double::NaN(), inf, nan);
        double result = converter.StringToDouble(str, static_cast<int>(strlen(str)), &processed);
        return result;
    }

    template<class S>
    S &removeInPlace(S &str, const typename S::value_type ch, typename S::size_type start = 0) {
        return replaceInPlace(str, ch, 0, start);
    }

    template<class T>
    inline bool isInfiniteImpl(T value) {

        return std::isinf((T) value) != 0;

    }

    template<class T>
    inline bool isNaNImpl(T value) {

        return std::isnan((T) value) != 0;

    }

    bool strToFloat(const String &str, float &result, char decSep, char thSep, const char *inf, const char *nan) {
        using namespace double_conversion;

        std::string tmp(str.c_str());
        trimInPlace(tmp);
        removeInPlace(tmp, thSep);
        removeInPlace(tmp, 'f');
        replaceInPlace(tmp, decSep, '.');
        result = strToFloat(tmp.c_str(), inf, nan);
        return !isInfiniteImpl(result) &&
               !isNaNImpl(result);
    }


    bool
    strToDouble(const String &str, double &result, char decSep, char thSep, const char *inf, const char *nan) {
        if (str.isEmpty()) return false;

        using namespace double_conversion;
        std::string tmp(str.c_str());
        trimInPlace(tmp);
        removeInPlace(tmp, thSep);
        replaceInPlace(tmp, decSep, '.');
        removeInPlace(tmp, 'f');
        result = strToDouble(tmp.c_str(), inf, nan);
        return !isInfiniteImpl(result) &&
               !isNaNImpl(result);
    }
}