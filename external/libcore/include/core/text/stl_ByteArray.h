/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/10 13:57
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STL_BYTEARRAY_H
#define MANXI_CORE_STL_BYTEARRAY_H

#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include "stl_String.h"
#include "../stl_macro.h"

typedef unsigned char byte;
namespace stl {
    class String;

    enum struct Endian {
        System,
        Little,
        Big
    };

    template<typename T>
    std::vector<T> &operator+(std::vector<T> &v1, const std::vector<T> &v2) {
        v1.insert(v1.end(), v2.begin(), v2.end());
        return v1;
    }

    template<typename T>
    std::vector<T> &operator+(std::vector<T> &v1, const std::string &v2) {
        v1.insert(v1.end(), v2.begin(), v2.end());
        return v1;
    }

    class ByteArray final : public std::vector<byte> {
    public:
        ByteArray() = default;

        ByteArray(const char *data, size_t len) {
            this->append(data, len);
        }

        ByteArray(const byte *data, size_t len) {
            this->append(data, len);
        }

        ByteArray(const void *data, size_t len) {
            this->append((const byte *) data, len);
        }

        ByteArray(const ByteArray &data) {
            this->append(data);
        }

        explicit ByteArray(const std::string &data) {
            this->append(data);
        }

        explicit ByteArray(const String &data);

        template<size_t N>
        explicit ByteArray(const byte (&data)[N]) {
            this->append(data, N);
        }

        template<size_t N>
        explicit ByteArray(const char (&data)[N]) {
            this->append(data, N);
        }

        ByteArray &operator+(const ByteArray &v) {
            this->insert(this->end(), v.begin(), v.end());
            return *this;
        }

        ByteArray &operator+(const std::string &v) {
            this->insert(this->end(), v.begin(), v.end());
            return *this;
        }

        friend std::ostream &operator<<(std::ostream &os, const ByteArray &ba) {
            return os << ba.data();
        }

        size_t indexOf(byte b, size_t from = 0) const;

        ByteArray &append(const ByteArray &v);

        ByteArray &append(const std::string &v) {
            this->insert(this->end(), v.begin(), v.end());
            return *this;
        }

        ByteArray &append(const byte v);

        ByteArray &append(const byte *v, size_t len);

        ByteArray &append(const char *v, size_t len);

        ByteArray &appendHex(const byte *v, size_t len);

        ByteArray &appendHex(const char *v, size_t len);

        ALWAYS_INLINE ByteArray &appendHex(const std::string &v) {
            return appendHex(v.data(), v.size());
        };

        ByteArray &assign(const ByteArray &v);

        ALWAYS_INLINE ByteArray &assign(const std::string &v) {
            this->clear();
            return append(v);
        };

        ByteArray &assign(const byte *v, size_t len);

        ByteArray &assign(const char *v, size_t len);

        ALWAYS_INLINE ByteArray &assignHex(const std::string &v) {
            this->clear();
            return appendHex(v);
        };

        ByteArray &writeBytes(const byte *v, size_t len, size_t offset);

        ByteArray &writeBytes(const char *v, size_t len, size_t offset);

        ByteArray &writeBytes(const byte *v, size_t len);

        ByteArray &writeBytes(const char *v, size_t len);

        ByteArray &writeString(const std::string &v);

        ByteArray readBytes(size_t offset, size_t len);

        byte readByte(size_t offset = 0);

        void writeByte(byte value);

        bool readBool(size_t offset = 0);

        void writeBool(bool value) {
            this->append((byte *) &value, sizeof(bool));
        }

        uint8_t readUint8(size_t offset = 0) {
            uint8_t result = *(uint8_t *) (this->data() + offset);
            return result;
        }

        void writeUint8(uint8_t value) {
            this->append((byte *) &value, sizeof(uint8_t));
        }

        uint16_t readUint16(size_t offset = 0);

        void writeUint16(uint16_t value);

        uint32_t readUint32(size_t offset = 0);

        void writeUint32(uint32_t value);

        uint64_t readUint64(size_t offset = 0);

        void writeUint64(uint64_t value);

        int8_t readInt8(size_t offset = 0) {
            int8_t result = *(int8_t *) (this->data() + offset);
            return result;
        }

        void writeInt8(int8_t value) {
            this->append((byte *) &value, sizeof(int8_t));
        }

        int16_t readInt16(size_t offset = 0) {
            return (int16_t) readUint16(offset);
        }

        void writeInt16(int16_t value) {
            writeUint16(*(uint16_t *) &value);
        }

        int32_t readInt32(size_t offset = 0) {
            return (int32_t) readUint32(offset);
        }

        void writeInt32(int32_t value) {
            writeUint32(*(uint32_t *) &value);
        }

        int64_t readInt64(size_t offset = 0) {
            return (int64_t) readUint64(offset);
        }

        void writeInt64(int64_t value) {
            writeUint64(*(uint64_t *) &value);
        }

        float readFloat(size_t offset = 0) {
            uint32_t value = readUint32(offset);
            return *(float *) &value;
        }

        void writeFloat(float value) {
            writeUint32(*(uint32_t *) &value);
        }

        double readDouble(size_t offset = 0) {
            uint64_t value = readUint64(offset);
            return *(double *) &value;
        }

        void writeDouble(double value) {
            writeUint64(*(uint64_t *) &value);
        }

        ByteArray &fromHex(const std::string &v) {
            return assignHex(v);
        }

        String toString() const;

        String toBase64() const;

        ALWAYS_INLINE std::string toHex(bool pretty = true) {
            static const char lookup[] = "0123456789ABCDEF";
            std::string result;
            size_t i = 0;
            size_t len = this->size();
            const byte *bytes = this->data();
            while (i < len) {
                char ch = bytes[i];
                result.push_back(lookup[(ch & 0xF0) >> 4]);
                result.push_back(lookup[(ch & 0x0F)]);
                if (pretty)
                    result.push_back(' ');
                i = i + 1;
            }
            if (pretty)
                result.pop_back();
            return result;
        };

        Endian getSystemEndian() {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            return Endian::Little;
#else
            return Endian::Big;
#endif
        }

        void setEndian(Endian value) {
            this->endian = value;
        }

        Endian endian = Endian::System;

    private:
        static char hex_to_unit(byte ch) {
            if (ch >= 0x30 && ch <= 0x39) { // 0...9
                return char(ch - 0x30);
            } else if (ch >= 0x41 && ch <= 0x46) { // ABCDEF
                return char(ch - 0x41 + 10);
            } else if (ch >= 0x61 && ch <= 0x66) { // abcdef
                return char(ch - 0x61 + 10);
            }
            return 0;
        }

        static byte hex_to_byte(const byte *src) {
            return byte(hex_to_unit(src[0]) * 16 + hex_to_unit(src[1]));
        }

    };

}


#endif //MANXI_CORE_STL_BYTEARRAY_H
