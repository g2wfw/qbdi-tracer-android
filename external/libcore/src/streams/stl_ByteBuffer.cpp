/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/16 15:00
 * @version: 1.0
 * @description: 
*/
#include "streams/stl_ByteBuffer.h"
#include "logging/check.h"
#include "files/stl_File.h"
#include "files/stl_memory_file.h"

namespace stl {

    /**
    *ByteBuffer构造函数
    *在ByteBuffer中消耗长度为len的整个uint8_t数组
     *
     * @param arr uint8_t array of data (should be of length len)
     * @param size Size of space to allocate
     */
    ByteBuffer::ByteBuffer(uint8_t *arr, size_t size) {
        // If the provided array is NULL, allocate a blank buffer of the provided size
        this->data_buffer.resize(size);
        this->data_buffer.assign(arr, arr + size);
        this->buf = data_buffer.data();
        this->buff_size = size;
        name = "";
    }


    uint32_t ByteBuffer::bytesRemaining() const {
        return size() - pos_;
    }


    void ByteBuffer::clear() {
        pos_ = 0;
    }

    bool ByteBuffer::equals(ByteBuffer *other) const {
        // If sizes aren't equal, they can't be equal
        if (size() != other->size())
            return false;

        // Compare byte by byte
        uint32_t len = size();
        for (uint32_t i = 0; i < len; i++) {
            if ((uint8_t) get(i) != (uint8_t) other->get(i))
                return false;
        }

        return true;
    }


    uint32_t ByteBuffer::size() const {
        return buff_size;
    }

    void ByteBuffer::replace(uint8_t key, uint8_t rep, uint32_t start, bool firstOccuranceOnly) {
        uint32_t len = buff_size;
        for (uint32_t i = start; i < len; i++) {
            auto data = read<uint8_t>(i);
            // Wasn't actually found, bounds of buffer were exceeded
            if ((key != 0) && (data == 0))
                break;

            // Key was found in array, perform replacement
            if (data == key) {
                buf[i] = rep;
                if (firstOccuranceOnly)
                    return;
            }
        }
    }


    uint8_t ByteBuffer::peek() const {
        return read<uint8_t>(pos_);
    }

    uint8_t ByteBuffer::get() const {
        return read<uint8_t>();
    }

    /**
     * 从指定位置读取一个 uint8_t
     * @param index  位置
     * @return
     */
    uint8_t ByteBuffer::get(uint32_t index) const {
        return read<uint8_t>(index);
    }

    /**
     * 读指定长度的内存到vector
     * @param buf 存储位置
     * @param len 读取长度
     * @return 已经读取到的长度
     */
    uint32_t ByteBuffer::getBytes(std::vector<uint8_t> &buf, uint32_t len) const {
        //check overflow
        for (uint32_t i = 0; i < len; i++) {
            if (pos_ + 1 > buff_size)
                return i;
            buf.push_back(read<uint8_t>());
        }
        return len;
    }


    uint32_t ByteBuffer::getBytes(uint8_t *buf, uint32_t len) const {
        //check overflow
        for (uint32_t i = 0; i < len; i++) {
            if (pos_ + 1 > buff_size)
                return i;
            buf[i] = read<uint8_t>();
        }
        return len;
    }


    ByteBuffer ByteBuffer::getByteBuffer(size_t offset, size_t size) {
        if (offset >= this->buff_size) {
            return ByteBuffer((uint8_t *) nullptr, 0);
        }
        if (size > this->buff_size) {
            return ByteBuffer((uint8_t *) nullptr, 0);
        }
        auto left = this->buff_size - offset;
        if (left < size) {
            size = left;
        }
        std::vector<uint8_t> buffer;
        buffer.resize(size);
        memcpy(buffer.data(), this->buf + offset, size);
        return ByteBuffer(buffer);
    }

    /**
   * 读取一个 char
   * @return char
   */
    char ByteBuffer::getChar() const {
        return read<char>();
    }

    /**
      * 从指定位置读取一个 char
      * @param index  位置
      * @return
      */
    char ByteBuffer::getChar(uint32_t index) const {
        return read<char>(index);
    }

    double ByteBuffer::getDouble() const {
        return read<double>();
    }

    /**
        * 从指定位置读取一个 double
        * @param index  位置
        * @return
        */
    double ByteBuffer::getDouble(uint32_t index) const {
        return read<double>(index);
    }

    float ByteBuffer::getFloat() const {
        return read<float>();
    }

    /**
        * 从指定位置读取一个 float
        * @param index  位置
        * @return
        */
    float ByteBuffer::getFloat(uint32_t index) const {
        return read<float>(index);
    }

    uint32_t ByteBuffer::getInt() const {
        return read<uint32_t>();
    }

    /**
          * 从指定位置读取一个 uint32_t
          * @param index  位置
          * @return
          */
    uint32_t ByteBuffer::getInt(uint32_t index) const {
        return read<uint32_t>(index);
    }

    uint64_t ByteBuffer::getLong() const {
        return read<uint64_t>();
    }

    uint64_t ByteBuffer::getLong(uint32_t index) const {
        return read<uint64_t>(index);
    }

    uint16_t ByteBuffer::getShort() const {
        return read<uint16_t>();
    }

    uint16_t ByteBuffer::getShort(uint32_t index) const {
        return read<uint16_t>(index);
    }

// Write Functions

    void ByteBuffer::put(ByteBuffer *src) {
        uint32_t len = src->size();
        for (uint32_t i = 0; i < len; i++)
            append<uint8_t>(src->get(i));
    }

    void ByteBuffer::put(uint8_t b) {
        append<uint8_t>(b);
    }

    void ByteBuffer::put(uint8_t b, uint32_t index) {
        insert<uint8_t>(b, index);
    }

    bool ByteBuffer::putBytes(const void *b, uint32_t len) {
        // Insert the data one byte at a time into the internal buffer at position i+starting index
        if (size() < (pos_ + len))
            return false;

        memcpy(&buf[pos_], (uint8_t *) b, len);
        //printf("writing %c to %i\n", (uint8_t)data, pos_);

        pos_ += len;
        return true;
    }

    void ByteBuffer::putBytes(uint8_t *b, uint32_t len, uint32_t index) {
        pos_ = index;

        // Insert the data one byte at a time into the internal buffer at position i+starting index
        for (uint32_t i = 0; i < len; i++)
            append<uint8_t>(b[i]);
    }

    void ByteBuffer::putChar(char value) {
        append<char>(value);
    }

    void ByteBuffer::putChar(char value, uint32_t index) {
        insert<char>(value, index);
    }

    void ByteBuffer::putDouble(double value) {
        append<double>(value);
    }

    void ByteBuffer::putDouble(double value, uint32_t index) {
        insert<double>(value, index);
    }

    void ByteBuffer::putFloat(float value) {
        append<float>(value);
    }

    void ByteBuffer::putFloat(float value, uint32_t index) {
        insert<float>(value, index);
    }

    void ByteBuffer::putInt(uint32_t value) {
        append<uint32_t>(value);
    }

    void ByteBuffer::putInt(uint32_t value, uint32_t index) {
        insert<uint32_t>(value, index);
    }

    void ByteBuffer::putLong(uint64_t value) {
        append<uint64_t>(value);
    }

    void ByteBuffer::putLong(uint64_t value, uint32_t index) {
        insert<uint64_t>(value, index);
    }

    void ByteBuffer::putShort(uint16_t value) {
        append<uint16_t>(value);
    }

    void ByteBuffer::putShort(uint16_t value, uint32_t index) {
        insert<uint16_t>(value, index);
    }

// Utility Functions

    void ByteBuffer::setName(std::string n) {
        name = n;
    }

    std::string ByteBuffer::getName() {
        return name;
    }

    bool ByteBuffer::hasRemaining() const {
        return pos_ < buff_size;
    }

    int ByteBuffer::remaining() const {
        return buff_size - pos_;
    }

    int ByteBuffer::position() const {
        return pos_;
    }

    std::unique_ptr<ByteBuffer> ByteBuffer::slice(size_t len, size_t postion) {
        if (len == 0) {
            len = buff_size;
        }
        ByteBuffer *zeroCopyByteBuffer = new ByteBuffer(buf, len);
        if (postion == -1) {
            postion = pos_;
        }
        zeroCopyByteBuffer->pos_ = postion;

        return std::unique_ptr<ByteBuffer>(zeroCopyByteBuffer);
    }

    uint8_t *ByteBuffer::getRawBuffer() const {
        return this->buf;
    }

    //
    bool ByteBuffer::available() {
        return pos_ < buff_size;
    }


    bool ByteBuffer::containsText(const char *other) const noexcept {
        if (other == nullptr) {
            return false;
        }
        size_t otherLen = strlen(other);
        if (buff_size < otherLen) {
            return -1;
        }
        // Preprocessing for Bad Character Heuristic
        std::vector<int> badCharShift(256, otherLen);
        for (int i = 0; i < otherLen - 1; ++i) {
            badCharShift[static_cast<int>(other[i])] = otherLen - i - 1;
        }

        // Boyer-Moore algorithm for string searching
        size_t i = 0;
        while (i <= buff_size - otherLen) {
            int j = otherLen - 1;
            while (j >= 0 && buf[i + j] == other[j]) {
                j--;
            }
            if (j < 0) {
                return true; // Match found
            } else {
                i += badCharShift[static_cast<int>(buf[i + j])] > otherLen - j ?
                     badCharShift[static_cast<int>(buf[i + j])] :
                     otherLen - j;
            }
        }
        return false; // No match found
    }

    int ByteBuffer::indexOf(const uint8_t *pattern, size_t patternSize) const noexcept {
        // Boyer-Moore algorithm for byte array searching
        if (buff_size < patternSize || pattern == nullptr) {
            return -1;
        }
        // Preprocessing for Bad Character Heuristic
        int badCharShift[256];
        for (int i = 0; i < 256; ++i) {
            badCharShift[i] = patternSize;
        }
        for (size_t i = 0; i < patternSize - 1; ++i) {
            badCharShift[static_cast<int>(pattern[i])] = patternSize - i - 1;
        }

        // Boyer-Moore algorithm for byte array searching
        size_t i = 0;
        while (i <= buff_size - patternSize) {
            int j = patternSize - 1;
            while (j >= 0 && buf[i + j] == pattern[j]) {
                j--;
            }
            if (j < 0) {
                return static_cast<int>(i); // Match found, return index
            } else {
                i += badCharShift[static_cast<int>(buf[i + j])] > patternSize - j ?
                     badCharShift[static_cast<int>(buf[i + j])] :
                     patternSize - j;
            }
        }
        return -1; // No match found
    }

    bool ByteBuffer::containsText(const StringRef &ref) const noexcept {
        if (ref.isEmpty()) {
            return false;
        }
        if (ref.text.getAddress() == nullptr) {
            return false;
        }
        return containsText(ref.text.getAddress());
    }

    int ByteBuffer::indexOfText(const char *other) const noexcept {
        if (other == nullptr) {
            return -1;
        }
        const uint8_t *buffer = buf; // Replace this with how you get your buffer
        size_t bufferSize = buff_size; // Replace this with how you get your buffer size
        size_t otherLen = strlen(other);
        if (buff_size < otherLen) {
            return -1;
        }

        // Boyer-Moore algorithm for byte array searching

        // Preprocessing for Bad Character Heuristic
        int badCharShift[256];
        for (int i = 0; i < 256; ++i) {
            badCharShift[i] = otherLen;
        }
        for (size_t i = 0; i < otherLen - 1; ++i) {
            badCharShift[static_cast<int>(other[i])] = otherLen - i - 1;
        }

        // Boyer-Moore algorithm for byte array searching
        size_t i = 0;
        while (i <= bufferSize - otherLen) {
            int j = otherLen - 1;
            while (j >= 0 && buffer[i + j] == static_cast<uint8_t>(other[j])) {
                j--;
            }
            if (j < 0) {
                return i; // Match found, return index
            } else {
                i += badCharShift[static_cast<int>(buffer[i + j])] > otherLen - j ?
                     badCharShift[static_cast<int>(buffer[i + j])] :
                     otherLen - j;
            }
        }
        return (-1); // No match found
    }

    int ByteBuffer::indexOfText(size_t start_pos, const char *other) const noexcept {
        if (start_pos >= buff_size || other == nullptr) {
            return -1;
        }
        const uint8_t *buffer = buf + start_pos;
        size_t bufferSize = buff_size - start_pos;

        size_t otherLen = strlen(other);
        if (buff_size < otherLen) {
            return -1;
        }
        // Boyer-Moore algorithm for byte array searching
        std::vector<int> badCharShift(256, otherLen);

        for (size_t i = 0; i < otherLen - 1; ++i) {
            badCharShift[static_cast<int>(other[i])] = otherLen - i - 1;
        }

        size_t i = 0;
        while (i <= bufferSize - otherLen) {
            int j = otherLen - 1;
            while (j >= 0 && buffer[i + j] == static_cast<uint8_t>(other[j])) {
                j--;
            }
            if (j < 0) {
                return static_cast<int>(start_pos + i); // Match found, return index
            } else {
                i += badCharShift[static_cast<int>(buffer[i + j])] > otherLen - j ?
                     badCharShift[static_cast<int>(buffer[i + j])] :
                     otherLen - j;
            }
        }

        return -1; // No match found
    }

    ByteBuffer::ByteBuffer(InputStream *stream) {
        size_t byte_length = stream->getTotalLength();
        this->data_buffer.resize(byte_length);
        auto read = stream->read(this->data_buffer.data(), byte_length);
        this->buf = data_buffer.data();
        this->buff_size = byte_length;
        name = "";
    }

    ByteBuffer::ByteBuffer(File *file, bool readOnly) {
        CHECK(file != nullptr);
        auto path = file->getFullPathName();
        this->memoryFile = new MemoryFile(path.c_str(), readOnly);
        CHECK_EQ(this->memoryFile->isOpen(), true);
        this->buf = this->memoryFile->data();
        this->buff_size = this->memoryFile->size();
        name = "";
    }

    ByteBuffer::~ByteBuffer() {
        if (this->memoryFile != nullptr) {
            delete memoryFile;
        }
    }

    ByteBuffer::ByteBuffer(std::vector<uint8_t> data) {
        this->data_buffer = std::move(data);
        this->buf = data_buffer.data();
        this->buff_size = data_buffer.size();
    }
}