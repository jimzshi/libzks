#ifndef ZKS_CONFIGURE_H_
#define ZKS_CONFIGURE_H_

#include <cinttypes>

#if defined(_WIN32) || defined(_WIN64)
#define ZKS_OS_WINDOWS_
#elif defined(__gnu_linux__)
#define ZKS_OS_GNULINUX_
#elif defined(__APPLE__) && defined(__MACH__)
#define ZKS_OS_MACOSX_
#else
#define OS_UNKNOWN
#endif

#if defined (ZKS_OS_WINDOWS_) && defined(__AFX_H__)
#define _HAS_WIN32_CSTRING
#endif

//#if defined(ZKS_OS_WINDOWS_)
//#define _HAS_CHAR_T_SUPPORT
//#endif

#if defined(ZKS_OS_WINDOWS_) || defined(__clang__)
#define _HAS_CODECVT
#endif

#define _ZKS_U8STRING_INDEX 1

#define _ZKS_U8STRING_NOVALIDATION

#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__) || defined(_M_IA64) || defined(__IA64__)
#define _ZKS64
#else
#define _ZKS32
#endif

typedef int64_t size_i;

#endif
