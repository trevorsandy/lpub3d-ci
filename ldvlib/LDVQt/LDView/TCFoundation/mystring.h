#ifndef __MYSTRING_H__
#define __MYSTRING_H__

#include <TCFoundation/TCDefines.h>
#include <TCFoundation/TCStlIncludes.h>

#include <istream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _AIX
#include <strings.h>
#endif

#ifdef _QT
#include <QtCore/qstring.h>
#endif // _QT

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef _OSMESA
#include <stdarg.h>
#endif // _OSMESA

#ifdef TC_NO_UNICODE
#define ucstrcmp strcmp
#define ucstrstr strstr
#define ucstrcpy strcpy
#define ucstrcat strcat
#define ucstrncpy strncpy
#define ucstrlen strlen
#define ucstrchr strchr
#define ucstrrchr strrchr
#ifdef WIN32
#define ucstrcasecmp stricmp
#else
#define ucstrcasecmp strcasecmp
#endif
#define sucscanf sscanf
#else // TC_NO_UNICODE
#define ucstrcmp wcscmp
#define ucstrstr wcsstr
#define ucstrcpy wcscpy
#define ucstrcat wcscat
#define ucstrncpy wcsncpy
#define ucstrlen wcslen
#define ucstrchr wcschr
#define ucstrrchr wcsrchr
#ifdef WIN32
#define ucstrcasecmp wcsicmp
#define ucstrncasecmp wcsnicmp
#else
#define ucstrcasecmp wcscasecmp
#define ucstrncasecmp wcsncasecmp
#endif
#define sucscanf swscanf
#endif // TC_NO_UNICODE

TCExport int ucunlink(const char* filename);
TCExport int ucrename(const char* src, const char* dst);
TCExport int ucclose(int fd);
TCExport FILE *ucfopen(const char *filename, const char *mode);
#ifdef _MSC_VER
typedef struct _stat TCStat;
#else // _MCS_VER
typedef struct stat TCStat;
#endif // !_MCS_VER
int ucstat(const char* filename, TCStat* statBuf);
TCExport char *copyString(const char *string, size_t pad = 0);
TCExport wchar_t *copyString(const wchar_t *string, size_t pad = 0);

#ifndef __APPLE__
TCExport char *strnstr(const char *s1, const char *s2, size_t n);
#ifndef __USE_GNU
TCExport char *strcasestr(const char *s1, const char *s2) __THROW;
#endif // !__USE_GNU
#endif // !__APPLE__
TCExport char *strnstr2(const char *s1, const char *s2, size_t n,
						int skipZero);
TCExport char *strncasestr(const char *s1, const char *s2, size_t n, int 
skipZero = 0);

TCExport UCSTR ucstrcasestr(CUCSTR s1, CUCSTR s2);

//int stringHasPrefix(const char *s1, const char *s2);

TCExport void printStringArray(char** array, int count);
TCExport char **copyStringArray(char** array, int count);
template<class T> inline TCExport void deleteStringArray(T** array, size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
	{
		delete[] array[i];
	}
	delete[] array;
}

TCExport bool arrayContainsString(char** array, int count, const char* string);
TCExport bool arrayContainsPrefix(char** array, int count, const char* prefix);
TCExport char **componentsSeparatedByString(const char* string,
											const char* separator, size_t& count);
TCExport wchar_t **componentsSeparatedByString(const wchar_t* string,
											   const wchar_t* separator,
											   size_t& count);
TCExport char *componentsJoinedByString(char** array, size_t count,
	const char* separator);
TCExport bool stringHasPrefix(const char* string, const char* prefix);
TCExport bool stringHasPrefix(const std::string& string, const std::string& prefix);
TCExport bool stringHasPrefix(const wchar_t *string, const wchar_t *prefix);
TCExport bool stringHasCaseInsensitivePrefix(const char* string,
	const char* prefix);
TCExport bool stringHasCaseInsensitivePrefix(const wchar_t* string,
	const wchar_t* prefix);
TCExport bool stringHasSuffix(const char* string, const char* suffix);
TCExport bool stringHasSuffix(const wchar_t* string, const wchar_t* suffix);
TCExport bool stringHasCaseInsensitiveSuffix(const char* string,
	const char* suffix);
TCExport bool stringHasCaseInsensitiveSuffix(const wchar_t* string,
	const wchar_t* suffix);
TCExport char* convertStringToUpper(char*);
TCExport char* convertStringToLower(char*);
TCExport std::string& convertStringToLower(std::string&);
TCExport std::string& convertStringToUpper(std::string&);
TCExport std::string lowerCaseString(const std::string &src);
TCExport std::string upperCaseString(const std::string &src);

TCExport char* filenameFromPath(const char*);
TCExport ucstring filenameFromPath(const ucstring &path);
TCExport char* directoryFromPath(const char*);
TCExport std::string directoryFromPath(const std::string &path);
TCExport ucstring directoryFromPath(const ucstring &path);
TCExport bool isRelativePath(const char*);
TCExport bool isDirectoryPath(const char* path);
TCExport bool isDirectoryPath(const std::string &path);
TCExport char* findRelativePath(const char* cwd, const char* path);
TCExport char* cleanedUpPath(const char* path);
TCExport void combinePath(const std::string &left, const std::string &right,
	std::string &combinedPath);
TCExport void combinePath(const char *left, const char *right,
	std::string &combinedPath);
TCExport void removeExtenstion(std::string&);
TCExport bool isFileUri(const std::string& fileUri);
TCExport std::string pathFromFileUri(const std::string& fileUri);

TCExport char* findExecutable(const char* executable);

TCExport void stripCRLF(std::string&);
TCExport void stripCRLF(char*);
TCExport void stripCRLF(wchar_t*);
TCExport void stripTrailingWhitespace(char*);
TCExport void stripTrailingWhitespace(wchar_t*);
TCExport void stripLeadingWhitespace(char*);
TCExport void stripLeadingWhitespace(wchar_t*);
TCExport void stripTrailingPathSeparators(char*);
TCExport void stripTrailingPathSeparators(wchar_t*);
TCExport void replaceStringCharacter(char*, char, char, int = 1);
TCExport void replaceStringCharacter(std::string&, char, char, int = 1);
TCExport void replaceStringCharacter(wchar_t*, wchar_t, wchar_t, int = 1);
TCExport char *stringByReplacingSubstring(const char* string,
										  const char* oldSubstring,
										  const char* newSubstring,
										  bool repeat = true);
TCExport wchar_t* stringByReplacingSubstring(
	const wchar_t* string,
	const wchar_t* oldSubstring,
	const wchar_t* newSubstring,
	bool repeat);
TCExport int countStringLines(const char*);

TCExport bool getCurrentDirectory(std::string &dir);
TCExport bool setCurrentDirectory(const std::string &dir);
TCExport bool createDirectory(const std::string &dir);
TCExport bool ensurePath(const std::string &path);

TCExport void setDebugLevel(int value);
TCExport int getDebugLevel(void);
TCExport void debugVLog(const char *udKey, const wchar_t *format,
	va_list argPtr);
TCExport void debugLog(const char *udKey, const wchar_t *format, ...);
TCExport void debugLog1s(const char *udKey, const wchar_t *format,
	const wchar_t *value);
TCExport void debugVLog(const char *udKey, const char *format, va_list argPtr);
TCExport void debugLog(const char *udKey, const char *format, ...);
TCExport void debugLog1s(const char *udKey, const char *format,
	const char *value);
TCExport void debugVPrintf(int level, const char *format, va_list argPtr);
TCExport void debugPrintf(const char *format, ...);
TCExport void debugPrintf(int level, const char *format, ...);
TCExport void indentPrintf(int indent, const char *format, ...);
TCExport void consolePrintf(const char *format, ...);
TCExport void consolePrintf(const wchar_t *format, ...);

TCExport void processEscapedString(char *string);
TCExport void processEscapedString(wchar_t *string);
TCExport char *createEscapedString(const char *string);

TCExport void stringtowstring(std::wstring &dst, const std::string &src);
TCExport void mbstowstring(std::wstring &dst, const char *src, int length = -1);
TCExport void wstringtostring(std::string &dst, const std::wstring &src);
TCExport void wcstostring(std::string &dst, const wchar_t *src, int length = -1);
#ifdef NO_WSTRING
TCExport unsigned long wcstoul(const wchar_t *start, wchar_t **end, int base);
#endif // NO_WSTRING
TCExport ucstring stringtoucstring(const std::string &src);
TCExport void stringtoucstring(ucstring &dst, const std::string &src);
TCExport UCSTR mbstoucstring(const char *src, int length = -1);
TCExport char *ucstringtombs(CUCSTR src, int length = -1);
TCExport char *ucstringtoutf8(CUCSTR src, int length = -1);
TCExport bool wstringtoutf8(std::string& dst, const std::wstring& src);
TCExport bool wstringtoutf8(std::string& dst, const wchar_t* src, int length = -1);
TCExport bool ucstringtoutf8(std::string& dst, const ucstring& src);
TCExport bool ucstringtoutf8(std::string& dst, CUCSTR src, int length = -1);
TCExport UCSTR utf8toucstring(const char *src, int length = -1);
TCExport bool utf8towstring(std::wstring& dst, const std::string &src);
TCExport bool utf8towstring(std::wstring& dst, const char *src, int length = -1);
TCExport bool utf8toucstring(ucstring& dst, const std::string &src);
TCExport bool utf8toucstring(ucstring& dst, const char *src, int length = -1);

TCExport bool skipUtf8BomIfPresent(std::istream &stream);

#ifdef WIN32

TCExport void runningWithConsole(bool bRealConsole = false);
TCExport bool haveConsole(void);
TCExport struct tm* gmtime_r(const time_t *clock, struct tm *result);
TCExport int mkstemp(char* nameTemplate);

#endif // WIN32

TCExport std::string stringFromLongLong(long long, bool commas = false);
TCExport long long longLongFromString(char*);


typedef std::vector<ucstring> ucstringVector;

TCExport void mbstoucstring(ucstring &dst, const char *src, int length = -1);
TCExport int sucprintf(UCSTR buffer, size_t maxLen, CUCSTR format, ...);
TCExport int vsucprintf(UCSTR buffer, size_t maxLen, CUCSTR format, va_list argPtr);
TCExport std::string ftostr(double value, int precision = 6);
TCExport ucstring ftoucstr(double value, int precision = 6);
TCExport std::string ltostr(long value);
TCExport size_t atoszt(const char *value);
TCExport ptrdiff_t atopdt(const char *value);
TCExport std::string szttostr(size_t value);
TCExport std::string pdttostr(ptrdiff_t value);
TCExport ucstring ltoucstr(long value);

TCExport bool base64Decode(const std::string& input, std::vector<TCByte>& decodedBytes);
TCExport bool isInBase64Charset(char character);

// LPub3D Mod - format string
TCExport std::string formatString(const char *format, ...);
// LPub3D Mod End

template<typename T, size_t size> size_t COUNT_OF(const T(&)[size]) { return size; }

TCExport bool isLittleEndian();

#endif
