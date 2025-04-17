//
// Created by xb on 2024/1/22.
//

#include <libgen.h>
#include "system/stl_Common.h"
#include "logging/check.h"
#include <mutex>
#include <cstdarg>
#include <cstring>
#include <algorithm>
#include <files/stl_File.h>


namespace stl {
    std::vector<uint8_t> Common::HexStringToBytes(std::string &str) {
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


    std::string Common::DirectoryName(const std::string &path) {
        // Copy path because dirname may modify the string passed in.
        std::string result(path);

#if !defined(__BIONIC__)
        // Use lock because dirname() may write to a process global and return a
        // pointer to that. Note that this locking strategy only works if all other
        // callers to dirname in the process also grab this same lock, but its
        // better than nothing.  Bionic's dirname returns a thread-local buffer.
        static std::mutex &dirname_lock = *new std::mutex();
        std::lock_guard<std::mutex> lock(dirname_lock);
#endif

        // Note that if std::string uses copy-on-write strings, &str[0] will cause
        // the copy to be made, so there is no chance of us accidentally writing to
        // the storage for 'path'.
        char *parent = dirname(&result[0]);
        // In case dirname returned a pointer to a process global, copy that string
        // before leaving the lock.
        result.assign(parent);
        return result;
    }

    std::string Common::FileName(const std::string &path) {
        // Copy path because basename may modify the string passed in.
        std::string result(path);

#if !defined(__BIONIC__)
        // Use lock because basename() may write to a process global and return a
        // pointer to that. Note that this locking strategy only works if all other
        // callers to basename in the process also grab this same lock, but its
        // better than nothing.  Bionic's basename returns a thread-local buffer.
        static std::mutex &basename_lock = *new std::mutex();
        std::lock_guard<std::mutex> lock(basename_lock);
#endif

        // Note that if std::string uses copy-on-write strings, &str[0] will cause
        // the copy to be made, so there is no chance of us accidentally writing to
        // the storage for 'path'.
        char *name = basename(&result[0]);

        // In case basename returned a pointer to a process global, copy that string
        // before leaving the lock.
        result.assign(name);

        return result;
    }

    std::string Common::StringReplace(std::string_view s, std::string_view from, std::string_view to, bool all) {
        if (from.empty())
            return std::string(s);
        std::string result;
        std::string_view::size_type start_pos = 0;
        do {
            std::string_view::size_type pos = s.find(from, start_pos);
            if (pos == std::string_view::npos)
                break;
            result.append(s.data() + start_pos, pos - start_pos);
            result.append(to.data(), to.size());
            start_pos = pos + from.size();
        } while (all);
        result.append(s.data() + start_pos, s.size() - start_pos);
        return result;
    }

    std::string Common::StringReplaceFirst(std::string_view s, std::string_view from, std::string_view to) {
        if (from.empty())
            return std::string(s);
        std::string result;
        std::string_view::size_type pos = s.find(from);
        if (pos != std::string_view::npos) {
            result.append(s.data(), pos);
            result.append(to.data(), to.size());
            result.append(s.data() + pos + from.size(), s.size() - pos - from.size());
        } else {
            result = std::string(s);
        }
        return result;
    }

    std::string Common::StringReplaceLast(std::string_view s, std::string_view from, std::string_view to) {
        if (from.empty())
            return std::string(s);
        std::string result;
        std::string_view::size_type pos = s.rfind(from);
        if (pos != std::string_view::npos) {
            result.append(s.data(), pos);
            result.append(to.data(), to.size());
            result.append(s.data() + pos + from.size(), s.size() - pos - from.size());
        } else {
            result = std::string(s);
        }
        return result;
    }

    bool Common::WriteDataToFile(uint8_t *data, size_t len, const std::string &tofile) {
        FILE *file = fopen(tofile.c_str(), "wb");
        if (file == nullptr) {
            return false;
        }
        fwrite(data, 1, len, file);
        fclose(file);
        return true;
    }



    bool Common::copyFile(const stl::String &source, const stl::String &dest) {
        const stl::File sourceFile(source);
        if (sourceFile.existsAsFile()) {
            const stl::File destFile(dest);
            return destFile.copyFileTo(sourceFile);
        }
        return false;
    }

    std::string Common::BytesToHexString(const uint8_t *input, size_t length) {
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

    std::string Common::StringPrintf(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        std::string result;
        StringAppendV(&result, fmt, ap);
        va_end(ap);
        return result;
    }


    void Common::StringAppendF(std::string *dst, const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        StringAppendV(dst, fmt, ap);
        va_end(ap);
    }

    void Common::StringAppendV(std::string *dst, const char *format, va_list ap) {
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

    std::string Common::Trim(const std::string &s) {
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

    std::vector<std::string> Common::Split(const std::string &s, const std::string &delimiters) {
        CHECK_NE(delimiters.size(), 0U);

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

    bool Common::StartsWith(std::string_view s, std::string_view prefix) {
        return s.substr(0, prefix.size()) == prefix;
    }

    bool Common::StartsWith(const std::string &s, const char *suffix) {
        return strncmp(s.c_str(), suffix, strlen(suffix)) == 0;
    }

    bool Common::StartsWithIgnoreCase(const std::string &s, const char *prefix) {
        return strncasecmp(s.c_str(), prefix, strlen(prefix)) == 0;
    }

    bool Common::StartsWith(const std::string &s, const std::string &prefix) {
        return strncmp(s.c_str(), prefix.c_str(), prefix.size()) == 0;
    }

    bool Common::StartsWithIgnoreCase(const std::string &s, const std::string &prefix) {
        return strncasecmp(s.c_str(), prefix.c_str(), prefix.size()) == 0;
    }

    bool Common::EndsWith(const std::string &s, const char *suffix) {
        return EndsWith(s, suffix, strlen(suffix), true);
    }

    bool Common::EndsWith(std::string_view s, std::string_view suffix) {
        return s.size() >= suffix.size() && s.substr(s.size() - suffix.size(), suffix.size()) == suffix;
    }


    bool Common::EndsWithIgnoreCase(const std::string &s, const char *suffix) {
        return EndsWith(s, suffix, strlen(suffix), false);
    }

    bool Common::ContainsIgnoreCase(const std::string &s, const char *suffix) {
        std::string source = s;
        std::string dest = suffix;
        std::transform(source.begin(), source.end(), source.begin(), ::tolower);
        std::transform(dest.begin(), dest.end(), dest.begin(), ::tolower);
        return source.find(suffix) != std::string::npos;
    }

    bool Common::EndsWith(const std::string &s, const std::string &suffix) {
        return EndsWith(s, suffix.c_str(), suffix.size(), true);
    }

    bool Common::EndsWithIgnoreCase(const std::string &s, const std::string &suffix) {
        return EndsWith(s, suffix.c_str(), suffix.size(), false);
    }

    bool Common::EqualsIgnoreCase(const std::string &lhs, const std::string &rhs) {
        return strcasecmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    bool Common::EndsWith(const std::string &s, const char *suffix, size_t suffix_length, bool case_sensitive) {
        size_t string_length = s.size();
        if (suffix_length > string_length) {
            return false;
        }
        size_t offset = string_length - suffix_length;
        return (case_sensitive ? strncmp : strncasecmp)(s.c_str() + offset, suffix, suffix_length) == 0;
    }

    uint32_t Common::murmurhash(const char *key, uint32_t len, uint32_t seed) {
        uint32_t c1 = 0xcc9e2d51;
        uint32_t c2 = 0x1b873593;
        uint32_t r1 = 15;
        uint32_t r2 = 13;
        uint32_t m = 5;
        uint32_t n = 0xe6546b64;
        uint32_t h = 0;
        uint32_t k = 0;
        uint8_t *d = (uint8_t *) key; // 32 bit extract from `key'
        const uint32_t *chunks = NULL;
        const uint8_t *tail = NULL; // tail - last 8 bytes
        int i = 0;
        int l = len / 4; // chunk length

        h = seed;

        chunks = (const uint32_t *) (d + l * 4); // body
        tail = (const uint8_t *) (d + l * 4); // last 8 byte chunk of `key'

        // for each 4 byte chunk of `key'
        for (i = -l; i != 0; ++i) {
            // next 4 byte chunk of `key'
            k = chunks[i];

            // encode next 4 byte chunk of `key'
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;

            // append to hash
            h ^= k;
            h = (h << r2) | (h >> (32 - r2));
            h = h * m + n;
        }

        k = 0;

        // remainder
        switch (len & 3) {
            // `len % 4'
            case 3:
                k ^= (tail[2] << 16);
            case 2:
                k ^= (tail[1] << 8);

            case 1:
                k ^= tail[0];
                k *= c1;
                k = (k << r1) | (k >> (32 - r1));
                k *= c2;
                h ^= k;
        }

        h ^= len;

        h ^= (h >> 16);
        h *= 0x85ebca6b;
        h ^= (h >> 13);
        h *= 0xc2b2ae35;
        h ^= (h >> 16);

        return h;
    }

    size_t Common::HashBytes(const uint8_t *data, size_t len) {
        size_t hash = 0x811c9dc5;
        for (uint32_t i = 0; i < len; ++i) {
            hash = (hash * 16777619) ^ data[i];
        }
        hash += hash << 13;
        hash ^= hash >> 7;
        hash += hash << 3;
        hash ^= hash >> 17;
        hash += hash << 5;
        return hash;
    }

    std::string Common::GetFileExtension(std::string path) {
        auto pos = path.find_last_of(".");
        if (pos == std::string::npos) {
            return "";
        }
        return path.substr(pos);
    }
}
