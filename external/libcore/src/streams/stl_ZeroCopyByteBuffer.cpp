/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/16 15:00
 * @version: 1.0
 * @description: 
*/
#include "streams/stl_ZeroCopyByteBuffer.h"

namespace stl {

/**
 * ZeroCopyByteBuffer constructor
 * Consume an entire uint8_t array of length len in the ZeroCopyByteBuffer
 *
 * @param arr uint8_t array of data (should be of length len)
 * @param size Size of space to allocate
 */
    ZeroCopyByteBuffer::ZeroCopyByteBuffer(uint8_t *arr, size_t size) {
        // If the provided array is NULL, allocate a blank buffer of the provided size
        this->buf = arr;
        this->buff_size = size;
        name = "";
    }

/**
 * Bytes Remaining
 * Returns the number of bytes from the current read position till the end of the buffer
 *
 * @return Number of bytes from rpos to the end (size())
 */
    uint32_t ZeroCopyByteBuffer::bytesRemaining() {
        return size() - pos_;
    }

/**
 * Clear
 * Clears out all data from the internal vector (original preallocated size remains), resets the positions to 0
 */
    void ZeroCopyByteBuffer::clear() {
        pos_ = 0;
    }


/**
 * Equals, test for data equivilancy
 * Compare this ZeroCopyByteBuffer to another by looking at each byte in the internal buffers and making sure they are the same
 *
 * @param other A pointer to a ZeroCopyByteBuffer to compare to this one
 * @return True if the internal buffers match. False if otherwise
 */
    bool ZeroCopyByteBuffer::equals(ZeroCopyByteBuffer *other) {
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


/**
 * Size
 * Returns the size of the internal buffer...not necessarily the length of bytes used as data!
 *
 * @return size of the internal buffer
 */
    uint32_t ZeroCopyByteBuffer::size() {
        return buff_size;
    }

// Replacement

/**
 * Replace
 * Replace occurance of a particular uint8_t, key, with the uint8_t rep
 *
 * @param key uint8_t to find for replacement
 * @param rep uint8_t to replace the found key with
 * @param start Index to start from. By default, start is 0
 * @param firstOccuranceOnly If true, only replace the first occurance of the key. If false, replace all occurances. False by default
 */
    void ZeroCopyByteBuffer::replace(uint8_t key, uint8_t rep, uint32_t start, bool firstOccuranceOnly) {
        uint32_t len = buff_size;
        for (uint32_t i = start; i < len; i++) {
            uint8_t data = read<uint8_t>(i);
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

// Read Functions

    uint8_t ZeroCopyByteBuffer::peek() const {
        return read<uint8_t>(pos_);
    }

    uint8_t ZeroCopyByteBuffer::get() const {
        return read<uint8_t>();
    }

    uint8_t ZeroCopyByteBuffer::get(uint32_t index) const {
        return read<uint8_t>(index);
    }

    uint32_t ZeroCopyByteBuffer::getBytes(std::vector<uint8_t> &buf, uint32_t len) const {
        //check overflow
        for (uint32_t i = 0; i < len; i++) {
            if (pos_ + 1 > buff_size)
                return i;
            buf.push_back(read<uint8_t>());
        }
        return len;
    }

    uint32_t ZeroCopyByteBuffer::getBytes(uint8_t *buf, uint32_t len) const {
        //check overflow
        for (uint32_t i = 0; i < len; i++) {
            if (pos_ + 1 > buff_size)
                return i;
            buf[i] = read<uint8_t>();
        }
        return len;
    }

    ZeroCopyByteBuffer ZeroCopyByteBuffer::getByteBuffer(size_t offset, size_t size) {
        if (offset >= this->buff_size) {
            return {nullptr, 0};
        }
        if (size > this->buff_size) {
            return {nullptr, 0};
        }
        auto left = this->buff_size - offset;
        if (left < size) {
            size = left;
        }
        return {this->buf + offset, size};
    }


    char ZeroCopyByteBuffer::getChar() const {
        return read<char>();
    }

    char ZeroCopyByteBuffer::getChar(uint32_t index) const {
        return read<char>(index);
    }

    double ZeroCopyByteBuffer::getDouble() const {
        return read<double>();
    }

    double ZeroCopyByteBuffer::getDouble(uint32_t index) const {
        return read<double>(index);
    }

    float ZeroCopyByteBuffer::getFloat() const {
        return read<float>();
    }

    float ZeroCopyByteBuffer::getFloat(uint32_t index) const {
        return read<float>(index);
    }

    uint32_t ZeroCopyByteBuffer::getInt() const {
        return read<uint32_t>();
    }

    uint32_t ZeroCopyByteBuffer::getInt(uint32_t index) const {
        return read<uint32_t>(index);
    }

    uint64_t ZeroCopyByteBuffer::getLong() const {
        return read<uint64_t>();
    }

    uint64_t ZeroCopyByteBuffer::getLong(uint32_t index) const {
        return read<uint64_t>(index);
    }

    uint16_t ZeroCopyByteBuffer::getShort() const {
        return read<uint16_t>();
    }

    uint16_t ZeroCopyByteBuffer::getShort(uint32_t index) const {
        return read<uint16_t>(index);
    }

// Write Functions

    void ZeroCopyByteBuffer::put(ZeroCopyByteBuffer *src) {
        uint32_t len = src->size();
        for (uint32_t i = 0; i < len; i++)
            append<uint8_t>(src->get(i));
    }

    void ZeroCopyByteBuffer::put(uint8_t b) {
        append<uint8_t>(b);
    }

    void ZeroCopyByteBuffer::put(uint8_t b, uint32_t index) {
        insert<uint8_t>(b, index);
    }

    bool ZeroCopyByteBuffer::putBytes(const void *b, uint32_t len) {
        // Insert the data one byte at a time into the internal buffer at position i+starting index
        if (size() < (pos_ + len))
            return false;

        memcpy(&buf[pos_], (uint8_t *) b, len);
        //printf("writing %c to %i\n", (uint8_t)data, pos_);

        pos_ += len;
        return true;
    }

    void ZeroCopyByteBuffer::putBytes(uint8_t *b, uint32_t len, uint32_t index) {
        pos_ = index;

        // Insert the data one byte at a time into the internal buffer at position i+starting index
        for (uint32_t i = 0; i < len; i++)
            append<uint8_t>(b[i]);
    }

    void ZeroCopyByteBuffer::putChar(char value) {
        append<char>(value);
    }

    void ZeroCopyByteBuffer::putChar(char value, uint32_t index) {
        insert<char>(value, index);
    }

    void ZeroCopyByteBuffer::putDouble(double value) {
        append<double>(value);
    }

    void ZeroCopyByteBuffer::putDouble(double value, uint32_t index) {
        insert<double>(value, index);
    }

    void ZeroCopyByteBuffer::putFloat(float value) {
        append<float>(value);
    }

    void ZeroCopyByteBuffer::putFloat(float value, uint32_t index) {
        insert<float>(value, index);
    }

    void ZeroCopyByteBuffer::putInt(uint32_t value) {
        append<uint32_t>(value);
    }

    void ZeroCopyByteBuffer::putInt(uint32_t value, uint32_t index) {
        insert<uint32_t>(value, index);
    }

    void ZeroCopyByteBuffer::putLong(uint64_t value) {
        append<uint64_t>(value);
    }

    void ZeroCopyByteBuffer::putLong(uint64_t value, uint32_t index) {
        insert<uint64_t>(value, index);
    }

    void ZeroCopyByteBuffer::putShort(uint16_t value) {
        append<uint16_t>(value);
    }

    void ZeroCopyByteBuffer::putShort(uint16_t value, uint32_t index) {
        insert<uint16_t>(value, index);
    }

// Utility Functions

    void ZeroCopyByteBuffer::setName(std::string n) {
        name = n;
    }

    std::string ZeroCopyByteBuffer::getName() {
        return name;
    }

    void ZeroCopyByteBuffer::printInfo() {
        uint32_t length = buff_size;
        std::cout << "ZeroCopyByteBuffer " << name.c_str() << " Length: " << length << ". Info Print" << std::endl;
    }

    void ZeroCopyByteBuffer::printAH() {
        uint32_t length = buff_size;
        std::cout << "ZeroCopyByteBuffer " << name.c_str() << " Length: " << length << ". ASCII & Hex Print"
                  << std::endl;

        for (uint32_t i = 0; i < length; i++) {
            std::printf("0x%02x ", buf[i]);
        }

        std::printf("\n");
        for (uint32_t i = 0; i < length; i++) {
            std::printf("%c ", buf[i]);
        }

        std::printf("\n");
    }

    void ZeroCopyByteBuffer::printAscii() {
        uint32_t length = buff_size;
        std::cout << "ZeroCopyByteBuffer " << name.c_str() << " Length: " << length << ". ASCII Print" << std::endl;

        for (uint32_t i = 0; i < length; i++) {
            std::printf("%c ", buf[i]);
        }

        std::printf("\n");
    }

    void ZeroCopyByteBuffer::printHex() {
        uint32_t length = buff_size;
        std::cout << "ZeroCopyByteBuffer " << name.c_str() << " Length: " << length << ". Hex Print" << std::endl;

        for (uint32_t i = 0; i < length; i++) {
            std::printf("0x%02x ", buf[i]);
        }

        std::printf("\n");
    }

    void ZeroCopyByteBuffer::printPosition() {
        uint32_t length = buff_size;
        std::cout << "ZeroCopyByteBuffer " << name.c_str() << " Length: " << length << " Read Pos: " << pos_
                  << ". Write Pos: "
                  << pos_ << std::endl;
    }

    bool ZeroCopyByteBuffer::hasRemaining() {
        return pos_ < buff_size;
    }

    int ZeroCopyByteBuffer::remaining() {
        return buff_size - pos_;
    }

    int ZeroCopyByteBuffer::position() {
        return pos_;
    }

    std::unique_ptr<ZeroCopyByteBuffer> ZeroCopyByteBuffer::getByteBuffer(size_t size) {
        if (size < 0) {
            return nullptr;
        }
        int position_ = position();
        int limit = position_ + size;
        if ((limit < position_)) {
            return nullptr;
        }
        return std::make_unique<ZeroCopyByteBuffer>(this->buf + position(), size);
    }

    std::unique_ptr<ZeroCopyByteBuffer> ZeroCopyByteBuffer::slice(size_t len, size_t postion) {
        if (len == 0) {
            len = buff_size;
        }
        ZeroCopyByteBuffer *zeroCopyByteBuffer = new ZeroCopyByteBuffer(buf, len);
        if (postion == -1) {
            postion = pos_;
        }
        zeroCopyByteBuffer->pos_ = postion;

        return std::unique_ptr<ZeroCopyByteBuffer>(zeroCopyByteBuffer);
    }

    uint8_t *ZeroCopyByteBuffer::getRawBuffer() const {
        return this->buf;
    }

    //
    bool ZeroCopyByteBuffer::available() {
        return pos_ < buff_size;
    }

    uint8_t *ZeroCopyByteBuffer::getPositionBuffer() {
        return buf + pos_;
    }

    bool ZeroCopyByteBuffer::containsText(const char *other) const noexcept {
        if (other == nullptr) {
            return false;
        }
        size_t otherLen = strlen(other);
        if (otherLen <= 0) {
            return false;
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

    int ZeroCopyByteBuffer::indexOf(const uint8_t *pattern, size_t patternSize) const noexcept {
        // Boyer-Moore algorithm for byte array searching
        if (pattern == nullptr && patternSize <= 0) {
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

    bool ZeroCopyByteBuffer::containsText(const StringRef &ref) const noexcept {
        return containsText(ref.text.getAddress());
    }

    int ZeroCopyByteBuffer::indexOfText(const char *other) const noexcept {
        if (other == nullptr) {
            return -1;
        }
        const uint8_t *buffer = buf; // Replace this with how you get your buffer
        size_t bufferSize = buff_size; // Replace this with how you get your buffer size

        size_t otherLen = strlen(other);
        if (otherLen <= 0) {
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

    int ZeroCopyByteBuffer::indexOfText(size_t start_pos, const char *other) const noexcept {
        if (start_pos >= buff_size || other == nullptr) {
            return false;
        }
        const uint8_t *buffer = buf + start_pos;
        size_t bufferSize = buff_size - start_pos;

        size_t otherLen = strlen(other);
        if (otherLen <= 0) {
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
}