/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LPUBSTRINGS_H
#define LPUBSTRINGS_H

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stack>

char *copyString(const char *string, size_t pad = 0);
char *cleanedUpPath(const char* path);
void  replaceStringCharacter(char*, char, char, int = 1);
void  stripTrailingPathSeparators(char*);
char *directoryFromPath(const char*);
char *componentsJoinedByString(char** array, int count,
          const char* separator);
char **componentsSeparatedByString(const char* string,
              const char* separator, int& count);
char *stringByReplacingSubstring(const char* string,
                const char* oldSubstring,
                const char* newSubstring,
                bool repeat = true);
template<class T> inline void deleteStringArray(T** array, int count)
{
  int i;
  for (i = 0; i < count; i++)
  {
    delete array[i];
  }
  delete array;
}
// END STRING UTILITIES

#endif // LPUBSTRINGS_H
