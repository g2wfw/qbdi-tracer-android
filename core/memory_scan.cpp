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
#include <core/logging/check.h>
#include "memory_scan.h"
#include "core/common.h"


namespace stl {


    std::vector<std::string>
    MemoryScan::string_split(std::string string, std::string delimiter, int max_tokens) {
        std::string::size_type pos = string.find(delimiter);

        std::string remainder = string;

        std::vector<std::string> str_list;

        if (pos != std::string::npos) {
            size_t delimiter_len = delimiter.length();
            while (--max_tokens && pos != std::string::npos) {
                std::string pfix = remainder.substr(0, pos);
                str_list.push_back(pfix);
                remainder = remainder.substr(pos + delimiter_len);
                pos = remainder.find(delimiter);
            }
            if (remainder.length() != 0) {
                str_list.push_back(remainder);
            }
        } else {
            str_list.push_back(string);
        }
        return str_list;
    }


    void MemoryScan::match_pattern_new_from_string(std::string str) {
        const char *ch, *mh;
        MatchToken *token = nullptr;
        auto parts = string_split(str, ":", 2);
        std::string match_str_ = parts.at(0);


        const char *match_str = match_str_.c_str();
        std::string mask_str_;
        if (parts.size() > 1) {
            mask_str_ = parts.at(1);
        }
        bool has_mask = mask_str_.length() != 0;
        const char *mask_str = has_mask ? mask_str_.c_str() : nullptr;
        for (ch = match_str, mh = mask_str;
             *ch != '\0' && (!has_mask || *mh != '\0');
             ch++, mh++) {
            int upper, lower;
            int mask = 0xff;
            uint8_t value;

            if (ch[0] == ' ') {
                continue;
            }
            if (has_mask) {
                while (mh[0] == ' ')
                    mh++;
                if ((upper = ascii_xdigit_value(mh[0])) == -1)
                    goto parse_error;
                if ((lower = ascii_xdigit_value(mh[1])) == -1)
                    goto parse_error;
                mask = (upper << 4) | lower;
            }

            if (ch[0] == '?') {
                upper = 4;
                mask &= 0x0f;
            } else if ((upper = ascii_xdigit_value(ch[0])) == -1) {
                goto parse_error;
            }

            if (ch[1] == '?') {
                lower = 2;
                mask &= 0xf0;
            } else if ((lower = ascii_xdigit_value(ch[1])) == -1) {
                goto parse_error;
            }

            value = (upper << 4) | lower;

            if (mask == 0xff) {
                if (token == nullptr || token->type != MATCH_EXACT) {
                    token = match_pattern_push_token(pattern_, MATCH_EXACT);
                }
                token->bytes.push_back(value);
            } else if (mask == 0x00) {
                if (token == nullptr || token->type != MATCH_WILDCARD) {
                    token = match_pattern_push_token(pattern_, MATCH_WILDCARD);
                }
                token->bytes.push_back(0x42);
            } else {
                if (token == nullptr || token->type != MATCH_MASK) {
                    token = match_pattern_push_token(pattern_, MATCH_MASK);
                }
                token->bytes.push_back(value);
                token->masks.push_back(mask);
            }

            ch++;
            mh++;

        }
        match_token_cmp(pattern_);
        return;
        parse_error:
        {
            LOGE("parse_error");
        };

    }


    static const uint16_t ascii_table_data[256] = {
            0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
            0x004, 0x104, 0x104, 0x004, 0x104, 0x104, 0x004, 0x004,
            0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
            0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
            0x140, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
            0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
            0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459,
            0x459, 0x459, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
            0x0d0, 0x653, 0x653, 0x653, 0x653, 0x653, 0x653, 0x253,
            0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
            0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
            0x253, 0x253, 0x253, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
            0x0d0, 0x473, 0x473, 0x473, 0x473, 0x473, 0x473, 0x073,
            0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
            0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
            0x073, 0x073, 0x073, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x004
            /* the upper 128 are all zeroes */
    };

    const uint16_t *const g_ascii_table = ascii_table_data;

/* Functions like the ones in <ctype.h> that are not affected by locale. */
    typedef enum {
        G_ASCII_ALNUM = 1 << 0,
        G_ASCII_ALPHA = 1 << 1,
        G_ASCII_CNTRL = 1 << 2,
        G_ASCII_DIGIT = 1 << 3,
        G_ASCII_GRAPH = 1 << 4,
        G_ASCII_LOWER = 1 << 5,
        G_ASCII_PRINT = 1 << 6,
        G_ASCII_PUNCT = 1 << 7,
        G_ASCII_SPACE = 1 << 8,
        G_ASCII_UPPER = 1 << 9,
        G_ASCII_XDIGIT = 1 << 10
    } GAsciiType;

#define g_ascii_isdigit(c) \
  ((g_ascii_table[(u_char) (c)] & G_ASCII_DIGIT) != 0)

    int g_ascii_digit_value(char c) {
        if (g_ascii_isdigit (c))
            return c - '0';
        return -1;
    }


    int MemoryScan::ascii_xdigit_value(char c) {
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        return g_ascii_digit_value(c);
    }

    MemoryScan::MatchToken *MemoryScan::match_pattern_push_token(MatchPattern *self, MatchType type) {
        MatchToken *token = new MatchToken;
        token->type = type;
        token->offset = self->tokens.size();
        self->tokens.push_back(token);
        self->size = self->tokens.size();
        return token;
    }

    MemoryScan::MatchPattern *MemoryScan::copy_pattern(MatchPattern *pattern) {
        return nullptr;
    }

    bool
    MemoryScan::memory_scan(void *base_, size_t len, std::function<bool(uint8_t *address)> func,
                            MatchPattern *pattern_) {

        uint8_t *cur = static_cast<uint8_t *>(base_);
        uint8_t *end_address = cur + len;
        MatchToken *needle = match_pattern_get_longest_token(pattern_, MATCH_EXACT);


        uint8_t *needle_data = needle->bytes.data();
        size_t needle_len = needle->bytes.size();

        while (cur < end_address) {
            //first match
            cur = static_cast<uint8_t *>(memmem(cur, end_address - cur, needle_data, needle_len));
            if (cur == nullptr) {
                //no data matche to
                return false;
            }
            uint8_t *temp_before = cur;
            if (needle->offset > 0) {
                if (!match_before(needle->offset - 1, temp_before, pattern_)) {
                    cur += needle_len;
                    continue;
                }
            }
            uint8_t *temp_after = cur + needle_len;
            if (needle->offset < pattern_->tokens.size()) {
                if (!match_after(needle->offset + 1, temp_after, pattern_)) {
                    cur += needle_len;
                    continue;
                }
            }
            if (!func(temp_before)) {
                return true;
            }
            cur += needle_len;
        }
        return true;
    }

    bool MemoryScan::match_after(size_t index, uint8_t *&data, MatchPattern *pattern_) {
        bool pass_after = true;
        for (int i = index; i < pattern_->size; ++i) {
            MatchToken *needle_ = pattern_->tokens.at(i);
            if (needle_->type == MATCH_WILDCARD) {
                data = data + needle_->bytes.size();
            } else if (needle_->type == MATCH_MASK) {
                pass_after = memcmp_mask(needle_, data);
                data = data + needle_->bytes.size();
            } else if (needle_->type == MATCH_EXACT) {
                pass_after = memcmp(data, needle_->bytes.data(), needle_->bytes.size()) == 0;
                data = data + needle_->bytes.size();
            } else {
                printf("error\n");
            }
        }
        return pass_after;
    }


    void *MemoryScan::memory_scan_asyn(void *arg) {

        Args *args = static_cast<Args *>(arg);

        void *base_ = args->base_;
        size_t len = args->len;
        std::function<bool(uint8_t *address)> func = args->func;
        MatchPattern *pattern_ = args->pattern_;


        uint8_t *cur = static_cast<uint8_t *>(base_);
        uint8_t *end_address = cur + len;
        MatchToken *needle = MemoryScan::match_pattern_get_longest_token(pattern_, MATCH_EXACT);


        uint8_t *needle_data = needle->bytes.data();
        size_t needle_len = needle->bytes.size();

        while (cur < end_address) {
            //first match
            cur = static_cast<uint8_t *>(memmem(cur, end_address - cur, needle_data, needle_len));
            if (cur == nullptr) {
                //no data matche to
                return nullptr;
            }
            uint8_t *temp_before = cur;
            if (needle->offset > 0) {
                if (!match_before(needle->offset - 1, temp_before, pattern_)) {
                    cur += needle_len;
                    continue;
                }
            }
            uint8_t *temp_after = cur + needle_len;
            if (needle->offset < pattern_->tokens.size()) {
                if (!match_after(needle->offset + 1, temp_after, pattern_)) {
                    cur += needle_len;
                    continue;
                }
            }
            if (!func(temp_before)) {
                return nullptr;
            }
            cur += needle_len;
        }

        for (auto &item: args->pattern_->tokens) {
            delete (item);
        }
        delete (args->pattern_);
        delete (args);
        return nullptr;
    }

    void
    MemoryScan::memoryScanAsyn(void *base, size_t len, std::function<bool(uint8_t *address)> func) {

        //异步线程要对数据进行复制
        Args *args = new Args;
        args->base_ = base;
        args->len = len;
        args->func = func;

        MatchPattern *matchPattern = new MatchPattern;
        matchPattern->byte_len = this->pattern_->byte_len;
        matchPattern->size = this->pattern_->size;

        for (const auto &item: this->pattern_->tokens) {
            MatchToken *matchToken = new MatchToken;
            matchToken->offset = item->offset;
            matchToken->type = item->type;
            for (const auto &byte: item->bytes) {
                matchToken->bytes.push_back(byte);
            }
            for (const auto &mask: item->masks) {
                matchToken->masks.push_back(mask);
            }
            matchPattern->tokens.push_back(matchToken);
        }
        args->pattern_ = matchPattern;
        pthread_t scan_thread;
        pthread_create(&scan_thread, nullptr, memory_scan_asyn, (void *) args);
    }

    bool MemoryScan::match_before(size_t index, uint8_t *&data, MatchPattern *pattern_) {

        bool pass_before = true;
        for (int i = index; i >= 0; --i) {
            MatchToken *needle_ = pattern_->tokens.at(i);
            if (needle_->type == MATCH_WILDCARD) {
                data = data - needle_->bytes.size();
            } else if (needle_->type == MATCH_MASK) {
                data = data - needle_->bytes.size();
                pass_before = memcmp_mask(needle_, data);
            } else if (needle_->type == MATCH_EXACT) {
                data = data - needle_->bytes.size();
                pass_before = memcmp(data, needle_->bytes.data(), needle_->bytes.size()) == 0;
            } else {
                printf("error\n");
            }
        }
        return pass_before;
    }


    void
    MemoryScan::memoryScanSync(void *base_, size_t len, std::function<bool(uint8_t *address)> func) {
        memory_scan(base_, len, func, this->pattern_);
    }


    void MemoryScan::memoryScanSync(void *base, void *end, std::function<bool(uint8_t *address)> func) {
        uint8_t *data = static_cast<uint8_t *>(base);
        uint8_t *end_data = static_cast<uint8_t *>(end);

        memory_scan(data, end_data - data, func, this->pattern_);
    }

    bool MemoryScan::memcmp_mask(MatchToken *matchToken, uint8_t *mask_data) {

        auto len = matchToken->bytes.size();

        for (auto i = 0; i < len; ++i) {
            uint8_t value = mask_data[i];
            uint8_t mask = matchToken->masks.at(i);
            uint8_t value_mask_calc = value & mask;
            uint8_t value_mask_data = matchToken->bytes.at(i) & mask;
            if (value_mask_data != value_mask_calc) {
                return false;
            }
        }
        return true;

    }


    MemoryScan::MatchToken *
    MemoryScan::match_pattern_get_longest_token(const MatchPattern *self, MatchType type) {
        MatchToken *longest = nullptr;
        for (const auto &item: self->tokens) {
            if (item->type == type &&
                (longest == nullptr || item->bytes.size() > longest->bytes.size())) {
                longest = item;
            }
        }
        return longest;
    }


    MemoryScan::MatchPattern *MemoryScan::match_token_cmp(MatchPattern *pattern) {
        for (const auto &item: pattern->tokens) {
            pattern->byte_len += item->bytes.size();
        }
        return pattern;
    }

    MemoryScan::MemoryScan(std::string pattern_str) : pattern_str_(pattern_str) {
        pattern_ = new MatchPattern;
        pattern_->size = 0;
        pattern_->byte_len = 0;
        match_pattern_new_from_string(pattern_str);
    }

    MemoryScan::~MemoryScan() {
        for (auto &item: pattern_->tokens) {
            delete (item);
        }
    }

    void MemoryScan::memoryScanAsyn(void *base, void *end, std::function<bool(uint8_t *address)> func) {
        uint8_t *data = static_cast<uint8_t *>(base);
        uint8_t *end_data = static_cast<uint8_t *>(end);
        memoryScanAsyn(data, end_data - data, func);
    }

    uint8_t *MemoryScan::memoryScanOnce(void *base_, void *end) {

        uint8_t *cur = static_cast<uint8_t *>(base_);
        uint8_t *end_address = static_cast<uint8_t *>(end);
        MatchToken *needle = match_pattern_get_longest_token(pattern_, MATCH_EXACT);
        uint8_t *needle_data = needle->bytes.data();
        size_t needle_len = needle->bytes.size();

        while (cur < end_address) {
            //first match
            cur = static_cast<uint8_t *>(memmem(cur, end_address - cur, needle_data, needle_len));
            if (cur == nullptr) {
                //no data matche to
                return nullptr;
            }
            uint8_t *temp_before = cur;
            if (needle->offset > 0) {
                if (!match_before(needle->offset - 1, temp_before, pattern_)) {
                    cur += needle_len;
                    continue;
                }
            }
            uint8_t *temp_after = cur + needle_len;
            if (needle->offset < pattern_->tokens.size()) {
                if (!match_after(needle->offset + 1, temp_after, pattern_)) {
                    cur += needle_len;
                    continue;
                }
            }
            return temp_before;

        }
        return nullptr;
    }


    MemoryScan::MatchToken::~MatchToken() {
        //std::cout << "delete MatchToken" << std::endl;
    }

}