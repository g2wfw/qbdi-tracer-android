//
// Created by xb on 2024/1/22.
//

#pragma once

#include "../stl_macro.h"
#include <cstdint>
#include <vector>
#include <string>
#include "../text/stl_String.h"


namespace stl {
    static constexpr char hex_asc[] = "0123456789abcdef";

    class STL_API Common {
    public:
        /**
         * hex格式字符串转 byte数组
         * @param str
         * @return 存储结果
         */

        static bool copyFile(const stl::String &source, const stl::String &dest);


        ALWAYS_INLINE static std::string BytesToHexStringV2(const uint8_t *input, size_t length) {
            std::string str;
            str.reserve(length << 1);
            for (size_t i = 0; i < length; i++) {
                int t = input[i];
                int a = t / 16;
                int b = t % 16;
                str.append(1, hex_asc[a]);
                str.append(1, hex_asc[b]);
            }
            return str;
        }

        static std::string BytesToHexString(const uint8_t *input, size_t length);


        ALWAYS_INLINE static std::vector<uint8_t> HexStringToBytesV2(std::string &str) {
            std::vector<uint8_t> result;
            size_t index = 0; // elements counter
            size_t len = str.length();
            for (size_t i = 0; i < len; i += 2) {
                uint8_t result_;
                if (sscanf(str.c_str() + i, "%2hhx", &result_) != 1) {
                    result.clear();
                    return result;
                };
                result.push_back(result_);
            }
            return result;
        }

        static std::vector<uint8_t> HexStringToBytes(std::string &str);


        // Splits a string into a vector of strings.
//
// The string is split at each occurrence of a character in delimiters.
//
// The empty string is not a valid delimiter list.
        static std::vector<std::string> Split(const std::string &s,
                                              const std::string &delimiters);

        ALWAYS_INLINE  static std::vector<std::string> SplitV2(const std::string &s,
                                                               const std::string &delimiters) {

            std::vector<std::string> result;

            size_t base = 0;
            size_t found;
            while (true) {
                found = s.find_first_of(delimiters, base);
                result.push_back(s.substr(base, found - base));
                if (found == s.npos) break;
                base = found + 1;
            }

            return result;
        }


// Trims whitespace off both ends of the given string.
        static std::string Trim(const std::string &s);


        ALWAYS_INLINE static std::string TrimV2(const std::string &s) {
            std::string result;

            if (s.size() == 0) {
                return result;
            }

            size_t start_index = 0;
            size_t end_index = s.size() - 1;

            // Skip initial whitespace.
            while (start_index < s.size()) {
                if (!isspace(s[start_index])) {
                    break;
                }
                start_index++;
            }

            // Skip terminating whitespace.
            while (end_index >= start_index) {
                if (!isspace(s[end_index])) {
                    break;
                }
                end_index--;
            }

            // All spaces, no beef.
            if (end_index < start_index) {
                return "";
            }
            // Start_index is the first non-space, end_index is the last one.
            return s.substr(start_index, end_index - start_index + 1);
        }

        ALWAYS_INLINE static bool StartsWithV2(std::string_view s, std::string_view prefix) {
            return s.substr(0, prefix.size()) == prefix;
        }


        static bool StartsWith(std::string_view s, std::string_view prefix);


        ALWAYS_INLINE  static bool StartsWithV2(const std::string &s, const char *suffix) {
            return strncmp(s.c_str(), suffix, strlen(suffix)) == 0;
        }

        static bool StartsWith(const std::string &s, const char *prefix);


        ALWAYS_INLINE static bool StartsWithIgnoreCaseV2(const std::string &s, const char *prefix) {
            return strncasecmp(s.c_str(), prefix, strlen(prefix)) == 0;
        }

        static bool StartsWithIgnoreCase(const std::string &s, const char *prefix);

        ALWAYS_INLINE static bool StartsWithV2(const std::string &s, const std::string &prefix) {
            return strncmp(s.c_str(), prefix.c_str(), prefix.size()) == 0;
        }

        static bool StartsWith(const std::string &s, const std::string &prefix);


        ALWAYS_INLINE static bool StartsWithIgnoreCaseV2(const std::string &s, const std::string &prefix) {
            return strncasecmp(s.c_str(), prefix.c_str(), prefix.size()) == 0;
        }

        static bool StartsWithIgnoreCase(const std::string &s, const std::string &prefix);


        ALWAYS_INLINE static bool EndsWithV2(std::string_view s, std::string_view suffix) {
            return s.size() >= suffix.size() && s.substr(s.size() - suffix.size(), suffix.size()) == suffix;
        }

        static bool EndsWith(std::string_view s, std::string_view suffix);

        ALWAYS_INLINE static bool EndsWithV2(const std::string &s, const char *suffix) {
            return EndsWith(s, suffix, strlen(suffix), true);
        }

        static bool EndsWith(const std::string &s, const char *suffix);

        ALWAYS_INLINE  static bool EndsWithIgnoreCaseV2(const std::string &s, const char *suffix) {
            return EndsWith(s, suffix, strlen(suffix), false);
        }

        static bool EndsWithIgnoreCase(const std::string &s, const char *suffix);

        ALWAYS_INLINE static bool ContainsIgnoreCaseV2(const std::string &s, const char *suffix) {
            std::string source = s;
            std::string dest = suffix;
            std::transform(source.begin(), source.end(), source.begin(), ::tolower);
            std::transform(dest.begin(), dest.end(), dest.begin(), ::tolower);
            return source.find(suffix) != std::string::npos;
        }

        static bool ContainsIgnoreCase(const std::string &s, const char *suffix);


        ALWAYS_INLINE  static bool EndsWithV2(const std::string &s, const std::string &suffix) {
            return EndsWith(s, suffix.c_str(), suffix.size(), true);
        }

        static bool EndsWith(const std::string &s, const std::string &suffix);


        ALWAYS_INLINE static bool EndsWithIgnoreCaseV2(const std::string &s, const std::string &suffix) {
            return EndsWith(s, suffix.c_str(), suffix.size(), false);
        }

        static bool EndsWithIgnoreCase(const std::string &s, const std::string &suffix);

// Tests whether 'lhs' equals 'rhs', ignoring case.

        ALWAYS_INLINE bool EqualsIgnoreCaseV2(const std::string &lhs, const std::string &rhs) {
            return strcasecmp(lhs.c_str(), rhs.c_str()) == 0;
        }

        bool EqualsIgnoreCase(const std::string &lhs, const std::string &rhs);


        ALWAYS_INLINE static std::string StringPrintfV2(const char *fmt, ...)
        __attribute__((__format__(__printf__, 1, 2))) {
            va_list ap;
            va_start(ap, fmt);
            std::string result;
            StringAppendVV2(&result, fmt, ap);
            va_end(ap);
            return result;
        }

        static std::string StringPrintf(const char *fmt, ...)
        __attribute__((__format__(__printf__, 1, 2)));


        ALWAYS_INLINE static void StringAppendFV2(std::string *dst, const char *fmt, ...)
        __attribute__((__format__(__printf__, 2, 3))) {
            va_list ap;
            va_start(ap, fmt);
            StringAppendV(dst, fmt, ap);
            va_end(ap);
        }

        static void StringAppendF(std::string *dst, const char *fmt, ...)
        __attribute__((__format__(__printf__, 2, 3)));

        ALWAYS_INLINE  static void StringAppendVV2(std::string *dst, const char *format, va_list ap)
        __attribute__((__format__(__printf__, 2, 0))) {
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

        static void StringAppendV(std::string *dst, const char *format, va_list ap)
        __attribute__((__format__(__printf__, 2, 0)));

        /**
         * 给定路径获取路径所在的文件夹
         * @param path 路径
         * @return
         */
        static std::string DirectoryName(const std::string &path);

        /**
         * 给定路径获取文件名称
         * @param path 路径
         * @return
         */
        static std::string FileName(const std::string &path);


        static bool WriteDataToFile(uint8_t *data, size_t len, const std::string &tofile);

        static std::string StringReplace(std::string_view s,
                                         std::string_view from,
                                         std::string_view to,
                                         bool all = true);

        // Replaces the first occurrence of 'from' with 'to' in the given string 's'.
        static std::string StringReplaceFirst(std::string_view s,
                                              std::string_view from,
                                              std::string_view to);

        // Replaces the last occurrence of 'from' with 'to' in the given string 's'.
        static std::string StringReplaceLast(std::string_view s,
                                             std::string_view from,
                                             std::string_view to);

        static size_t HashBytes(const uint8_t *data, size_t len);

        static uint32_t murmurhash(const char *key, uint32_t len, uint32_t seed);


        template<typename T>
        static inline T alignAdd(T size, uint32_t toalign) {
            if (size % toalign == 0) {
                return size;
            }
            auto mode = size % toalign;
            return size + (toalign - mode);
        };

        static std::string GetFileExtension(std::string path);

    private:
        static bool EndsWith(const std::string &s, const char *suffix, size_t suffix_length,
                             bool case_sensitive);

    private:
        DISALLOW_COPY_AND_ASSIGN(Common);
    };
}


