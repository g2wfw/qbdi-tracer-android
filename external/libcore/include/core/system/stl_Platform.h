/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/12/26 17:22
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STL_PLATFORM_H
#define MANXI_CORE_STL_PLATFORM_H

#include "../text/stl_String.h"
#include "../text/stl_StringRef.h"

namespace stl {
    class STL_API Platform {
    public:
        static stl::String GetExecutablePath();

        static stl::String GetExecutableDirectory();

        /**
         * 控制台应用，等待应用输入任意字符
         * unix 回车按键
         * windows 任意按键
         * @param msg 需要打印的消息
         */
        static void PressAnyKeyWait(const stl::String &msg);

    };
}

#endif //MANXI_CORE_STL_PLATFORM_H
