// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "../common.h"
#include <ctime> // std::time_t

namespace stl {
namespace details {
namespace os {

STL_LOG_API stl::log_clock::time_point now() STL_LOG_NOEXCEPT;

STL_LOG_API std::tm localtime(const std::time_t &time_tt) STL_LOG_NOEXCEPT;

STL_LOG_API std::tm localtime() STL_LOG_NOEXCEPT;

STL_LOG_API std::tm gmtime(const std::time_t &time_tt) STL_LOG_NOEXCEPT;

STL_LOG_API std::tm gmtime() STL_LOG_NOEXCEPT;

// eol definition
#if !defined(STL_LOG_EOL)
#    ifdef _WIN32
#        define STL_LOG_EOL "\r\n"
#    else
#        define STL_LOG_EOL "\n"
#    endif
#endif

STL_LOG_CONSTEXPR static const char *default_eol = STL_LOG_EOL;

// folder separator
#if !defined(STL_LOG_FOLDER_SEPS)
#    ifdef _WIN32
#        define STL_LOG_FOLDER_SEPS "\\/"
#    else
#        define STL_LOG_FOLDER_SEPS "/"
#    endif
#endif

STL_LOG_CONSTEXPR static const char folder_seps[] = STL_LOG_FOLDER_SEPS;
STL_LOG_CONSTEXPR static const filename_t::value_type folder_seps_filename[] = STL_LOG_FILENAME_T(STL_LOG_FOLDER_SEPS);

// fopen_s on non windows for writing
STL_LOG_API bool fopen_s(FILE **fp, const filename_t &filename, const filename_t &mode);

// Remove filename. return 0 on success
STL_LOG_API int remove(const filename_t &filename) STL_LOG_NOEXCEPT;

// Remove file if exists. return 0 on success
// Note: Non atomic (might return failure to delete if concurrently deleted by other process/thread)
STL_LOG_API int remove_if_exists(const filename_t &filename) STL_LOG_NOEXCEPT;

STL_LOG_API int rename(const filename_t &filename1, const filename_t &filename2) STL_LOG_NOEXCEPT;

// Return if file exists.
STL_LOG_API bool path_exists(const filename_t &filename) STL_LOG_NOEXCEPT;

// Return file size according to open FILE* object
STL_LOG_API size_t filesize(FILE *f);

// Return utc offset in minutes or throw spdlog_ex on failure
STL_LOG_API int utc_minutes_offset(const std::tm &tm = details::os::localtime());

// Return current thread id as size_t
// It exists because the std::this_thread::get_id() is much slower(especially
// under VS 2013)
STL_LOG_API size_t _thread_id() STL_LOG_NOEXCEPT;

// Return current thread id as size_t (from thread local storage)
STL_LOG_API size_t thread_id() STL_LOG_NOEXCEPT;

// This is avoid msvc issue in sleep_for that happens if the clock changes.
// See https://github.com/gabime/issues/609
STL_LOG_API void sleep_for_millis(unsigned int milliseconds) STL_LOG_NOEXCEPT;

STL_LOG_API std::string filename_to_str(const filename_t &filename);

STL_LOG_API int pid() STL_LOG_NOEXCEPT;

// Determine if the terminal supports colors
// Source: https://github.com/agauniyal/rang/
STL_LOG_API bool is_color_terminal() STL_LOG_NOEXCEPT;

// Determine if the terminal attached
// Source: https://github.com/agauniyal/rang/
STL_LOG_API bool in_terminal(FILE *file) STL_LOG_NOEXCEPT;

#if (defined(STL_LOG_WCHAR_TO_UTF8_SUPPORT) || defined(STL_LOG_WCHAR_FILENAMES)) && defined(_WIN32)
STL_LOG_API void wstr_to_utf8buf(wstring_view_t wstr, memory_buf_t &target);

STL_LOG_API void utf8_to_wstrbuf(string_view_t str, wmemory_buf_t &target);
#endif

// Return directory name from given path or empty string
// "abc/file" => "abc"
// "abc/" => "abc"
// "abc" => ""
// "abc///" => "abc//"
STL_LOG_API filename_t dir_name(const filename_t &path);

// Create a dir from the given path.
// Return true if succeeded or if this dir already exists.
STL_LOG_API bool create_dir(const filename_t &path);

// non thread safe, cross platform getenv/getenv_s
// return empty string if field not found
STL_LOG_API std::string getenv(const char *field);

// Do fsync by FILE objectpointer.
// Return true on success.
STL_LOG_API bool fsync(FILE *fp);

} // namespace os
} // namespace details
} // namespace stl

#ifdef STL_LOG_HEADER_ONLY
#    include "os-inl.h"
#endif
