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

#ifndef REVERSE_TOOLS_LINKER_H
#define REVERSE_TOOLS_LINKER_H

#include <string>
#include "common.h"

namespace stl{
    typedef enum {
        DLOPEN,
        DLOPEN_POST,
        INIT_PROC,
        INIT_PROC_POST,
        INIT_ARRAY,
        INIT_ARRAY_POST,
    } linker_type_t;

    typedef void (*linker_callback_t)(const char *path, linker_type_t type, void *so_info, void *user_data);

    class Linker {
    public:
        static Linker *getInstance();

        bool add_library_monitor(const char *library_name, linker_type_t type, linker_callback_t callback,
                                 void *user_data = nullptr);

        void *get_so_info(const char *library_name);

        ~Linker();

    private:
        Linker();

        void init_dlopen_hook();

        void init_linker_hook();

    private:
        void *handler = nullptr;
        DISALLOW_COPY_AND_ASSIGN(Linker);
    };

}


#endif //REVERSE_TOOLS_LINKER_H
