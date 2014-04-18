#ifndef ZKS_CONFIGURE_H_
#define ZKS_CONFIGURE_H_

#if defined(_WIN32) || defined(_WIN64)
#define OS_WINDOWS
#elif defined(__gnu_linux__)
#define OS_GNU_LINUX
#else
#define OS_UNKNOWN
#endif

#if defined (OS_WINDOWS) && defined(__AFX_H__)
#define _HAS_WIN32_CSTRING
#endif

#if defined(OS_WINDOWS)
#define _HAS_CHAR_T_SUPPORT
#endif

#if defined(OS_WINDOWS) || defined(__clang__)
#define _HAS_CODECVT
#endif

#define _ZKS_U8STRING_INDEX 1

#define _ZKS_U8STRING_NOVALIDATION

#endif