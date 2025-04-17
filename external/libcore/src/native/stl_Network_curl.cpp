/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace stl
{

struct CURLSymbols
{
    CURL* (*curl_easy_init) (void);
    CURLcode (*curl_easy_setopt) (CURL *curl, CURLoption option, ...);
    void (*curl_easy_cleanup) (CURL *curl);
    CURLcode (*curl_easy_getinfo) (CURL *curl, CURLINFO info, ...);
    CURLMcode (*curl_multi_add_handle) (CURLM *multi_handle, CURL *curl_handle);
    CURLMcode (*curl_multi_cleanup) (CURLM *multi_handle);
    CURLMcode (*curl_multi_fdset) (CURLM *multi_handle, fd_set *read_fd_set, fd_set *write_fd_set, fd_set *exc_fd_set, int *max_fd);
    CURLMsg* (*curl_multi_info_read) (CURLM *multi_handle, int *msgs_in_queue);
    CURLM* (*curl_multi_init) (void);
    CURLMcode (*curl_multi_perform) (CURLM *multi_handle, int *running_handles);
    CURLMcode (*curl_multi_remove_handle) (CURLM *multi_handle, CURL *curl_handle);
    CURLMcode (*curl_multi_timeout) (CURLM *multi_handle, long *milliseconds);
    struct curl_slist* (*curl_slist_append) (struct curl_slist *, const char *);
    void (*curl_slist_free_all) (struct curl_slist *);
    curl_version_info_data* (*curl_version_info) (CURLversion);

    static std::unique_ptr<CURLSymbols> create()
    {
        std::unique_ptr<CURLSymbols> symbols (new CURLSymbols);

       #if STL_LOAD_CURL_SYMBOLS_LAZILY
        const ScopedLock sl (getLibcurlLock());
        #define STL_INIT_CURL_SYMBOL(name)  if (! symbols->loadSymbol (symbols->name, #name)) return nullptr;
       #else
        #define STL_INIT_CURL_SYMBOL(name)  symbols->name = ::name;
       #endif

        STL_INIT_CURL_SYMBOL (curl_easy_init)
        STL_INIT_CURL_SYMBOL (curl_easy_setopt)
        STL_INIT_CURL_SYMBOL (curl_easy_cleanup)
        STL_INIT_CURL_SYMBOL (curl_easy_getinfo)
        STL_INIT_CURL_SYMBOL (curl_multi_add_handle)
        STL_INIT_CURL_SYMBOL (curl_multi_cleanup)
        STL_INIT_CURL_SYMBOL (curl_multi_fdset)
        STL_INIT_CURL_SYMBOL (curl_multi_info_read)
        STL_INIT_CURL_SYMBOL (curl_multi_init)
        STL_INIT_CURL_SYMBOL (curl_multi_perform)
        STL_INIT_CURL_SYMBOL (curl_multi_remove_handle)
        STL_INIT_CURL_SYMBOL (curl_multi_timeout)
        STL_INIT_CURL_SYMBOL (curl_slist_append)
        STL_INIT_CURL_SYMBOL (curl_slist_free_all)
        STL_INIT_CURL_SYMBOL (curl_version_info)

        return symbols;
    }

    // liburl's curl_multi_init calls curl_global_init which is not thread safe
    // so we need to get a lock during calls to curl_multi_init and curl_multi_cleanup
    static CriticalSection& getLibcurlLock() noexcept
    {
        static CriticalSection cs;
        return cs;
    }

private:
    CURLSymbols() = default;

   #if STL_LOAD_CURL_SYMBOLS_LAZILY
    static DynamicLibrary& getLibcurl()
    {
        const ScopedLock sl (getLibcurlLock());
        static DynamicLibrary libcurl;

        if (libcurl.getNativeHandle() == nullptr)
            for (auto libName : { "libcurl.so",
                                  "libcurl.so.4", "libcurl.so.3",
                                  "libcurl-gnutls.so.4", "libcurl-gnutls.so.3" })
                if (libcurl.open (libName))
                    break;

        return libcurl;
    }

    template <typename FuncPtr>
    bool loadSymbol (FuncPtr& dst, const char* name)
    {
        dst = reinterpret_cast<FuncPtr> (getLibcurl().getFunction (name));
        return (dst != nullptr);
    }
   #endif
};





} // namespace stl
