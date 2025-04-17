/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2024/1/9 15:43
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STL_BYTEBUFFERSTREAM_H
#define MANXI_CORE_STL_BYTEBUFFERSTREAM_H

#include "../stl_macro.h"
#include "../stl_bit_utils.h"
#include "../logging/check.h"
#include <vector>
#include <cstring>
namespace stl {

    class STL_EXPORT ByteBufferStream {
    public:
        explicit ByteBufferStream(size_t buffer_size);

        template<typename type>
        type get() {
            uint32_t s = sizeof(type);
            CHECK_LE(position_ + s, capacity_);
            type *result = (type *) (buffer_data_ + position_);
            position_ += s;
            return *result;
        }

        template<typename type>
        type get(size_t position) {
            uint32_t s = sizeof(type);
            CHECK_LE(position + s, capacity_);
            type *result = (type *) (buffer_data_ + position);
            return *result;
        }

        template<typename type>
        type *get() {
            uint32_t s = sizeof(type);
            CHECK_LE(position_ + s, capacity_);
            type *result = (type *) (buffer_data_ + position_);
            position_ += s;
            return result;
        }

        template<typename type>
        bool put(type data) {
            uint32_t s = sizeof(type);
            EnsureStorage(s);
            memcpy(this->buffer_data_ + position_, &data, s);
            this->position_ += s;
            if (position_ > this->real_size_) {
                this->real_size_ = position_;
            }
            return true;
        }

        bool write(const void *buffer, size_t length) {
            EnsureStorage(length);
            memcpy(this->buffer_data_ + position_, buffer, length);
            this->position_ += length;
            if (position_ > this->real_size_) {
                this->real_size_ = position_;
            }
            return true;
        }

        size_t tell() const {
            return this->position_;
        }

        void seek(size_t position) {
            this->position_ = position;
            EnsureStorage(0);
        }

        size_t getRealSize() const {
            return this->real_size_;
        }

        uint8_t *Begin() {
            return data_vector.data();
        }

        size_t getBufferSize() const {
            return this->data_vector.size();
        }


        size_t clear(size_t position, size_t length) {
            EnsureStorage(length);
            memset(&buffer_data_[position], 0, length);
            return length;
        }

        void alignTo(const size_t alignment) {
            auto real_size = RoundUp(real_size_, alignment);
            if (real_size - real_size_ >= 0) {
                EnsureStorage(real_size - real_size_);
                real_size_ = real_size;
            }
        }


        void skip(const size_t count) {
            position_ += count;
            EnsureStorage(0u);
        }

        std::vector<uint8_t> toVector();

        void resize(size_t i);

    private:
        ALWAYS_INLINE void EnsureStorage(size_t length) {
            size_t end = position_ + length;
            while (UNLIKELY(end > capacity_)) {
                data_vector.resize(capacity_ * 3 / 2 + 1);
                SyncWithSection();
            }
        }

        ALWAYS_INLINE void EnsureStorageCap(size_t length) {
            size_t end = capacity_ + length;
            while (UNLIKELY(end > capacity_)) {
                data_vector.resize(capacity_ * 3 / 2 + 1);
                SyncWithSection();
            }
        }

        ALWAYS_INLINE void SyncWithSection() {
            buffer_data_ = &data_vector[0];
            capacity_ = data_vector.size();
        }


    private:

        std::vector<uint8_t> data_vector;
        uint8_t *buffer_data_ = nullptr;//
        size_t capacity_ = 0;//
        size_t real_size_ = 0;//实际所占大小
        size_t position_ = 0;
    private:
        DISALLOW_COPY_AND_ASSIGN(ByteBufferStream);
    };
}
#endif //MANXI_CORE_STL_BYTEBUFFERSTREAM_H
