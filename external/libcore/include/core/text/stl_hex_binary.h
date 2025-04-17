/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/10 13:14
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STL_HEX_BINARY_H
#define MANXI_CORE_STL_HEX_BINARY_H

#include "../stl_core.h"
#include "stl_String.h"

namespace stl {
    class STL_EXPORT HexBinary final {
    public:
        static String encodeToHex(const String &text);

        static String encodeToHex(const void *data, size_t length);

    };
}


#endif //MANXI_CORE_STL_HEX_BINARY_H
