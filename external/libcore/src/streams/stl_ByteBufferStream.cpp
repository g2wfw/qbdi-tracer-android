/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2024/1/9 15:45
 * @version: 1.0
 * @description: 
*/
#include "streams/stl_ByteBufferStream.h"

namespace stl {
    ByteBufferStream::ByteBufferStream(size_t buffer_size) {
        EnsureStorage(buffer_size);
    }

    std::vector<uint8_t> ByteBufferStream::toVector() {
        std::vector<uint8_t> data;
        data.insert(data.begin(), this->data_vector.data(), this->data_vector.data() + real_size_);
        return data;
    }

    void ByteBufferStream::resize(size_t i) {
        data_vector.resize(i);
        SyncWithSection();
    }
}