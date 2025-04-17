/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/11/8 13:01
 * @version: 1.0
 * @description: 
*/


#include "files/stl_memory_file.h"


namespace stl {


    bool MemoryFile::flush() {
        if (read_only_)return true;
#if defined(_WIN32)
        return FlushViewOfFile(buf_, size_);
#else
        return msync(buf_, size_, MS_ASYNC) == 0;
#endif
    }

#if defined(_WIN32)
#define MAP_FAILED NULL
#endif

    MemoryFile::MemoryFile(const char *path, bool read_only)
#if defined(_WIN32)
    : hFile_(NULL),
      hMapping_(NULL)
#else
            : fd_(-1)
#endif
            ,
              size_(0),
              buf_(nullptr) {
        this->read_only_ = read_only;
#if defined(_WIN32)
        if (not read_only){
            hFile_ = ::CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile_ == INVALID_HANDLE_VALUE) {
                std::runtime_error("");
            }

            size_ = ::GetFileSize(hFile_, NULL);

            hMapping_ = ::CreateFileMapping(hFile_, NULL, PAGE_READWRITE, 0, 0, NULL);

            if (hMapping_ == NULL) {
                cleanup();
                std::runtime_error("");
            }

            buf_ = static_cast<uint8_t *>(::MapViewOfFile(hMapping_, FILE_MAP_ALL_ACCESS, 0, 0, 0));
        } else{
            hFile_ = ::CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile_ == INVALID_HANDLE_VALUE) {
                std::runtime_error("");
            }

            size_ = ::GetFileSize(hFile_, NULL);

            hMapping_ = ::CreateFileMapping(hFile_, NULL, PAGE_READONLY, 0, 0, NULL);

            if (hMapping_ == NULL) {
                cleanup();
                std::runtime_error("");
            }

            buf_ = static_cast<uint8_t *>(::MapViewOfFile(hMapping_, FILE_MAP_READ, 0, 0, 0));

        }

#else

#ifdef SAFE_CALL
        if (read_only) {
            fd_ = sys_open(path, O_RDONLY, 0);
        } else {
            fd_ = sys_open(path, O_CREAT | O_RDWR, 0);
        }

#else

        if (read_only) {
            fd_ = open(path, O_RDONLY, 0);
        } else {
            fd_ = open(path, O_CREAT | O_RDWR, 0);
        }
#endif
        if (fd_ == -1) {
            std::runtime_error("");
        }
#ifdef SAFE_CALL
        struct kernel_stat sb;
        if (sys_fstat(fd_, &sb) == -1) {
#else
        struct stat sb;
        if (fstat(fd_, &sb) == -1) {
#endif
            cleanup();
            std::runtime_error("");
        }
        size_ = sb.st_size;
#ifdef SAFE_CALL
        if (read_only){
            buf_ = static_cast<uint8_t *>(sys_mmap(NULL, size_, PROT_READ, MAP_PRIVATE, fd_, 0));
        } else{
            buf_ = static_cast<uint8_t *>(sys_mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
        }

#else
        if (read_only) {
            buf_ = static_cast<uint8_t *>(mmap(NULL, size_, PROT_READ, MAP_PRIVATE, fd_, 0));
        } else {
            buf_ = static_cast<uint8_t *>(mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
        }

#endif
#endif

        if (buf_ == MAP_FAILED) {
            cleanup();
            //exit(GetLastError());
            std::runtime_error("");
        }
    }

    MemoryFile::~MemoryFile() {
        flush();
        cleanup();
    }

    bool MemoryFile::is_open() const { return buf_ != MAP_FAILED; }

    bool MemoryFile::isOpen() const {
        return is_open();
    }

    size_t MemoryFile::size() const { return size_; }

    uint8_t *MemoryFile::data() const { return buf_; }

    void MemoryFile::sync() const {
#if defined(_WIN32)

#else

#endif

    }

    void MemoryFile::cleanup() {
#if defined(_WIN32)
        if (buf_) {
            ::UnmapViewOfFile(buf_);
            buf_ = MAP_FAILED;
        }

        if (hMapping_) {
            ::CloseHandle(hMapping_);
            hMapping_ = NULL;
        }

        if (hFile_ != INVALID_HANDLE_VALUE) {
            ::CloseHandle(hFile_);
            hFile_ = INVALID_HANDLE_VALUE;
        }
#else
        if (buf_ != MAP_FAILED) {
#ifdef SAFE_CALL
            sys_munmap(buf_, size_);
#else
            munmap(buf_, size_);
#endif
            buf_ = nullptr;
        }

        if (fd_ != -1) {
#ifdef SAFE_CALL
            sys_close(fd_);
#else
            close(fd_);
#endif
            fd_ = -1;
        }
#endif
        size_ = 0;
    }
}