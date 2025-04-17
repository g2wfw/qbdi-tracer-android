/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/10 16:58
 * @version: 1.0
 * @description: 
*/

#include "text/stl_StringTokenizer.h"
#include "text/stl_Ascii.h"
#include "stl_core.h"
#include <string>
#include <algorithm>

namespace stl {



    StringTokenizer::~StringTokenizer() {
    }


    void StringTokenizer::trim(std::string &token) {
        std::string::size_type front = 0;
        std::string::size_type back = 0;
        std::string::size_type length = token.length();
        std::string::const_iterator tIt = token.begin();
        std::string::const_iterator tEnd = token.end();
        for (; tIt != tEnd; ++tIt, ++front) {
            if (!Ascii::isSpace(*tIt)) break;
        }
        if (tIt != tEnd) {
            std::string::const_reverse_iterator tRit = token.rbegin();
            std::string::const_reverse_iterator tRend = token.rend();
            for (; tRit != tRend; ++tRit, ++back) {
                if (!Ascii::isSpace(*tRit)) break;
            }
        }
        token = token.substr(front, length - back - front);
    }


    std::size_t StringTokenizer::count(const std::string &token) const {
        std::size_t result = 0;
        auto it = std::find(_tokens.begin(), _tokens.end(), token);
        while (it != _tokens.end()) {
            result++;
            it = std::find(++it, _tokens.end(), token);
        }
        return result;
    }





    StringTokenizer::StringTokenizer(const String &str_, const String &separators_, int options) {
        auto str = str_.toStdString();
        auto separators = separators_.toStdString();
        std::string::const_iterator it = str.begin();
        std::string::const_iterator end = str.end();
        std::string token;
        bool doTrim = ((options & TOK_TRIM) != 0);
        bool ignoreEmpty = ((options & TOK_IGNORE_EMPTY) != 0);
        bool lastToken = false;
        for (; it != end; ++it) {
            if (separators.find(*it) != std::string::npos) {
                if (doTrim) trim(token);
                if (!token.empty() || !ignoreEmpty) _tokens.push_back(token);
                if (!ignoreEmpty) lastToken = true;
                token.clear();
            } else {
                token += *it;
                lastToken = false;
            }
        }
        if (!token.empty()) {
            if (doTrim) trim(token);
            if (!token.empty() || !ignoreEmpty) _tokens.push_back(token);
        } else if (lastToken) {
            _tokens.push_back(std::string());
        }
    }
}
