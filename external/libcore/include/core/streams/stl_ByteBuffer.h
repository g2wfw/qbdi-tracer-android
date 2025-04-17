/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/16 14:59
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STL_BYTEBUFFER_H
#define MANXI_CORE_STL_BYTEBUFFER_H

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <memory>
#include <iostream>
#include <cstdio>
#include "../text/stl_String.h"
#include "../text/stl_StringRef.h"
#include "stl_InputStream.h"

namespace stl {
    class File;

    class MemoryFile;

    class STL_API ByteBuffer {
    public:
        /**
         * 通过指定内存和大小构造ByteBuffer
         * 可写和读由arr属性决定
         * @param arr 内存地址
         * @param size 内存大小
         */
        explicit ByteBuffer(uint8_t *arr, size_t size);

        /**
         * 通过流 构造ByteBuffer
         * 可读可写
         * @param stream
         */
        explicit ByteBuffer(InputStream *stream);

        /**
         * 通过File对象 构造ByteBuffer
         * readOnly 为ture时候一旦改动流数据，将会直接同步到文件
         * 通过此函数时，一定需要注意，修改任何数据，将会直接修改原始文件
         * 不需要再次写入
         * @param file
         * @param readOnly 是否可写
         */
        explicit ByteBuffer(File *file, bool readOnly = true);

        ~ByteBuffer();

        /**
         * 内存下标到文件结束位置的差值
         * 这个下标内部维护也可以通过 setPosition getPosition 处理下标信息
         * @return 差值信息
         */
        uint32_t bytesRemaining() const;

        /**
         * 相当于 setPosition(0)
         */
        void clear();

        /**
         * 比较两个Bytebuffer内容是否相同
         * @param other
         * @return
         */
        bool equals(ByteBuffer *other) const; // Compare if the contents are equivalent
        /**
         * 内存切片，切片维护的数据和当前维护数据相同
         * 需要注意的是 这里和 getByteBuffer 不是一个概念
         * 需要拷贝数据使用 getByteBuffer
         * @param len 切片长度
         * @param position 切片下边位置
         * @return 切片对象
         */
        std::unique_ptr<ByteBuffer> slice(size_t len = 0, size_t position = -1);

        /**
         * 返回一个新的对象，这个新的对象会拷贝需要的数据，和原始数据不同
         * 内存拷贝，这里返回的对象和当前对象维护的不是同一个数据，
         * 和 slice 函数不是一个概念
         * 需要切片数据使用 slice
         * @param offset 偏移
         * @param size 长度
         * @return 拷贝对象
         */
        ByteBuffer getByteBuffer(size_t offset, size_t size);

        /**
         * 获取当前对象控制数据的指针
         * @return
         */
        uint8_t *getRawBuffer() const;

        /**
         * 获取当前控制数据的大小
         * @return 数据大小
         */
        uint32_t size() const;

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

        /**
         * 在当前数据中查找指定数据位置
         * @param pattern 内存数据
         * @param patternSize  内存数据大小
         * @return  -1:未查找到
         */
        int indexOf(const uint8_t *pattern, size_t patternSize) const noexcept;

        /**
        * 在当前数据中查找字符串位置
        * @param other 内存数据
        * @return  -1:未查找到
        */
        int indexOfText(const char *other) const noexcept;

        /**
         * 在当前数据中查找字符串位置
         * @param start_pos  查找起始位置
         * @param other 需要查找的字符串
         * @return -1:未查找到
         */
        int indexOfText(size_t start_pos, const char *other) const noexcept;

        /**
         * 判断是否包含某个字符串
         * @param other
         * @return
         */
        bool containsText(const char *other) const noexcept;

        bool containsText(const StringRef &ref) const noexcept;

        /**
         * 在当前位置读取一个uint8_t
         * 注意改函数不会修改下标 和 get 函数不一样
         * get 函数会修改下标
         * @return
         */
        uint8_t peek() const;

        /**
         * 在当前位置读取一个uint8_t
         * 注意改函数下标+1
         * @return
         */
        uint8_t get() const;

        /**
         * 从指定下标读取一个 uint8_t
         * 注意改函数不会修改下标
         * @param index 下标位置
         * @return
         */
        uint8_t get(uint32_t index) const;

        /**
         * 读指定长度的内存到指定内存，
         * 注意改函数自动修改下标
         * @param buf 存储位置，这里需要提前申请内存
         * @param len 读取长度
         * @return 已经读取到的长度
         */
        uint32_t getBytes(uint8_t *buf, uint32_t len) const;

        /**
         * 读指定长度的内存到指定内存，
         * 注意改函数自动修改下标
         * @param buf 存储位置，这里需要提前申请内存
         * @param len 读取长度
         * @return 已经读取到的长度
         */
        uint32_t getBytes(std::vector<uint8_t> &buf, uint32_t len) const;

        /**
         * 在当前位置读取一个 char
         * 注意改函数下标+1
         * @return
         */
        char getChar() const;

        /**
         * 从指定下标读取一个 char
         * 注意改函数不会修改下标
         * @param index 下标位置
         * @return
         */
        char getChar(uint32_t index) const;

        /**
         * 在当前位置读取一个 double
         * 注意改函数下标+sizeof(double)
         * @return
         */
        double getDouble() const;

        /**
         * 从指定下标读取一个 double
         * 注意改函数不会修改下标
         * @param index 下标位置
         * @return
         */
        double getDouble(uint32_t index) const;

        /**
         * 在当前位置读取一个 float
         * 注意改函数下标+sizeof(float)
         * @return
         */
        float getFloat() const;

        /**
         * 从指定下标读取一个 float
         * 注意改函数不会修改下标
         * @param index 下标位置
         * @return
         */
        float getFloat(uint32_t index) const;

        /**
         * 在当前位置读取一个 uint32_t
         * 注意改函数下标+sizeof(uint32_t)
         * @return
         */
        uint32_t getInt() const;

        /**
         * 从指定下标读取一个 uint32_t
         * 注意改函数不会修改下标
         * @param index 下标位置
         * @return
         */
        uint32_t getInt(uint32_t index) const;

        /**
         * 在当前位置读取一个 uint64_t
         * 注意改函数下标+sizeof(uint64_t)
         * @return
         */
        uint64_t getLong() const;

        /**
         * 从指定下标读取一个 uint64_t
         * 注意改函数不会修改下标
         * @param index 下标位置
         * @return
         */
        uint64_t getLong(uint32_t index) const;

        /**
         * 在当前位置读取一个 uint16_t
         * 注意改函数下标+sizeof(uint16_t)
         * @return
         */
        uint16_t getShort() const;

        /**
         * 从指定下标读取一个 uint16_t
         * 注意改函数不会修改下标
         * @param index 下标位置
         * @return
         */
        uint16_t getShort(uint32_t index) const;

        /**
         * 写数据，将指定类型的数据写入到内存
         * 会修改下标
         * @tparam T 数据类型
         * @param t 数据
         * @return
         */
        template<typename T>
        bool putData(T t) {
            uint32_t s = sizeof(t);
            if (size() < (pos_ + s))
                return false;
            memcpy(buf + pos_, (uint8_t *) &t, s);
            pos_ += s;
            return true;
        };

        /**
         * 写数据，将指定类型的数据写入到内存
         * 会修改下标
         * @param src 数据
         * @return
         */
        void put(ByteBuffer *src);

        void put(uint8_t b);

        void put(uint8_t b, uint32_t index);

        bool putBytes(const void *b, uint32_t len);

        /**
         * 将数据写入到指定位置
         * @param b
         * @param len
         * @param position 指定位置
         */
        void putBytes(uint8_t *b, uint32_t len, uint32_t position);

        void putChar(char value);

        void putChar(char value, uint32_t index);

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
        void setPosition(uint32_t w) {
            pos_ = w;
        }

        uint32_t getPosition() const {
            return pos_;
        }

        /**
         * 设置当前名称
         * @param n
         */
        void setName(std::string n);

        std::string getName();

        /**
         * 是否由剩余
         * = position-size>0
         * @return position-size>0
         */
        bool hasRemaining() const;

        /**
         * 剩余字节
         * @return =position-size
         */
        int remaining() const;

        int position() const;

        //判断当前buffer是否可用
        bool available();

        /**
         * 可以用来读取结构体
         * 自己注意结构体对齐问题
         * @tparam T
         * @return
         */
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
        explicit ByteBuffer(std::vector<uint8_t> data);

    private:
        mutable int64 pos_ = 0;
        MemoryFile *memoryFile = nullptr;
        std::vector<uint8_t> data_buffer;
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
#endif //MANXI_CORE_STL_BYTEBUFFER_H
