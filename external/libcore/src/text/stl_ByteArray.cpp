/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/10 13:52
 * @version: 1.0
 * @description: 
*/
#include "text/stl_ByteArray.h"
#include "stl_core.h"
#include "text/stl_Base64.h"

namespace stl {


    uint16_t ByteArray::readUint16(size_t offset) {
        uint16_t result = *(uint16_t *) (this->data() + offset);
        if (this->endian != Endian::System && this->endian != this->getSystemEndian()) {
            result = ByteOrder::swap(result);
        }
        return result;
    }

    void ByteArray::writeUint16(uint16_t value) {
        if (this->endian != Endian::System && this->endian != this->getSystemEndian()) {
            value = ByteOrder::swap(value);
        }
        this->append((byte *) &value, sizeof(uint16_t));
    }



    ByteArray &ByteArray::appendHex(const byte *v, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            if (v[i] == ' ')
                continue;
            if (i + 1 >= len)
                break;
            if (v[i + 1] == ' ')
                continue;
            this->append(hex_to_byte(v + i));
            i = i + 1;
        }
        return *this;
    }

    uint32_t ByteArray::readUint32(size_t offset) {
        uint32_t result = *(uint32_t *) (this->data() + offset);
        if (this->endian != Endian::System && this->endian != this->getSystemEndian()) {
            result = ByteOrder::swap(result);
        }
        return result;
    }

    void ByteArray::writeUint32(uint32_t value) {
        if (this->endian != Endian::System && this->endian != this->getSystemEndian()) {
            value = ByteOrder::swap(value);
        }
        this->append((byte *) &value, sizeof(uint32_t));
    }

    uint64_t byteswap_uint64(uint64_t value) {
        return ((value & 0x00000000000000FF) << 56) |
               ((value & 0x000000000000FF00) << 40) |
               ((value & 0x0000000000FF0000) << 24) |
               ((value & 0x00000000FF000000) << 8) |
               ((value & 0x000000FF00000000) >> 8) |
               ((value & 0x0000FF0000000000) >> 24) |
               ((value & 0x00FF000000000000) >> 40) |
               ((value & 0xFF00000000000000) >> 56);
    }

    uint64_t ByteArray::readUint64(size_t offset) {
        uint64_t result = *(uint64_t *) (this->data() + offset);
        if (this->endian != Endian::System && this->endian != this->getSystemEndian()) {
            result = byteswap_uint64(result);
        }
        return result;
    }

    void ByteArray::writeUint64(uint64_t value) {
        if (this->endian != Endian::System && this->endian != this->getSystemEndian()) {
            value = byteswap_uint64(value);
        }
        this->append((byte *) &value, sizeof(uint64_t));
    }

    ByteArray &ByteArray::append(const ByteArray &v) {
        this->insert(this->end(), v.begin(), v.end());
        return *this;
    }


    ByteArray &ByteArray::append(const byte v) {
        this->push_back(v);
        return *this;
    }

    ByteArray &ByteArray::append(const byte *v, size_t len) {
        this->resize(this->size() + len);
        memcpy(this->data() + this->size() - len, v, len);
        return *this;
    }

    ByteArray &ByteArray::append(const char *v, size_t len) {
        this->resize(this->size() + len);
        memcpy(this->data() + this->size() - len, v, len);
        return *this;
    }

    ByteArray &ByteArray::appendHex(const char *v, size_t len) {
        return appendHex((const byte *) v, len);
    }

    ByteArray &ByteArray::assign(const ByteArray &v) {
        this->clear();
        return append(v);
    }

    ByteArray &ByteArray::assign(const byte *v, size_t len) {
        this->clear();
        return append(v, len);
    }



    ByteArray &ByteArray::assign(const char *v, size_t len) {
        this->clear();
        return append(v, len);
    }



    ByteArray &ByteArray::writeBytes(const byte *v, size_t len, size_t offset) {
        this->resize(this->size() + len);
        memcpy(this->data() + offset + len, this->data() + offset, this->size() - offset - len);
        memcpy(this->data() + offset, v, len);
        return *this;
    }

    ByteArray &ByteArray::writeBytes(const char *v, size_t len, size_t offset) {
        return writeBytes((const byte *) v, len, offset);
    }

    ByteArray &ByteArray::writeBytes(const byte *v, size_t len) {
        return append(v, len);
    }

    ByteArray &ByteArray::writeBytes(const char *v, size_t len) {
        return append(v, len);
    }

    ByteArray &ByteArray::writeString(const std::string &v) {
        return append(v);
    }

    ByteArray ByteArray::readBytes(size_t offset, size_t len) {
        return {this->data() + offset, len};
    }

    byte ByteArray::readByte(size_t offset) {
        byte result = *(byte *) (this->data() + offset);
        return result;
    }

    void ByteArray::writeByte(byte value) {
        this->append((byte *) &value, sizeof(byte));
    }

    bool ByteArray::readBool(size_t offset) {
        bool result = *(bool *) (this->data() + offset);
        return result;
    }

    ByteArray::ByteArray(const String &data) {
        this->append(data.toStdString());
    }

    String ByteArray::toString() const {
        return {(char *) this->data(), this->size()};
    }

    String ByteArray::toBase64() const {
        return Base64::toBase64(this->data(), this->size());
    }
}