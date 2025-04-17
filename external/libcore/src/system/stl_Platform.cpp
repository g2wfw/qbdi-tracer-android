/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/12/26 17:24
 * @version: 1.0
 * @description: 
*/
#include "system/stl_Platform.h"
#include <files/stl_File.h>
#include <logging/stllog.h>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#endif
#if defined(_WIN32)

#include <windows.h>
#include <libgen.h>
#include <mutex>
#include <conio.h>


#define O_CLOEXEC O_NOINHERIT
#define O_NOFOLLOW 0
#endif

namespace stl {


#ifndef __WIN32

    static bool Readlink(const std::string &path, std::string *result) {
        result->clear();

        // Most Linux file systems (ext2 and ext4, say) limit symbolic links to
        // 4095 bytes. Since we'll copy out into the string anyway, it doesn't
        // waste memory to just start there. We add 1 so that we can recognize
        // whether it actually fit (rather than being truncated to 4095).
        std::vector<char> buf(4095 + 1);
        while (true) {
#if defined(__linux__) && defined(SAFE_CALL)
            ssize_t size = sys_readlink(path.c_str(), &buf[0], buf.size());
#else
            ssize_t size = readlink(path.c_str(), &buf[0], buf.size());
#endif

            // Unrecoverable error?
            if (size == -1) return false;
            // It fit! (If size == buf.size(), it may have been truncated.)
            if (static_cast<size_t>(size) < buf.size()) {
                result->assign(&buf[0], size);
                return true;
            }
            // Double our buffer and try again.
            buf.resize(buf.size() * 2);
        }
    }

#endif

    stl::String Platform::GetExecutablePath() {
#if defined(__linux__)
        std::string path;
        Readlink("/proc/self/exe", &path);
        return path;
#elif defined(__APPLE__)
        char path[PATH_MAX + 1];
        uint32_t path_len = sizeof(path);
        int rc = _NSGetExecutablePath(path, &path_len);
        if (rc < 0) {
          std::unique_ptr<char> path_buf(new char[path_len]);
          _NSGetExecutablePath(path_buf.get(), &path_len);
          return path_buf.get();
        }
        return path;
#elif defined(_WIN32)
        char path[PATH_MAX + 1];
        DWORD result = GetModuleFileNameA(NULL, path, sizeof(path) - 1);
        if (result == 0 || result == sizeof(path) - 1) return "";
        path[PATH_MAX - 1] = 0;
        return path;
#else
#error unknown OS
#endif
        return "";
    }

    stl::String Platform::GetExecutableDirectory() {
        stl::File file(GetExecutablePath());
        if (!file.existsAsFile()) {
            return "";
        }
        return file.getParentDirectory().getFullPathName();
    }

    void Platform::PressAnyKeyWait(const String &msg) {
#ifdef _WIN32
        stl::info("{},please press any key .......", msg);
        while (!_kbhit()) {
            // 等待按键
        }
        _getch(); // 读取按键，但不显示
        return;
#else
        stl::info("{},please press Enter key .......", msg);
        std::cin.get();
        return;
#endif
    }



}