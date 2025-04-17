/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/10 16:58
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STRING_TOKENIZER_H
#define MANXI_CORE_STRING_TOKENIZER_H

#include <vector>
#include <string>
#include <exception>
#include <stdexcept>
#include "stl_String.h"

namespace stl {
    class String;

    class STL_EXPORT StringTokenizer {
    public:
        enum Options {
            TOK_IGNORE_EMPTY = 1, /// ignore empty tokens
            TOK_TRIM = 2  /// remove leading and trailing whitespace from tokens
        };
        typedef std::vector<std::string> TokenVec;
        typedef TokenVec::const_iterator Iterator;

        StringTokenizer(const String &str, const String &separators, int options = 0);
        /// Splits the given string into tokens. The tokens are expected to be
        /// separated by one of the separator characters given in separators.
        /// Additionally, options can be specified:
        ///   * TOK_IGNORE_EMPTY: empty tokens are ignored
        ///   * TOK_TRIM: trailing and leading whitespace is removed from tokens.

        ~StringTokenizer();
        /// Destroys the tokenizer.

        Iterator begin() const;

        Iterator end() const;

        const std::string &operator[](std::size_t index) const;
        /// Returns const reference the index'th token.
        /// Throws a RangeException if the index is out of range.

        std::string &operator[](std::size_t index);
        /// Returns reference to the index'th token.
        /// Throws a RangeException if the index is out of range.

        ALWAYS_INLINE bool has(const std::string &token) const {
            auto it = std::find(_tokens.begin(), _tokens.end(), token);
            return it != _tokens.end();
        }
        /// Returns true if token exists, false otherwise.

        ALWAYS_INLINE  std::string::size_type find(const std::string &token, std::string::size_type pos = 0) const {
            auto it = std::find(_tokens.begin() + pos, _tokens.end(), token);
            if (it != _tokens.end()) {
                return it - _tokens.begin();
            }
            return std::string::npos;
        }
        /// Returns the index of the first occurrence of the token
        /// starting at position pos.
        /// Throws a NotFoundException if the token is not found.

        ALWAYS_INLINE std::size_t
        replace(const std::string &oldToken, const std::string &newToken, std::string::size_type pos = 0) {
            std::size_t result = 0;
            TokenVec::iterator it = std::find(_tokens.begin() + pos, _tokens.end(), oldToken);
            while (it != _tokens.end()) {
                result++;
                *it = newToken;
                it = std::find(++it, _tokens.end(), oldToken);
            }
            return result;
        }
        /// Starting at position pos, replaces all subsequent tokens having value
        /// equal to oldToken with newToken.
        /// Returns the number of modified tokens.

        std::size_t count() const;
        /// Returns the total number of tokens.

        std::size_t count(const std::string &token) const;
        /// Returns the number of tokens equal to the specified token.

    private:
        void trim(std::string &token);

        TokenVec _tokens;
    };

    inline StringTokenizer::Iterator StringTokenizer::begin() const {
        return _tokens.begin();
    }


    inline StringTokenizer::Iterator StringTokenizer::end() const {
        return _tokens.end();
    }


    inline std::string &StringTokenizer::operator[](std::size_t index) {
        if (index >= _tokens.size()) throw std::overflow_error("index over flow");
        return _tokens[index];
    }


    inline const std::string &StringTokenizer::operator[](std::size_t index) const {
        if (index >= _tokens.size()) throw std::overflow_error("index over flow");
        return _tokens[index];
    }


    inline std::size_t StringTokenizer::count() const {
        return _tokens.size();
    }
}


#endif //MANXI_CORE_STRING_TOKENIZER_H
