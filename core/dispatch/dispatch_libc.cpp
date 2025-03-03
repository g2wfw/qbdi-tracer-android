//
//
//
#include <spdlog/fmt/fmt.h>
#include <fcntl.h>
#include <unistd.h>
#include "dispatch_libc.h"
#include "../memory_manager.h"

static std::unordered_map<GumAddress, std::pair<const char*, std::function<void(
                                                    inst_trace_info_t* info)>>> libc_handlers;

DispatchLibc* DispatchLibc::get_instance() {
    static DispatchLibc dispatchLibc;
    return &dispatchLibc;
}


DispatchLibc::DispatchLibc() {
    auto module = gum_process_get_libc_module();
    auto range = gum_module_get_range(module);
    module_range.base = range->base_address;
    module_range.end = range->size + range->base_address;
    //string.h
    REGISTER_HANDLER(libc_handlers, strlen, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     //size_t strlen(const char* __s)
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, __strlen_chk, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     //size_t __strlen_chk(const char* __s, size_t __len)
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("len={}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //memccpy
    REGISTER_HANDLER(libc_handlers, memccpy, {
                     //void *memccpy(void *__dest, const void *__src, int __c, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("dest={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("src={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("c={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg3));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    REGISTER_HANDLER(libc_handlers, memchr, {
                     //void *memchr(const void *__s, int __c, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("c={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    REGISTER_HANDLER(libc_handlers, memcpy, {
                     //void *memcpy(void *__dest, const void *__src, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("dest={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("src={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    //memcmp
    REGISTER_HANDLER(libc_handlers, memcmp, {
                     //int memcmp(const void *__s1, const void *__s2, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s1={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("s2={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, memmove, {
                     //void *memmove(void *__dest, const void *__src, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("dest={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("src={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    REGISTER_HANDLER(libc_handlers, memset, {
                     //void *memset(void *__s, int __c, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("c={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    //memmem
    REGISTER_HANDLER(libc_handlers, memmem, {
                     //void *memmem(const void *__haystack, size_t __haystacklen, const void *__needle, size_t __needlelen)
                     trace_info->fun_call->args.push_back(
                         fmt::format("haystack={:#x}", get_arg_register_value(&trace_info->pre_status, 0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("haystacklen={:#x}",
                             get_arg_register_value(&trace_info->pre_status, 1)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("needle={:#x}", get_arg_register_value(&trace_info->pre_status, 2)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("needlelen={:#x}", get_arg_register_value(&trace_info->pre_status, 3)));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    //strchr
    REGISTER_HANDLER(libc_handlers, strchr, {
                     //char *strchr(const char *__s, int __c)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("c={:#x}", arg1));
                     trace_info->fun_call->ret_type = kString;
                     });
    //__strchr_chk
    REGISTER_HANDLER(libc_handlers, __strchr_chk, {
                     //char *__strchr_chk(const char *__s, int __c, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("c={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strrchr
    REGISTER_HANDLER(libc_handlers, strrchr, {
                     //char *strrchr(const char *__s, int __c)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("c={:#x}", arg1));
                     trace_info->fun_call->ret_type = kString;
                     });
    //__strrchr_chk
    REGISTER_HANDLER(libc_handlers, __strrchr_chk, {
                     //char *__strrchr_chk(const char *__s, int __c, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("c={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strcmp
    REGISTER_HANDLER(libc_handlers, strcmp, {
                     //int strcmp(const char *__s1, const char *__s2)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s1={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("s2={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //stpcpy
    REGISTER_HANDLER(libc_handlers, stpcpy, {
                     //char *stpcpy(char *__dest, const char *__src)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("dest={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("src={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strcpy
    REGISTER_HANDLER(libc_handlers, strcpy, {
                     //char *strcpy(char *__dest, const char *__src)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("dest={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("src={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strcat
    REGISTER_HANDLER(libc_handlers, strcat, {
                     //char *strcat(char *__dest, const char *__src)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("dest={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("src={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strdup
    REGISTER_HANDLER(libc_handlers, strdup, {
                     //char *strdup(const char *__s)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strstr
    REGISTER_HANDLER(libc_handlers, strstr, {
                     //char *strstr(const char *__haystack, const char *__needle)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("haystack={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("needle={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strcasestr
    REGISTER_HANDLER(libc_handlers, strcasestr, {
                     //char *strcasestr(const char *__haystack, const char *__needle)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("haystack={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("needle={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strcasestr
    REGISTER_HANDLER(libc_handlers, strncasestr, {
                     //char *strncasestr(const char *__haystack, const char *__needle, size_t __len)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("haystack={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("needle={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strtok
    REGISTER_HANDLER(libc_handlers, strtok, {
                     //char *strtok(char *__s, const char *__delim)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("delim={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strtok_r
    REGISTER_HANDLER(libc_handlers, strtok_r, {
                     //char *strtok_r(char *__s, const char *__delim, char **__lasts)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("delim={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("lasts={:#x}", arg2));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strerror
    REGISTER_HANDLER(libc_handlers, strerror, {
                     //char *strerror(int __errnum)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("errnum={:#x}", arg0));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strerror_r
    REGISTER_HANDLER(libc_handlers, strerror_r, {
                     //int strerror_r(int __errnum, char *__buf, size_t __buflen)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("errnum={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("buflen={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strnlen
    REGISTER_HANDLER(libc_handlers, strnlen, {
                     //size_t strnlen(const char *__s, size_t __maxlen)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("maxlen={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strncat
    REGISTER_HANDLER(libc_handlers, strncat, {
                     //char *strncat(char *__dest, const char *__src, size_t __maxlen)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("dest={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("src={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("maxlen={:#x}", arg2));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strndup
    REGISTER_HANDLER(libc_handlers, strndup, {
                     //char *strndup(const char *__s, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg1));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strncmp
    REGISTER_HANDLER(libc_handlers, strncmp, {
                     //int strncmp(const char *__s1, const char *__s2, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s1={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("s2={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //stpncpy
    REGISTER_HANDLER(libc_handlers, stpncpy, {
                     //char *stpncpy(char *__dest, const char *__src, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("dest={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("src={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strncpy
    REGISTER_HANDLER(libc_handlers, strncpy, {
                     //char *strncpy(char *__dest, const char *__src, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("dest={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("src={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strlcat
    REGISTER_HANDLER(libc_handlers, strlcat, {
                     //size_t strlcat(char *__dest, const char *__src, size_t __size)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("dest={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("src={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strlcpy
    REGISTER_HANDLER(libc_handlers, strlcpy, {
                     //size_t strlcpy(char *__dest, const char *__src, size_t __size)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("dest={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("src={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strcspn
    REGISTER_HANDLER(libc_handlers, strcspn, {
                     //size_t strcspn(const char *__s, const char *__reject)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("reject={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strpbrk
    REGISTER_HANDLER(libc_handlers, strpbrk, {
                     //char *strpbrk(const char *__s, const char *__accept)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("accept={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strsep
    REGISTER_HANDLER(libc_handlers, strsep, {
                     //char *strsep(char **__stringp, const char *__delim)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("stringp={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("delim={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strspn
    REGISTER_HANDLER(libc_handlers, strspn, {
                     //size_t strspn(const char *__s, const char *__accept)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("accept={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strsignal
    REGISTER_HANDLER(libc_handlers, strsignal, {
                     //const char *strsignal(int __sig)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg0));
                     trace_info->fun_call->ret_type = kString;
                     });
    //strcoll
    REGISTER_HANDLER(libc_handlers, strcoll, {
                     //int strcoll(const char *__s1, const char *__s2)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s1={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("s2={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strxfrm
    REGISTER_HANDLER(libc_handlers, strxfrm, {
                     //size_t strxfrm(char *__dest, const char *__src, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("dest={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("src={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strcoll_l
    REGISTER_HANDLER(libc_handlers, strcoll_l, {
                     //size_t strcoll_l(const char *__s1, const char *__s2, locale_t __l)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s1={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("s2={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("l={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strcasecmp
    REGISTER_HANDLER(libc_handlers, strcasecmp, {
                     //int strcasecmp(const char *__s1, const char *__s2)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("s1={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("s2={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strncasecmp
    REGISTER_HANDLER(libc_handlers, strncasecmp, {
                     //int strncasecmp(const char *__s1, const char *__s2, size_t __n)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s1={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("s2={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //memory function handler
    REGISTER_HANDLER(libc_handlers, malloc, {
                     //void *malloc(size_t __size)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg0));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    REGISTER_HANDLER(libc_handlers, calloc, {
                     //void *calloc(size_t __nmemb, size_t __size)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("nmemb={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, realloc, {
                     //void *realloc(void *__ptr, size_t __size)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("ptr={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    //free
    REGISTER_HANDLER(libc_handlers, free, {
                     //void free(void *__ptr)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("ptr={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    REGISTER_HANDLER(libc_handlers, memalign, {
                     //void *memalign(size_t __alignment, size_t __size)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("alignment={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    //stdlib.h fun  handler
    //abort
    REGISTER_HANDLER(libc_handlers, abort, {
                     //void abort(void)
                     trace_info->fun_call->ret_type = kVoid;
                     });
    REGISTER_HANDLER(libc_handlers, exit, {
                     //void exit(int __status)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    REGISTER_HANDLER(libc_handlers, _Exit, {
                     //void _Exit(int __status)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    //atexit
    REGISTER_HANDLER(libc_handlers, atexit, {
                     //int atexit(void (*__func)(void))
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("func={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, at_quick_exit, {
                     //int at_quick_exit(void (*__func)(void))
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("func={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //quick_exit
    REGISTER_HANDLER(libc_handlers, quick_exit, {
                     //void quick_exit(int __status)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    //getenv
    REGISTER_HANDLER(libc_handlers, getenv, {
                     //char *getenv(const char *__name)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("name={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kString;
                     });
    REGISTER_HANDLER(libc_handlers, setenv, {
                     //int setenv(const char *__name, const char *__value, int __replace)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("name={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(
                         fmt::format("value={}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("replace={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //putenv
    REGISTER_HANDLER(libc_handlers, putenv, {
                     //int putenv(char *__string)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("string={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //unsetenv
    REGISTER_HANDLER(libc_handlers, unsetenv, {
                     //int unsetenv(const char *__name)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("name={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //clearenv
    REGISTER_HANDLER(libc_handlers, clearenv, {
                     //int clearenv(void)
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //mkdtemp
    REGISTER_HANDLER(libc_handlers, mkdtemp, {
                     //char *mkdtemp(char *__template)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("template={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //mktemp
    REGISTER_HANDLER(libc_handlers, mktemp, {
                     //char *mktemp(char *__template)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("template={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kString;
                     });
    //mkstemp64
    REGISTER_HANDLER(libc_handlers, mkstemp64, {
                     //int mkstemp64(char *__template)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("template={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, mkstemp, {
                     //int mkstemp(char *__template)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("template={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //mkstemps
    REGISTER_HANDLER(libc_handlers, mkstemps, {
                     //int mkstemps(char *__template, int __suffixlen)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("template={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("suffixlen={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strtol
    REGISTER_HANDLER(libc_handlers, strtol, {
                     //long int strtol(const char *__restrict __nptr, char **__restrict __endptr, int __base)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("nptr={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("endptr={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("base={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strtoll
    REGISTER_HANDLER(libc_handlers, strtoll, {
                     //long long strtoll(const char* __s, char** __end_ptr, int __base);
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("end_ptr={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("base={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strtoul
    REGISTER_HANDLER(libc_handlers, strtoul, {
                     //unsigned long int strtoul(const char *__restrict __nptr, char **__restrict __endptr, int __base)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("nptr={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("endptr={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("base={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, strtoull, {
                     //unsigned long int strtoul(const char *__restrict __nptr, char **__restrict __endptr, int __base)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("nptr={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("endptr={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("base={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //posix_memalign
    REGISTER_HANDLER(libc_handlers, posix_memalign, {
                     //int posix_memalign(void **__memptr, size_t __alignment, size_t __size)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("memptr={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("alignment={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //strtod
    REGISTER_HANDLER(libc_handlers, strtod, {
                     //double strtod(const char *__restrict __nptr, char **__restrict __endptr)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("nptr={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("endptr={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, strtof, {
                     //float strtof(const char *__restrict __nptr, char **__restrict __endptr)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("nptr={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("endptr={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, strtold, {
                     //long double strtold(const char *__restrict __nptr, char **__restrict __endptr)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("nptr={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("endptr={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //atoi
    REGISTER_HANDLER(libc_handlers, atoi, {
                     //int atoi(const char *__nptr)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("nptr={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //atol
    REGISTER_HANDLER(libc_handlers, atol, {
                     //long int atol(const char *__nptr)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("nptr={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, atoll, {
                     //long long int atoll(const char *__nptr)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("nptr={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //realpath
    REGISTER_HANDLER(libc_handlers, realpath, {
                     //char *realpath(const char *__restrict __name, char *__restrict __resolved)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("name={}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("resolved={:#x}", arg1));
                     trace_info->fun_call->ret_type = kString;
                     });
    //bsearch
    REGISTER_HANDLER(libc_handlers, bsearch, {
                     //void *bsearch(const void *__key, const void *__base, size_t __nmemb, size_t __size, int (*__compar)(const void *, const void *))
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     auto arg4 = get_arg_register_value(&trace_info->pre_status, 4);

                     trace_info->fun_call->args.push_back(fmt::format("key={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("base={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("nmemb={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
                     trace_info->fun_call->args.push_back(fmt::format("compar={:#x}", arg4));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    REGISTER_HANDLER(libc_handlers, qsort, {
                     //void qsort(void *__base, size_t __nmemb, size_t __size, int (*__compar)(const void *, const void *))
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);

                     trace_info->fun_call->args.push_back(fmt::format("base={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("nmemb={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("compar={:#x}", arg3));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    //arc4random
    REGISTER_HANDLER(libc_handlers, arc4random, {
                     //unsigned int arc4random(void)
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //arc4random_uniform
    REGISTER_HANDLER(libc_handlers, arc4random_uniform, {
                     //unsigned int arc4random_uniform(unsigned int __upper_bound)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("upper_bound={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //arc4random_buf
    REGISTER_HANDLER(libc_handlers, arc4random_buf, {
                     //void arc4random_buf(void *__buf, size_t __nbytes)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("nbytes={:#x}", arg1));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    //rand_r
    REGISTER_HANDLER(libc_handlers, rand_r, {
                     //int rand_r(unsigned int *__seed)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("seed={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //drand48
    REGISTER_HANDLER(libc_handlers, drand48, {
                     //double drand48(void)
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //erand48
    REGISTER_HANDLER(libc_handlers, erand48, {
                     //double erand48(unsigned short int __xsubi[3])
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("xsubi={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //jrand48
    REGISTER_HANDLER(libc_handlers, jrand48, {
                     //long int jrand48(unsigned short int __xsubi[3])
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("xsubi={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //lcong48
    REGISTER_HANDLER(libc_handlers, lcong48, {
                     //void lcong48(unsigned short int __param[7])
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("param={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    //lrand48
    REGISTER_HANDLER(libc_handlers, lrand48, {
                     //long int lrand48(void)
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //mrand48
    REGISTER_HANDLER(libc_handlers, mrand48, {
                     //long int mrand48(void)
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //nrand48
    REGISTER_HANDLER(libc_handlers, nrand48, {
                     //long int nrand48(unsigned short int __xsubi[3])
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("xsubi={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //srand48
    REGISTER_HANDLER(libc_handlers, srand48, {
                     //void srand48(long int __seedval)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("seedval={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    //getprogname
    REGISTER_HANDLER(libc_handlers, getprogname, {
                     //const char *getprogname(void)
                     trace_info->fun_call->ret_type = kString;
                     });
    REGISTER_HANDLER(libc_handlers, setprogname, {
                     //void setprogname(const char *__name)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("name={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    //rand
    REGISTER_HANDLER(libc_handlers, rand, {
                     //int rand(void)
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //srand
    REGISTER_HANDLER(libc_handlers, srand, {
                     //void srand(unsigned int __seed)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("seed={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    //random
    REGISTER_HANDLER(libc_handlers, random, {
                     //long int random(void)
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //srandom
    REGISTER_HANDLER(libc_handlers, srandom, {
                     //void srandom(unsigned int __seed)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("seed={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    //io func
    //creat
    REGISTER_HANDLER(libc_handlers, creat, {
                     //int creat(const char *__path, mode_t __mode)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={:}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //creat64
    REGISTER_HANDLER(libc_handlers, creat64, {
                     //int creat64(const char *__path, mode_t __mode)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={:}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //openat
    REGISTER_HANDLER(libc_handlers, openat, {
                     //int openat(int __fd, const char *__path, int __oflag, mode_t __mode)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={:}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("oflag={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //openat64
    REGISTER_HANDLER(libc_handlers, openat64, {
                     //int openat64(int __fd, const char *__path, int __oflag, mode_t __mode)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={:}", read_string_from_address(arg1)));
                     trace_info->fun_call->args.push_back(fmt::format("oflag={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //open
    REGISTER_HANDLER(libc_handlers, open, {
                     //int open(const char *__path, int __oflag, mode_t __mode)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={:}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("oflag={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //open64
    REGISTER_HANDLER(libc_handlers, open64, {
                     //int open64(const char *__path, int __oflag, mode_t __mode)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={:}", read_string_from_address(arg0)));
                     trace_info->fun_call->args.push_back(fmt::format("oflag={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //fallocate
    REGISTER_HANDLER(libc_handlers, fallocate, {
                     //int fallocate(int __fd, int __mode, off_t __offset, off_t __len)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //fallocate64
    REGISTER_HANDLER(libc_handlers, fallocate64, {
                     //int fallocate64(int __fd, int __mode, off64_t __offset, off64_t __len)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //posix_fadvise
    REGISTER_HANDLER(libc_handlers, posix_fadvise, {
                     //int posix_fadvise(int __fd, off_t __offset, off_t __len, int __advise)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("advise={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //posix_fadvise64
    REGISTER_HANDLER(libc_handlers, posix_fadvise64, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("advise={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //posix_fallocate
    REGISTER_HANDLER(libc_handlers, posix_fallocate, {
                     //int posix_fallocate(int __fd, off_t __offset, off_t __len)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    REGISTER_HANDLER(libc_handlers, posix_fallocate64, {
                     //int posix_fallocate(int __fd, off_t __offset, off_t __len)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //readahead
    REGISTER_HANDLER(libc_handlers, readahead, {
                     //int readahead(int __fd, off_t __offset, size_t __count)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //unistd.h
    REGISTER_HANDLER(libc_handlers, _exit, {
                     //_exit(int __status)
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //fork
    REGISTER_HANDLER(libc_handlers, fork, {
                     //pid_t fork(void)
                     trace_info->fun_call->ret_type = kNumber;
                     });
    /*
     pid_t  vfork(void);
     pid_t  getpid(void);
     pid_t  gettid(void) __attribute_const__;
     pid_t  getpgid(pid_t __pid);
     int    setpgid(pid_t __pid, pid_t __pgid);
     pid_t  getppid(void);
     pid_t  getpgrp(void);
     int    setpgrp(void);
     */
    REGISTER_HANDLER(libc_handlers, getpid, {

                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, gettid, {

                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, setpgrp, {
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, getpgrp, {

                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, getppid, {

                     trace_info->fun_call->ret_type = kNumber;
                     });
    //getpgid
    REGISTER_HANDLER(libc_handlers, getpgid, {
                     // pid_t  getpgid(pid_t __pid);
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //setpgid
    REGISTER_HANDLER(libc_handlers, setpgid, {
                     //int setpgid(pid_t __pid, pid_t __pgid);
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("pgid={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    /*
        int access(const char* __path, int __mode);*/
    REGISTER_HANDLER(libc_handlers, access, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("path={}", arg0));
                     trace_info->fun_call->args.push_back(
                         fmt::format("mode={:#x}", get_arg_register_value(&trace_info->pre_status, 1)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    //int faccessat(int __dirfd, const char* __path, int __mode, int __flags)
    REGISTER_HANDLER(libc_handlers, faccessat, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg1)));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    // int link(const char* __old_path, const char* __new_path)
    REGISTER_HANDLER(libc_handlers, link, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("old_path={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("new_path={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int linkat(int __old_dir_fd, const char* __old_path, int __new_dir_fd, const char* __new_path, int __flags)
    REGISTER_HANDLER(libc_handlers, linkat, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("old_dir_fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("old_path={}", read_string_from_address(arg1)));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("new_dir_fd={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(
                         fmt::format("new_path={}", read_string_from_address(arg3)));
                     auto arg4 = get_arg_register_value(&trace_info->pre_status, 4);
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int unlink(const char* __path)
    REGISTER_HANDLER(libc_handlers, unlink, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int unlinkat(int __dirfd, const char* __path, int __flags)
    REGISTER_HANDLER(libc_handlers, unlinkat, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg1)));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int chdir(const char* __path)
    REGISTER_HANDLER(libc_handlers, chdir, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int fchdir(int __fd)
    REGISTER_HANDLER(libc_handlers, fchdir, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int rmdir(const char* __path)
    REGISTER_HANDLER(libc_handlers, rmdir, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pipe(int __fds[2])
    REGISTER_HANDLER(libc_handlers, pipe, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fds={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pipe2(int __fds[2], int __flags)
    REGISTER_HANDLER(libc_handlers, pipe2, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fds={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int chroot(const char* __path)
    REGISTER_HANDLER(libc_handlers, chroot, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int symlink(const char* __old_path, const char* __new_path)
    REGISTER_HANDLER(libc_handlers, symlink, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("old_path={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("new_path={}", read_string_from_address(arg1)));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int symlinkat(const char* __old_path, int __new_dir_fd, const char* __new_path)
    REGISTER_HANDLER(libc_handlers, symlinkat, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("old_path={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("new_dir_fd={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(
                         fmt::format("new_path={}", read_string_from_address(arg2)));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // ssize_t readlink(const char* __path, char* __buf, size_t __buf_size)
    REGISTER_HANDLER(libc_handlers, readlink, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("buf_size={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // ssize_t readlinkat(int __dir_fd, const char* __path, char* __buf, size_t __buf_size)
    REGISTER_HANDLER(libc_handlers, readlinkat, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("dir_fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg1)));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("buf_size={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int chown(const char* __path, uid_t __owner, gid_t __group)
    REGISTER_HANDLER(libc_handlers, chown, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int fchown(int __fd, uid_t __owner, gid_t __group)
    REGISTER_HANDLER(libc_handlers, fchown, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int fchownat(int __dir_fd, const char* __path, uid_t __owner, gid_t __group, int __flags)
    REGISTER_HANDLER(libc_handlers, fchownat, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("dir_fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg1)));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg3));
                     auto arg4 = get_arg_register_value(&trace_info->pre_status, 4);
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int lchown(const char* __path, uid_t __owner, gid_t __group)
    REGISTER_HANDLER(libc_handlers, lchown, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // char* getcwd(char* __buf, size_t __size)
    REGISTER_HANDLER(libc_handlers, getcwd, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kString;
                     });

    // void sync(void)
    REGISTER_HANDLER(libc_handlers, sync, {
                     trace_info->fun_call->ret_type = kVoid;
                     });

    // int syncfs(int __fd)
    REGISTER_HANDLER(libc_handlers, syncfs, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int close(int __fd)
    REGISTER_HANDLER(libc_handlers, close, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // ssize_t read(int __fd, void* __buf, size_t __count)
    REGISTER_HANDLER(libc_handlers, read, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // ssize_t write(int __fd, const void* __buf, size_t __count)
    REGISTER_HANDLER(libc_handlers, write, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int dup(int __old_fd)
    REGISTER_HANDLER(libc_handlers, dup, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("old_fd={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int dup2(int __old_fd, int __new_fd)
    REGISTER_HANDLER(libc_handlers, dup2, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("old_fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("new_fd={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int dup3(int __old_fd, int __new_fd, int __flags)
    REGISTER_HANDLER(libc_handlers, dup3, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("old_fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("new_fd={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int fsync(int __fd)
    REGISTER_HANDLER(libc_handlers, fsync, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int fdatasync(int __fd)
    REGISTER_HANDLER(libc_handlers, fdatasync, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // off_t lseek(int __fd, off_t __offset, int __whence)
    REGISTER_HANDLER(libc_handlers, lseek, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("whence={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // off64_t lseek64(int __fd, off64_t __offset, int __whence)
    REGISTER_HANDLER(libc_handlers, lseek64, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("whence={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int truncate(const char* __path, off_t __length)
    REGISTER_HANDLER(libc_handlers, truncate, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int truncate64(const char* __path, off64_t __length)
    REGISTER_HANDLER(libc_handlers, truncate64, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // ssize_t pread(int __fd, void* __buf, size_t __count, off_t __offset)
    REGISTER_HANDLER(libc_handlers, pread, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // ssize_t pread64(int __fd, void* __buf, size_t __count, off64_t __offset)
    REGISTER_HANDLER(libc_handlers, pread64, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // ssize_t pwrite(int __fd, const void* __buf, size_t __count, off_t __offset)
    REGISTER_HANDLER(libc_handlers, pwrite, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // ssize_t pwrite64(int __fd, const void* __buf, size_t __count, off64_t __offset)
    REGISTER_HANDLER(libc_handlers, pwrite64, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int ftruncate(int __fd, off_t __length)
    REGISTER_HANDLER(libc_handlers, ftruncate, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int ftruncate64(int __fd, off64_t __length)
    REGISTER_HANDLER(libc_handlers, ftruncate64, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pause(void)
    REGISTER_HANDLER(libc_handlers, pause, {
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // unsigned int alarm(unsigned int __seconds)
    REGISTER_HANDLER(libc_handlers, alarm, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("seconds={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // unsigned int sleep(unsigned int __seconds)
    REGISTER_HANDLER(libc_handlers, sleep, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("seconds={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int usleep(useconds_t __microseconds)
    REGISTER_HANDLER(libc_handlers, usleep, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("microseconds={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int gethostname(char* __buf, size_t __buf_size)
    REGISTER_HANDLER(libc_handlers, gethostname, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf_size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int sethostname(const char* __name, size_t __n)
    REGISTER_HANDLER(libc_handlers, sethostname, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("name={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int brk(void* __addr)
    REGISTER_HANDLER(libc_handlers, brk, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // void* sbrk(ptrdiff_t __increment)
    REGISTER_HANDLER(libc_handlers, sbrk, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("increment={:#x}", arg0));
                     trace_info->fun_call->ret_type = kPointer;
                     });

    // int isatty(int __fd)
    REGISTER_HANDLER(libc_handlers, isatty, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // char* ttyname(int __fd)
    REGISTER_HANDLER(libc_handlers, ttyname, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     trace_info->fun_call->ret_type = kString;
                     });

    // int ttyname_r(int __fd, char* __buf, size_t __buf_size)
    REGISTER_HANDLER(libc_handlers, ttyname_r, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("buf_size={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int acct(const char* __path)
    REGISTER_HANDLER(libc_handlers, acct, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", read_string_from_address(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int getpagesize(void)
    REGISTER_HANDLER(libc_handlers, getpagesize, {
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // long syscall(long __number, ...)
    REGISTER_HANDLER(libc_handlers, syscall, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("number={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int daemon(int __no_chdir, int __no_close)
    REGISTER_HANDLER(libc_handlers, daemon, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("no_chdir={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("no_close={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // time_t time(time_t* __t)
    REGISTER_HANDLER(libc_handlers, time, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("t={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int nanosleep(const struct timespec* __request, struct timespec* __remainder)
    REGISTER_HANDLER(libc_handlers, nanosleep, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("request={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("remainder={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // char* asctime(const struct tm* __tm)
    REGISTER_HANDLER(libc_handlers, asctime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg0));
                     trace_info->fun_call->ret_type = kString;
                     });

    // char* asctime_r(const struct tm* __tm, char* __buf)
    REGISTER_HANDLER(libc_handlers, asctime_r, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     trace_info->fun_call->ret_type = kString;
                     });

    // double difftime(time_t __lhs, time_t __rhs)
    REGISTER_HANDLER(libc_handlers, difftime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("lhs={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("rhs={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // time_t mktime(struct tm* __tm)
    REGISTER_HANDLER(libc_handlers, mktime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // struct tm* localtime(const time_t* __t)
    REGISTER_HANDLER(libc_handlers, localtime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("t={:#x}", arg0));
                     trace_info->fun_call->ret_type = kPointer;
                     });

    // struct tm* localtime_r(const time_t* __t, struct tm* __tm)
    REGISTER_HANDLER(libc_handlers, localtime_r, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("t={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg1));
                     trace_info->fun_call->ret_type = kPointer;
                     });

    // struct tm* gmtime(const time_t* __t)
    REGISTER_HANDLER(libc_handlers, gmtime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("t={:#x}", arg0));
                     trace_info->fun_call->ret_type = kPointer;
                     });

    // struct tm* gmtime_r(const time_t* __t, struct tm* __tm)
    REGISTER_HANDLER(libc_handlers, gmtime_r, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("t={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg1));
                     trace_info->fun_call->ret_type = kPointer;
                     });

    // char* strptime(const char* __s, const char* __fmt, struct tm* __tm)
    REGISTER_HANDLER(libc_handlers, strptime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("s={}", read_string_from_address(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("fmt={}", read_string_from_address(arg1)));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg2));
                     trace_info->fun_call->ret_type = kString;
                     });

    // size_t strftime(char* __buf, size_t __n, const char* __fmt, const struct tm* __tm)
    REGISTER_HANDLER(libc_handlers, strftime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("fmt={}", read_string_from_address(arg2)));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // size_t strftime_l(char* __buf, size_t __n, const char* __fmt, const struct tm* __tm, locale_t __l)
    REGISTER_HANDLER(libc_handlers, strftime_l, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("n={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("fmt={}", read_string_from_address(arg2)));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg3));
                     auto arg4 = get_arg_register_value(&trace_info->pre_status, 4);
                     trace_info->fun_call->args.push_back(fmt::format("l={:#x}", arg4));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // char* ctime(const time_t* __t)
    REGISTER_HANDLER(libc_handlers, ctime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("t={:#x}", arg0));
                     trace_info->fun_call->ret_type = kString;
                     });

    // char* ctime_r(const time_t* __t, char* __buf)
    REGISTER_HANDLER(libc_handlers, ctime_r, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("t={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     trace_info->fun_call->ret_type = kString;
                     });

    // void tzset(void)
    REGISTER_HANDLER(libc_handlers, tzset, {
                     trace_info->fun_call->ret_type = kVoid;
                     });

    // clock_t clock(void)
    REGISTER_HANDLER(libc_handlers, clock, {
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int clock_getcpuclockid(pid_t __pid, clockid_t* __clock)
    REGISTER_HANDLER(libc_handlers, clock_getcpuclockid, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("clock={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int clock_getres(clockid_t __clock, struct timespec* __resolution)
    REGISTER_HANDLER(libc_handlers, clock_getres, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("clock={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("resolution={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int clock_gettime(clockid_t __clock, struct timespec* __ts)
    REGISTER_HANDLER(libc_handlers, clock_gettime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("clock={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("ts={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int clock_nanosleep(clockid_t __clock, int __flags, const struct timespec* __request, struct timespec* __remainder)
    REGISTER_HANDLER(libc_handlers, clock_nanosleep, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("clock={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("request={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("remainder={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int clock_settime(clockid_t __clock, const struct timespec* __ts)
    REGISTER_HANDLER(libc_handlers, clock_settime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("clock={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("ts={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int timer_create(clockid_t __clock, struct sigevent* __event, timer_t* __timer_ptr)
    REGISTER_HANDLER(libc_handlers, timer_create, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("clock={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("event={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("timer_ptr={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int timer_delete(timer_t __timer)
    REGISTER_HANDLER(libc_handlers, timer_delete, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("timer={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int timer_settime(timer_t __timer, int __flags, const struct itimerspec* __new_value, struct itimerspec* __old_value)
    REGISTER_HANDLER(libc_handlers, timer_settime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("timer={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("new_value={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("old_value={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int timer_gettime(timer_t __timer, struct itimerspec* __ts)
    REGISTER_HANDLER(libc_handlers, timer_gettime, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("timer={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("ts={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int timer_getoverrun(timer_t __timer)
    REGISTER_HANDLER(libc_handlers, timer_getoverrun, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("timer={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // time_t timelocal(struct tm* __tm)
    REGISTER_HANDLER(libc_handlers, timelocal, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // time_t timegm(struct tm* __tm)
    REGISTER_HANDLER(libc_handlers, timegm, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("tm={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    // int pthread_atfork(void (*__prepare)(void), void (*__parent)(void), void (*__child)(void))
#if __ANDROID_API__ >= 12
    REGISTER_HANDLER(libc_handlers, pthread_atfork, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("prepare={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("parent={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("child={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
#endif /* __ANDROID_API__ >= 12 */

    // int pthread_attr_destroy(pthread_attr_t* __attr)
    REGISTER_HANDLER(libc_handlers, pthread_attr_destroy, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_getdetachstate(const pthread_attr_t* __attr, int* __state)
    REGISTER_HANDLER(libc_handlers, pthread_attr_getdetachstate, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("state={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_getguardsize(const pthread_attr_t* __attr, size_t* __size)
    REGISTER_HANDLER(libc_handlers, pthread_attr_getguardsize, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_getschedparam(const pthread_attr_t* __attr, struct sched_param* __param)
    REGISTER_HANDLER(libc_handlers, pthread_attr_getschedparam, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("param={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_getschedpolicy(const pthread_attr_t* __attr, int* __policy)
    REGISTER_HANDLER(libc_handlers, pthread_attr_getschedpolicy, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_getscope(const pthread_attr_t* __attr, int* __scope)
    REGISTER_HANDLER(libc_handlers, pthread_attr_getscope, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("scope={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_getstack(const pthread_attr_t* __attr, void** __addr, size_t* __size)
    REGISTER_HANDLER(libc_handlers, pthread_attr_getstack, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_getstacksize(const pthread_attr_t* __attr, size_t* __size)
    REGISTER_HANDLER(libc_handlers, pthread_attr_getstacksize, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_init(pthread_attr_t* __attr)
    REGISTER_HANDLER(libc_handlers, pthread_attr_init, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_setdetachstate(pthread_attr_t* __attr, int __state)
    REGISTER_HANDLER(libc_handlers, pthread_attr_setdetachstate, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("state={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_setguardsize(pthread_attr_t* __attr, size_t __size)
    REGISTER_HANDLER(libc_handlers, pthread_attr_setguardsize, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_setschedparam(pthread_attr_t* __attr, const struct sched_param* __param)
    REGISTER_HANDLER(libc_handlers, pthread_attr_setschedparam, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("param={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_setschedpolicy(pthread_attr_t* __attr, int __policy)
    REGISTER_HANDLER(libc_handlers, pthread_attr_setschedpolicy, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_setscope(pthread_attr_t* __attr, int __scope)
    REGISTER_HANDLER(libc_handlers, pthread_attr_setscope, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("scope={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_setstack(pthread_attr_t* __attr, void* __addr, size_t __size)
    REGISTER_HANDLER(libc_handlers, pthread_attr_setstack, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_attr_setstacksize(pthread_attr_t* __attr, size_t __size)
    REGISTER_HANDLER(libc_handlers, pthread_attr_setstacksize, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_condattr_destroy(pthread_condattr_t* __attr)
    REGISTER_HANDLER(libc_handlers, pthread_condattr_destroy, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_condattr_getclock(const pthread_condattr_t* __attr, clockid_t* __clock)
#if __ANDROID_API__ >= 21
    REGISTER_HANDLER(libc_handlers, pthread_condattr_getclock, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("clock={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
#endif /* __ANDROID_API__ >= 21 */

    // int pthread_condattr_getpshared(const pthread_condattr_t* __attr, int* __shared)
    REGISTER_HANDLER(libc_handlers, pthread_condattr_getpshared, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("shared={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_condattr_init(pthread_condattr_t* __attr)
    REGISTER_HANDLER(libc_handlers, pthread_condattr_init, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_condattr_setclock(pthread_condattr_t* __attr, clockid_t __clock)
#if __ANDROID_API__ >= 21
    REGISTER_HANDLER(libc_handlers, pthread_condattr_setclock, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("clock={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });
#endif /* __ANDROID_API__ >= 21 */

    // int pthread_condattr_setpshared(pthread_condattr_t* __attr, int __shared)
    REGISTER_HANDLER(libc_handlers, pthread_condattr_setpshared, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("shared={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_cond_broadcast(pthread_cond_t* __cond)
    REGISTER_HANDLER(libc_handlers, pthread_cond_broadcast, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("cond={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_cond_destroy(pthread_cond_t* __cond)
    REGISTER_HANDLER(libc_handlers, pthread_cond_destroy, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("cond={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_cond_init(pthread_cond_t* __cond, const pthread_condattr_t* __attr)
    REGISTER_HANDLER(libc_handlers, pthread_cond_init, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("cond={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_cond_signal(pthread_cond_t* __cond)
    REGISTER_HANDLER(libc_handlers, pthread_cond_signal, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("cond={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_cond_timedwait(pthread_cond_t* __cond, pthread_mutex_t* __mutex, const struct timespec* __timeout)
    REGISTER_HANDLER(libc_handlers, pthread_cond_timedwait, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("cond={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("mutex={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    // int pthread_cond_wait(pthread_cond_t* __cond, pthread_mutex_t* __mutex)
    REGISTER_HANDLER(libc_handlers, pthread_cond_wait, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("cond={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("mutex={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    REGISTER_HANDLER(libc_handlers, pthread_create, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("thread_ptr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("start_routine={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, pthread_mutex_lock, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("mutex={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, pthread_mutex_trylock, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("mutex={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, pthread_key_create, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("key_ptr={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("destructor={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, pthread_mutex_timedlock, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("mutex={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, pthread_getattr_np, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("thread={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("attr_ptr={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, pthread_cond_timedwait_relative_np, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("cond={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("mutex={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("rel_time={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });
    REGISTER_HANDLER(libc_handlers, gettimeofday, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("tv={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("tz={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, settimeofday, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("tv={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("tz={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, getitimer, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("which={}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("current_value={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, setitimer, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("which={}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("new_value={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("old_value={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, utimes, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path=\"{}\"", reinterpret_cast<const char *>(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("times={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, statfs, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path=\"{}\"", reinterpret_cast<const char *>(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

#if __ANDROID_API__ >= 21
    REGISTER_HANDLER(libc_handlers, statfs64, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path=\"{}\"", reinterpret_cast<const char *>(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });
#endif /* __ANDROID_API__ >= 21 */

    REGISTER_HANDLER(libc_handlers, fstatfs, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

#if __ANDROID_API__ >= 21
    REGISTER_HANDLER(libc_handlers, fstatfs64, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fd={}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });
#endif /* __ANDROID_API__ >= 21 */

    REGISTER_HANDLER(libc_handlers, wait, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, waitpid, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("pid={}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("options={}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

#if __ANDROID_API__ >= __ANDROID_API_J_MR2__
    REGISTER_HANDLER(libc_handlers, wait4, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("pid={}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("options={}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("rusage={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });
#endif /* __ANDROID_API__ >= __ANDROID_API_J_MR2__ */

    REGISTER_HANDLER(libc_handlers, waitid, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("type={}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("id={}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("info={:#x}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("options={}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });
    REGISTER_HANDLER(libc_handlers, clearerr, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fclose, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, feof, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, ferror, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fflush, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fgetc, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fgets, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("size={}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg2));
                     trace_info->fun_call->ret_type = kPointer;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fprintf, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("fmt=\"{}\"", reinterpret_cast<const char *>(arg1)));
                     // Variadic arguments are not captured here.
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fputc, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("ch={}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fputs, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("s=\"{}\"", reinterpret_cast<const char *>(arg0)));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fread, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("size={}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("count={}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fscanf, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(
                         fmt::format("fmt=\"{}\"", reinterpret_cast<const char *>(arg1)));
                     // Variadic arguments are not captured here.
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, fwrite, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("size={}", arg1));
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("count={}", arg2));
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, getc, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    REGISTER_HANDLER(libc_handlers, getchar, {
                     {
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    /*    ssize_t getdelim(char** __line_ptr, size_t* __line_length_ptr, int __delimiter, FILE* __fp) __INTRODUCED_IN(18);
        ssize_t getline(char** __line_ptr, size_t* __line_length_ptr, FILE* __fp) __INTRODUCED_IN(18);*/

    REGISTER_HANDLER(libc_handlers, getdelim, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     trace_info->fun_call->args.push_back(fmt::format("lineptr={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("line_length_ptr={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("delimiter={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg3));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });
    REGISTER_HANDLER(libc_handlers, getline, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("lineptr={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("line_length_ptr={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });

    /*    int remove(const char* __path);
        void rewind(FILE* __fp);
        int scanf(const char* __fmt, ...) __scanflike(1, 2);*/
    REGISTER_HANDLER(libc_handlers, remove, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("path={}", reinterpret_cast<const char *>(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    REGISTER_HANDLER(libc_handlers, rewind, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->ret_type = kVoid;
                     });
    REGISTER_HANDLER(libc_handlers, scanf, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(
                         fmt::format("fmt={}", reinterpret_cast<const char *>(arg0)));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    /*
     * int fseek(FILE* __fp, long __offset, int __whence);
     * long ftell(FILE* __fp);
     * */
    REGISTER_HANDLER(libc_handlers, fseek, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("whence={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });
    REGISTER_HANDLER(libc_handlers, ftell, {
                     {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     trace_info->fun_call->args.push_back(fmt::format("fp={:#x}", arg0));
                     trace_info->fun_call->ret_type = kNumber;
                     }
                     });
    //todo add libc fun
    //mmap
    //void* mmap(void* __addr, size_t __size, int __prot, int __flags, int __fd, off_t __offset);
    REGISTER_HANDLER(libc_handlers, mmap, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     auto arg4 = get_arg_register_value(&trace_info->pre_status, 4);
                     auto arg5 = get_arg_register_value(&trace_info->pre_status, 5);
                     trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg3));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg4));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    REGISTER_HANDLER(libc_handlers, mmap64, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
                     auto arg4 = get_arg_register_value(&trace_info->pre_status, 4);
                     auto arg5 = get_arg_register_value(&trace_info->pre_status, 5);
                     trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
                     trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                     trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg3));
                     trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg4));
                     trace_info->fun_call->ret_type = kPointer;
                     });
    //    int munmap(void* __addr, size_t __size);
    REGISTER_HANDLER(libc_handlers, munmap, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->ret_type = kNumber;
                     });

    //    int mprotect(void* __addr, size_t __size, int __prot);
    REGISTER_HANDLER(libc_handlers, mprotect, {
                     auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
                     auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
                     auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
                     trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                     trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                     trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
                     trace_info->fun_call->ret_type = kNumber;
                     });
    gum_module_enumerate_exports(module, dispatch_export_func, this);
    g_object_unref(module);
}

bool DispatchLibc::dispatch_args(inst_trace_info_t* info) {
    if (!is_module_address(info->fun_call->fun_address)) {
        return false;
    }
    info->fun_call->call_module_name = "libc.so";
    auto handler = libc_handlers.find(info->fun_call->fun_address);
    if (handler != libc_handlers.end()) {
        auto pair = handler->second;
        info->fun_call->fun_name = pair.first;
        pair.second(info);
        return true;
    }
    auto fun_name = get_address_symbol(info->fun_call->fun_address);
    if (fun_name.empty()) {
        LOGE("fun_name is empty %p", (void *) info->fun_call->fun_address);
        return true;
    }
    info->fun_call->fun_name = fun_name;
    add_common_reg_values(info);

    return true;
}

bool
DispatchLibc::get_format_result(inst_trace_info_t* trace_info, const QBDI::GPRState* ret_status) {
    auto fun_name = trace_info->fun_call->fun_name;
    if (fun_name.empty()) {
        return false;
    }
    auto ret_value = get_ret_register_value(ret_status, 0);
    auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
    auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
    auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);

    const std::unordered_map<std::string, std::function<void()>> format_fun_handlers = {
        {"sprintf", [&]() {
            if (arg0 == 0) {
                return;
            }
            trace_info->fun_call->args.clear();
            trace_info->fun_call->args.push_back(fmt::format("fmt={}", (arg1)));
            trace_info->fun_call->ret_value = fmt::format("{}", read_string_from_address(arg0));
        }},
        {"vsprintf", [&]() {
            if (arg0 == 0) {
                return;
            }
            trace_info->fun_call->args.clear();
            trace_info->fun_call->args.push_back(fmt::format("fmt={}", (arg1)));
            trace_info->fun_call->ret_value = fmt::format("{}", read_string_from_address(arg0));
        }},
        {"snprintf", [&]() {
            if (arg0 == 0) {
                return;
            }
            //int snprintf(char* __buf, size_t __size, const char* __fmt, ...)
            trace_info->fun_call->args.clear();
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", (arg1)));
            trace_info->fun_call->args.push_back(
                fmt::format("fmt={}", read_string_from_address(arg2)));
            trace_info->fun_call->ret_value = fmt::format("{}", read_string_from_address(arg0));
        }},
        //int sscanf(const char* __s, const char* __fmt, ...)
        {"sscanf", [&]() {
            if (arg0 == 0) {
                return;
            }
            trace_info->fun_call->args.clear();
            trace_info->fun_call->args.push_back(
                fmt::format("fmt={}", read_string_from_address(arg1)));
            trace_info->fun_call->ret_value = fmt::format("{}", read_string_from_address(arg0));
        }},
    };
    if (format_fun_handlers.find(fun_name) != format_fun_handlers.end()) {
        format_fun_handlers.at(fun_name)();
        return true;
    }
    return false;
}

bool DispatchLibc::record_memory_info(inst_trace_info_t* trace_info, const QBDI::GPRState* ret_status) {
    auto fun_name = trace_info->fun_call->fun_name;
    if (fun_name.empty()) {
        return false;
    }
    auto ret_value = get_ret_register_value(ret_status, 0);
    auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
    auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
    auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
    const std::unordered_map<std::string, std::function<void()>> memory_fun_handlers = {
        {"malloc", [&]() {
            if (arg0 == 0 || ret_value == 0) {
                return;
            }
            MemoryManager::get_instance()->add_memory(ret_value, arg0);
            return;
        }},
        {"free", [&]() {
            if (arg0 == 0) {
                return;
            }
            MemoryManager::get_instance()->remove_memory(ret_value);
        }},
        {"calloc", [&]() {
            if (arg0 == 0 || ret_value == 0 || arg1 == 0) {
                return;
            }
            auto len = arg0 * arg1;
            MemoryManager::get_instance()->add_memory(ret_value, len);
            return;

        }},
        {"realloc", [&]() {
            //void* realloc(void* __ptr, size_t __byte_count)
            if (arg1 == 0) {
                MemoryManager::get_instance()->remove_memory(arg0);
                return;
            }
            if (ret_value != 0 || arg0 != 0) {
                MemoryManager::get_instance()->remove_memory(arg0);
            }
            if (ret_value != 0) {
                MemoryManager::get_instance()->add_memory(ret_value, arg1);
            }
        }},
        {"mmap", [&]() {
            //void* mmap(void* __addr, size_t __byte_count, int __prot, int __flags, int __fd, off_t __offset)
            if (arg1 == 0 || ret_value == 0) {
                return;
            }
            MemoryManager::get_instance()->add_memory(ret_value, arg1);
        }},
        {"mmap64", [&]() {
            //void* mmap(void* __addr, size_t __byte_count, int __prot, int __flags, int __fd, off_t __offset)
            if (arg1 == 0 || ret_value == 0) {
                return;
            }
            MemoryManager::get_instance()->add_memory(ret_value, arg1);
        }},
        {"munmap", [&]() {
            //int munmap(void* __addr, size_t __byte_count)
            if (arg0 == 0) {
                return;
            }
            MemoryManager::get_instance()->remove_memory(arg0);
        }},
    };
    if (memory_fun_handlers.find(fun_name) != memory_fun_handlers.end()) {
        memory_fun_handlers.at(fun_name)();
        return true;
    }
    return false;
}

bool DispatchLibc::dispatch_ret(inst_trace_info_t* info, const QBDI::GPRState* ret_status) {
    if (!is_module_address(info->fun_call->fun_address)) {
        return false;
    }
    auto ret_type = info->fun_call->ret_type;
    auto ret_value = get_ret_register_value(ret_status, 0);
    record_memory_info(info, ret_status);
    switch (ret_type) {
        case kUnknown: {
            if (get_format_result(info, ret_status)) {
                return true;
            }
        }
        break;
        case kNumber: {
            info->fun_call->ret_value = fmt::format("ret={:#x}", ret_value);
        }
        break;
        case kPointer: {
            info->fun_call->ret_value = (fmt::format("ret={}",
                                                     read_buffer_hexdump_from_address(
                                                         ret_value)));
        }
        break;
        case kString: {
            info->fun_call->ret_value = (
                fmt::format("ret={}", read_string_from_address(ret_value)));
        }
        break;
        default:
            break;
    }
    return true;

}