#include "stl_core.h"
#include "logging/stllog.h"
#include <thread>
#include <core/text/stl_String.h>

#include <core/files/stl_File.h>
#include <core/text/stl_hex_binary.h>
#include <core/files/stl_FileInputStream.h>
#include <core/containers/stl_Collections.h>
#include <core/streams/stl_ZeroCopyByteBuffer.h>
#include <core/streams/stl_ByteBuffer.h>

void CollectionsTest() {

}

void LogTest() {
    stl::set_pattern("[%Y-%m-%d %T] %P %^%8l%$ %@: %v");
    stl::info("hello");
    stl::warn("hello");
    stl::debug("hello");
    stl::critical("hello");
    stl::error("hello");
    stl::trace("hello");
    stl::fatal("will abort");
    stl::log({__FILE__, __LINE__, __FUNCTION__}, stl::level::err, "hello2");
}


void strTest() {
    {
        stl::String text = "hello";
        stl::info("stl::String length:{} size:{}", text.length(), text.size());
        stl::String text2 = "你好";
        stl::info("stl::String length:{} size:{}", text2.length(), text2.size());
        std::string text3 = "hello";
        stl::info("std::string hello length:{} size:{}", text3.length(), text3.size());

        std::string text4 = "你好,我是你的朋友";
        stl::info("std::string 你好 length:{} size:{}", text4.length(), text4.size());
        stl::info("std::string 你好 index:{}", text4.find("朋"));
    }

    {
        stl::String text = "s,23,455";
        stl::info("indexOfChar:{}", text.indexOfChar(','));
        stl::info("indexOfChar:{}", text.lastIndexOfChar(','));

        stl::String text_ch = "你好，我是你的朋友";
        stl::info("indexOfChar:{}", text_ch.indexOf("，"));
        stl::info("lastIndexOf:{}", text_ch.lastIndexOf("朋"));
        stl::info("lastIndexOf:{}", text_ch.lastIndexOfChar(L'朋'));
        text_ch.contains("");
    }
    {
        stl::String empty;
        stl::info("length:{}", empty.length());
        stl::info("size:{}", empty.size());
    }
}


void fileTest() {
    stl::File file("hap_test");
    std::unique_ptr<stl::ByteBuffer> byteBuffer = file.readByteBuffer();
    stl::info("containsText:{}", byteBuffer->containsText("itsIwEEE2idE"));
    stl::info("index:{}", byteBuffer->indexOfText("itsIwEEE2idE"));

}

void strToZeroCopyByteBuffer() {
    stl::String text = "hello,this is string test";
    //创建零拷贝字符串，该类直接操作String的内存，不声明新的内存
    auto buffer = text.createZeroCopyByteBuffer();
    stl::info("indexOfText:{}", buffer->indexOfText("string"));
    //containsText 和 stl::String::contains 不同，
    //containsText 是内存搜索模式，String::contains是字符索引模式
    stl::info("containsText:{}", buffer->containsText("string"));
}


int main() {
    strToZeroCopyByteBuffer();
    //strTest();
    //LogTest();
    //CollectionsTest();

    return 0;
}