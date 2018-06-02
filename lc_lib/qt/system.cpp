#include "lc_global.h"

#ifdef WIN32
/*** LPub3D Mod - use TC instance of strcasestr() ***/
#ifdef _TC_STATIC
  #include "lpub_strings.h"
#else
/*** LPub3D Mod end ***/
char* strcasestr(const char *s, const char *find)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != 0)
	{
		c = tolower((unsigned char)c);
		len = strlen(find);
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
#else

char* strupr(char *string)
{
	for (char *c = string; *c; c++)
		*c = toupper(*c);

	return string;
}

char* strlwr(char *string)
{
	for (char *c = string; *c; c++)
		*c = tolower(*c);

	return string;
}

#endif
