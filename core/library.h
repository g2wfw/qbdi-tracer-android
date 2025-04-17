/*
The MIT License (MIT)

Copyright (c) 2025 g2wfw

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#ifndef REVERSE_TOOLS_LIBRARY_H
#define REVERSE_TOOLS_LIBRARY_H

#include <string>
#include "common.h"
#include "range.h"
namespace stl {

    typedef void(*export_callback_t)(const char *symbol_name, uintptr_t address, void *user_data);
    class Library {
    public:
        static std::unique_ptr<Library> find_library(const std::string &library_name);

        static std::unique_ptr<Library> find_library(uintptr_t address);

        void enumerate_exports(export_callback_t callback, void *user_data);

        Library(std::string library_name, void *so_info);

        explicit Library(const std::string &library_name);

        std::string get_symbol_by_address(uintptr_t address);

        std::string get_library_name();

        Range<uintptr_t> get_library_range();

        ~Library();

        uintptr_t get_load_bias();

        size_t get_library_size();

        void *get_init_proc();

        size_t get_init_array_count();

        void set_init_array_count(size_t count);

        void set_init_proc(void *ptr);

        void *get_init_array();

        bool is_ctor_called();

        void *get_so_info();

        void *find_symbol(const std::string symbol_name);


    private:
        void *handler = nullptr;
        void *so_info = nullptr;
        std::string library_name;
        DISALLOW_COPY_AND_ASSIGN(Library);

    };

} // rt

#endif //REVERSE_TOOLS_LIBRARY_H
