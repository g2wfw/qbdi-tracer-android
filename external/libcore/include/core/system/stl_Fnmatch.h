#ifndef	_FNMATCH_H
#define	_FNMATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define	FNM_PATHNAME 0x1 //如果设置了这个标志,仅在字符串中匹配斜杠以斜线(/)中,而不能由星号(*)或者一个问号(?)元字符,也由括号表达式([])包含斜杠.
#define	FNM_NOESCAPE 0x2 //如果设置了此标志,反斜线视为普通字符的转义字符
#define	FNM_PERIOD   0x4 //如果设置了这个标志 ,pattern 中的前导句点来匹配字符串中有精确的期限.如果被看作是领先的是字符的字符串,或如果有 fnm_pathname 是紧跟在斜杠和期限.
#define	FNM_LEADING_DIR	0x8 //如果此标志(gnu扩展)设置,如果它匹配来匹配模式被认为是一个初始段的字符串,它后面跟有一个正斜杠.此标志主要用于在内部使用的 glibc ,只有在特定的情况下实现.
#define	FNM_CASEFOLD	0x10 //如果此标志(gnu扩展)设置,模式是匹配 case insensitively .
#define	FNM_FILE_NAME	FNM_PATHNAME

#define	FNM_NOMATCH 1
#define FNM_NOSYS   (-1)
#ifndef STL_API
#ifdef _WIN32
#define STL_API __declspec(dllexport)
#else
#define STL_API __attribute__((visibility("default")))
#endif
#endif
STL_API int fnmatch(const char *, const char *, int);

#ifdef __cplusplus
}
#endif

#endif