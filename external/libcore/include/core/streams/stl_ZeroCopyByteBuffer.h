/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/16 14:59
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STL_ZEROCOPYBYTEBUFFER_H
#define MANXI_CORE_STL_ZEROCOPYBYTEBUFFER_H

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <memory>
#include <iostream>
#include <cstdio>
#include "../text/stl_String.h"
#include "../text/stl_StringRef.h"

namespace stl {
    class STL_API ZeroCopyByteBuffer {
    public:
        ZeroCopyByteBuffer(uint8_t *arr, size_t size);

        ~ZeroCopyByteBuffer() = default;

        uint32_t bytesRemaining(); // Number of uint8_ts from the current read position till the end of the buffer
        void clear();

        bool equals(ZeroCopyByteBuffer *other); // Compare if the contents are equivalent

        std::unique_ptr<ZeroCopyByteBuffer> slice(size_t len = 0, size_t postion = -1);

        ZeroCopyByteBuffer getByteBuffer(size_t offset, size_t size);

        uint8_t *getRawBuffer() const;

        std::unique_ptr<ZeroCopyByteBuffer> getByteBuffer(size_t size);

        uint32_t size(); // Size of internal vector

        // Basic Searching (Linear)
        template<typename T>
        int32_t find(T key, uint32_t start = 0) {
            int32_t ret = -1;
            uint32_t len = buff_size;
            for (uint32_t i = start; i < len; i++) {
                T data = read<T>(i);
                // Wasn't actually found, bounds of buffer were exceeded
                if ((key != 0) && (data == 0))
                    break;

                // Key was found in array
                if (data == key) {
                    ret = (int32_t) i;
                    break;
                }
            }
            return ret;
        }

        // Replacement
        void replace(uint8_t key, uint8_t rep, uint32_t start = 0, bool firstOccuranceOnly = false);

        int indexOf(const uint8_t *pattern, size_t patternSize) const noexcept;

        int indexOfText(const char *other) const noexcept;

        int indexOfText(size_t start_pos, const char *other) const noexcept;

        bool containsText(const char *other) const noexcept;

        bool containsText(const StringRef &ref) const noexcept;

        // Read
// Relative peek. Reads and returns the next uint8_t in the buffer from the current position
// but does not increment the read position
        uint8_t peek() const;

        // Relative get method. Reads the uint8_t at the buffers current position then increments the position
        uint8_t get() const;

        uint8_t get(uint32_t index) const; // Absolute get method. Read uint8_t at index
        uint32_t getBytes(uint8_t *buf, uint32_t len) const; // Absolute read into array buf of length len
        uint32_t
        getBytes(std::vector<uint8_t> &buf, uint32_t len) const; // Absolute read into array buf of length len


        char getChar() const; // Relative
        char getChar(uint32_t index) const; // Absolute
        double getDouble() const;

        double getDouble(uint32_t index) const;

        float getFloat() const;

        float getFloat(uint32_t index) const;

        uint32_t getInt() const;

        uint32_t getInt(uint32_t index) const;

        uint64_t getLong() const;

        uint64_t getLong(uint32_t index) const;

        uint16_t getShort() const;

        uint16_t getShort(uint32_t index) const;

        // Write
        template<typename T>
        bool putData(T t) {
            uint32_t s = sizeof(t);
            if (size() < (pos_ + s))
                return false;
            memcpy(buf + pos_, (uint8_t *) &t, s);
            //printf("writing %c to %i\n", (uint8_t)data, pos_);
            pos_ += s;
            return true;
        };

        void put(ZeroCopyByteBuffer *src); // Relative write of the entire contents of another ByteBuffer (src)
        void put(uint8_t b); // Relative write
        void put(uint8_t b, uint32_t index); // Absolute write at index
        bool putBytes(const void *b, uint32_t len); // Relative write
        void putBytes(uint8_t *b, uint32_t len, uint32_t index); // Absolute write starting at index
        void putChar(char value); // Relative
        void putChar(char value, uint32_t index); // Absolute
        void putDouble(double value);

        void putDouble(double value, uint32_t index);

        void putFloat(float value);

        void putFloat(float value, uint32_t index);

        void putInt(uint32_t value);

        void putInt(uint32_t value, uint32_t index);

        void putLong(uint64_t value);

        void putLong(uint64_t value, uint32_t index);

        void putShort(uint16_t value);

        void putShort(uint16_t value, uint32_t index);

        // Buffer Position Accessors & Mutators

        void setReadPos(uint32_t r) {
            pos_ = r;
        }

        uint32_t getReadPos() const {
            return pos_;
        }

        void setPostion(uint32_t w) {
            pos_ = w;
        }

        // Utility Functions

        void setName(std::string n);

        std::string getName();

        void printInfo();

        void printAH();

        void printAscii();

        void printHex();

        void printPosition();

        bool hasRemaining();

        int remaining();

        int position();

        //判断当前buffer是否可用
        bool available();

        uint8_t *getPositionBuffer();

        template<typename T>
        T readData() const {
            if (pos_ + sizeof(T) <= buff_size) {
                auto cur = buf + pos_;
                T *t = (T *) cur;
                pos_ += sizeof(T);
                return *t;
            }
            return {};
        }

    private:
        mutable uint32_t pos_ = 0;
        uint8_t *buf;
        size_t buff_size;

        std::string name;


        template<typename T>
        T read() const {
            T data = read<T>(pos_);
            pos_ += sizeof(T);
            return data;
        }

        template<typename T>
        T read(uint32_t index) const {
            if (index + sizeof(T) <= buff_size) {
                auto cur = buf + index;
                T *t = (T *) cur;
                return *t;
            }
            return 0;
        }

        template<typename T>
        bool append(T data) {
            uint32_t s = sizeof(data);

            if (size() < (pos_ + s))
                return false;
            memcpy(&buf[pos_], (uint8_t *) &data, s);
            //printf("writing %c to %i\n", (uint8_t)data, pos_);

            pos_ += s;
            return true;
        }

        template<typename T>
        void insert(T data, uint32_t index) {
            if ((index + sizeof(data)) > size())
                return;

            memcpy(&buf[index], (uint8_t *) &data, sizeof(data));
            pos_ = index + sizeof(data);
        }
    };
}
#endif //MANXI_CORE_STL_ZEROCOPYBYTEBUFFER_H
