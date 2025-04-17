/*
The MIT License (MIT)

Copyright (c) 2025 g2wfw

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#ifndef REVERSE_TOOLS_MEMORY_SCAN_H
#define REVERSE_TOOLS_MEMORY_SCAN_H

#include <core/stl_macro.h>
#include <vector>
#include <string>
#include <functional>
namespace stl{
    class MemoryScan {
    public:
        explicit MemoryScan(std::string pattern_str);

        void memoryScanSync(void *base, size_t len, std::function<bool(uint8_t *address)> func);

        void memoryScanSync(void *base, void *end, std::function<bool(uint8_t *address)> func);

        uint8_t *memoryScanOnce(void *base, void *end);

        void memoryScanAsyn(void *base, size_t len, std::function<bool(uint8_t *address)> func);

        void memoryScanAsyn(void *base, void *end, std::function<bool(uint8_t *address)> func);

        ~MemoryScan();

    private:
        enum MatchType {
            MATCH_EXACT,
            MATCH_WILDCARD,
            MATCH_MASK
        };
        typedef struct MatchToken {
            MatchType type;
            std::vector<uint8_t> bytes;
            std::vector<uint8_t> masks;
            uint offset;
        public:
            ~MatchToken();
        } MatchToken;

        typedef struct MatchPattern {
            std::vector<MatchToken *> tokens;
            uint size;
            size_t byte_len;
        } MatchPattern;

        typedef struct Args {
            void *base_;
            size_t len;
            std::function<bool(uint8_t *address)> func;
            MemoryScan::MatchPattern *pattern_;
        } Args;

    private:

        static bool
        memory_scan(void *base, size_t len, std::function<bool(uint8_t *address)> func, MatchPattern *pattern_);

        static void *memory_scan_asyn(void *arg);

        static bool match_before(size_t index, uint8_t *&data, MatchPattern *pattern_);

        static bool match_after(size_t index, uint8_t *&data, MatchPattern *pattern_);

        static MatchToken *match_pattern_get_longest_token(const MatchPattern *self, MatchType type);

        static bool memcmp_mask(MatchToken *matchToken, uint8_t *temp);

        static MatchPattern *copy_pattern(MatchPattern *pattern);

    private:
        std::vector<std::string> string_split(std::string string, std::string delimiter, int max_tokens);

        void match_pattern_new_from_string(std::string str);

        int ascii_xdigit_value(char c);

        MatchPattern *match_token_cmp(MatchPattern *pattern);

        MatchToken *match_pattern_push_token(MatchPattern *self, MatchType type);


    private:
        MatchPattern *pattern_;
        std::string pattern_str_;


    private:
        DISALLOW_COPY_AND_ASSIGN(MemoryScan);
    };

}
#endif //REVERSE_TOOLS_MEMORY_SCAN_H
