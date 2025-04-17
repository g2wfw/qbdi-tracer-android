/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/14 18:30
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STL_COLLECTIONS_H
#define MANXI_CORE_STL_COLLECTIONS_H

#include "../text/stl_String.h"
#include "../text/stl_StringRef.h"
#include "stl_Array.h"
#include "stl_HashMap.h"
#include "stl_SortedSet.h"
#include <set>
#include <list>
#include <map>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>

namespace stl {
    class STL_API Collections final {
    public:
        //for std
        template<typename Container>
        static std::string join(const Container &v, const char *delim) {
            if (v.empty()) {
                return "[]";
            }
            std::ostringstream os;
            os << "[";
            std::copy(v.begin(), std::prev(v.end()),
                      std::ostream_iterator<typename Container::value_type>(os, delim));
            os << *(v.rbegin()); // 添加最后一个元素，后面没有分隔符
            os << "]";
            return os.str();
        }


        template<typename K, typename V>
        std::string join(const std::map<K, V> &m, const char *delim) {
            if (m.empty()) {
                return "[]";
            }
            std::ostringstream os;
            os << "[";
            for (auto iter = m.begin(); iter != m.end(); ++iter) {
                if (iter != m.begin())
                    os << delim;
                os << "{" << iter->first << ":" << iter->second << "}";
            }
            os << "]";
            return os.str();
        }

        template<typename K, typename V>
        std::string join(const HashMap<K, V> &m, const char *delim) {
            if (m.isEmpty()) {
                return "[]";
            }
            std::ostringstream os;
            os << "[";
            for (auto iter = m.begin(); iter != m.end(); ++iter) {
                if (iter != m.begin())
                    os << delim;
                os << "{" << iter->first << ":" << iter->second << "}";
            }
            os << "]";
            return os.str();
        }

        //for stl array
        template<typename T>
        static std::string join(const Array<T> &v, const char *delim) {
            if (v.isEmpty()) {
                return "[]";
            }
            std::ostringstream os;
            os << "[";
            std::copy(v.begin(), v.end() - 1, std::ostream_iterator<T>(os, delim));
            os << v.getLast(); // 添加最后一个元素，后面没有分隔符
            os << "]";
            return os.str();
        }

        template<typename T>
        static std::string joinWithBorder(const Array<T> &v, const char *delim, const char *left = "", const char *right = "") {
            if (v.isEmpty()) {
                return "[]";
            }
            std::ostringstream os;
            os << left;
            std::copy(v.begin(), v.end() - 1, std::ostream_iterator<T>(os, delim));
            os << v.getLast(); // 添加最后一个元素，后面没有分隔符
            os << right;
            return os.str();
        }
    };
}

#endif //MANXI_CORE_STL_COLLECTIONS_H
