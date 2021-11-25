#include "lc_global.h"

#ifdef Q_OS_WIN
/*** LPub3D Mod - Use LDVLib instance of strcasestr ***/
#ifndef _TC_STATIC
char* strcasestr(const char *s, const char *find)
{
	char c, sc;

	if ((c = *find++) != 0)
	{
		c = tolower((unsigned char)c);
		const int len = (int)strlen(find);
		do
		{
			do
			{
				if ((sc = *s++) == 0)
					return (nullptr);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (qstrnicmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}
#endif
/*** LPub3D Mod end ***/
#else

char* strupr(char *string)
{
	for (char *c = string; *c; c++)
		*c = toupper(*c);

	return string;
}

#endif
