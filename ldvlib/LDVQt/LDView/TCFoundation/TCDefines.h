#ifndef __TCDEFINES_H__
#define __TCDEFINES_H__

// This is a shameless hack but I haven't found a better workardound.
// QMake is mysteriously setting _MSC_VER to 1933 which generates all 
// manner of errors when using _MSC_VER 1944 (Visual Studio 2022 17.13.
// To counter this situation, set QMAKE_MSC_VER as a preprocessor macro
// used to redefine _MSC_VER.
#if defined(_MSC_VER) && defined(_QT)
#  if (QMAKE_MSC_VER >= 1933)
#    define _MSC_VER QMAKE_MSC_VER
#  endif 
#endif // _MSC_VER && _QT

#include <string>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <stddef.h>

#if __cplusplus >= 201703L || (defined(_MSC_VER) && _MSC_VER >= 1920)
#define FALLTHROUGH [[fallthrough]];
#else // C++17 or later OR Visual Studio 2019 or later
#define FALLTHROUGH
#endif // C++17 or later OR Visual Studio 2019 or later

#define PNGDATA_1X 41
#define PNGDATA_2X 42

// MSVC SUCKS! We should not have to check the fricken MS compiler version
// before defining nullptr when __cplusplus <= 199711, but they don't give
// a sane __cplusplus, so we have to just ignore that and go by MSVC
// compiler version. Once again, MSVC SUCKS!
#if defined(_MSC_VER)
#  if _MSC_VER < 1800 // 1800 is Visual Studio 2013.
#    define nullptr NULL
#  endif
#elif __cplusplus <= 199711L
#  define nullptr NULL
#endif

#ifdef COCOA
// The following is necessary to get rid of some truly screwed up warnings that
// show up when compiling on the Mac.
#pragma GCC visibility push(default)
#endif // COCOA

#ifdef WIN32

#define REQUIRES_LOCK_HELD(lock) _Requires_lock_held_(lock)

#define RT_PNGDATA_1X MAKEINTRESOURCE(PNGDATA_1X)
#define RT_PNGDATA_2X MAKEINTRESOURCE(PNGDATA_2X)

// The following shouldn't be necessary here, but due to bugs in Microsoft's
// precompiled headers, it is.  The warning being disabled below is the one
// that warns about identifiers longer than 255 characters being truncated to
// 255 characters in the debug info.  (Also, "not inlined", and one other.)
#pragma warning(disable : 4786 4702 4710)

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ifdef _WIN32_WINDOWS
#undef _WIN32_WINDOWS
#endif // _WIN32_WINDOWS

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif // _WIN32_WINNT

#define _WIN32_WINDOWS 0x0600
#define _WIN32_WINNT 0x0600

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
   #define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif // _DEBUG

#include <winsock2.h>
#include <crtdbg.h>

#undef min
#undef max

#pragma warning( disable : 4514 4127 )

#ifdef _BUILDING_TCFOUNDATION
#define TCExport __declspec(dllexport)
#elif defined _BUILDING_TCFOUNDATION_LIB || defined _TC_STATIC
#define TCExport
#else // _BUILDING_TCFOUNDATION
#define TCExport __declspec(dllimport)
#endif // _BUILDING_TCFOUNDATION

#else // WIN32

#define REQUIRES_LOCK_HELD(lock)
#define TCExport

#endif // WIN32

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifdef __APPLE__
#endif // __APPLE__

#define START_IGNORE_DEPRECATION \
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")

#define END_IGNORE_DEPRECATION _Pragma("clang diagnostic pop")

#if defined (__APPLE__) || defined (_OSMESA)
#include <wchar.h>
#endif // __APPLE__ || _OSMESA

// NOTE: the following should be 1, 2, and 4 bytes each.  So on a 64-bit system,
// the following defines aren't appropriate, and something else needs to be
// substituted.

typedef unsigned char TCByte;
typedef unsigned char TCUChar;
typedef unsigned short TCUShort;
typedef unsigned int TCUInt;
#ifdef __U32_TYPE
typedef __U32_TYPE TCULong;
#else // __U32_TYPE
// I know the following seems wrong, but int should always be 32-bit.
typedef unsigned int TCULong;
#endif // __U32_TYPE
typedef char TCChar;
typedef short TCShort;
typedef int TCInt;
// I know the following seems wrong, but int should always be 32-bit.
typedef int TCLong;

// Define LDVIEW_DOUBLES to have LDView use doubles instead of floats.  Comment
// out the definition for floats.
//#define LDVIEW_DOUBLES

// I'm not sure if floats are 64 bits on a 64-bit system or not.  I know that
// TCFloat has to be 32 bits when LDVIEW_DOUBLES isn't defined in order for it
// to work.
#ifdef LDVIEW_DOUBLES
typedef double TCFloat;
#else // LDVIEW_DOUBLES
typedef float TCFloat;
#endif // LDVIEW_DOUBLES

// The following must always be defined to 32 bits.
typedef float TCFloat32;

#ifndef __THROW
#define __THROW
#endif //__THROW

//#define TC_NO_UNICODE

#ifdef TC_NO_UNICODE
typedef char UCCHAR;
typedef char * UCSTR;
typedef const char * CUCSTR;
typedef std::string UCSTRING;
typedef std::string ucstring;
#define _UC(x) x
#else // TC_NO_UNICODE
typedef wchar_t UCCHAR;
typedef wchar_t * UCSTR;
typedef const wchar_t * CUCSTR;
typedef std::wstring UCSTRING;
typedef std::wstring ucstring;
#define _UC(x) L ## x
#endif // TC_NO_UNICODE

typedef std::vector<int> IntVector;
typedef std::vector<ptrdiff_t> PtrDiffTVector;
typedef std::vector<size_t> SizeTVector;
typedef std::vector<long> LongVector;
typedef std::vector<std::string> StringVector;
typedef std::set<int> IntSet;
typedef std::set<std::string> StringSet;
typedef std::list<int> IntList;
typedef std::list<size_t> SizeTList;
typedef std::list<std::string> StringList;
typedef std::list<TCULong> TCULongList;
typedef std::map<int, int> IntIntMap;
typedef std::map<int, bool> IntBoolMap;
typedef std::map<std::string, int> StringIntMap;
typedef std::map<std::string, bool> StringBoolMap;
typedef std::map<std::string, std::string> StringStringMap;
typedef std::map<std::string, ucstring> StringUCStringMap;
typedef std::map<std::string, long> StringLongMap;
typedef std::map<std::string, float> StringFloatMap;
typedef std::map<char, std::string> CharStringMap;
typedef std::map<std::wstring, std::wstring> WStringWStringMap;

#endif // __TCDEFINES_H__
