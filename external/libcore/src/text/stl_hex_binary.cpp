/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/10 13:14
 * @version: 1.0
 * @description: 
*/

#include "text/stl_hex_binary.h"

namespace stl {

    String HexBinary::encodeToHex(const String &text) {
        return encodeToHex(text.toRawUTF8(), text.length());
    }

    String HexBinary::encodeToHex(const void *data, size_t length) {
        const auto *bytes = static_cast<const unsigned char *>(data);
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << static_cast<unsigned int>(bytes[i]);
        }
        std::string result = ss.str();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }


}
