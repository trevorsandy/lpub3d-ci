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

#include "lpub_strings.h"

// STRING UTILITIES
char *copyString(const char *string, size_t pad)
{
  if (string)
    {
      return strcpy(new char[strlen(string) + 1 + pad], string);
    }
  else
    {
      return NULL;
    }
}

wchar_t *copyString(const wchar_t *string, size_t pad)
{
  if (string)
    {
      return wcscpy(new wchar_t[wcslen(string) + 1 + pad], string);
    }
  else
    {
      return NULL;
    }
}

char* cleanedUpPath(const char* path)
{
  char *newPath = copyString(path);

#ifdef WIN32
  replaceStringCharacter(newPath, '\\', '/');
#endif // WIN32
  if (strstr(newPath, "../"))
    {
      char **pathComponents;
      int pathCount;
      //int newCount;
      std::stack<std::string> pathStack;
      std::list<std::string> pathList;
      std::list<std::string>::const_iterator it;
      size_t len = 1;	// The terminating NULL.
      size_t offset = 0;

      pathComponents = componentsSeparatedByString(newPath, "/", pathCount);
      // Note that we're intentionally skipping the first component.  That's
      // either empty (for a Unix path), or the drive letter followed by a
      // colon (for a Windows path).  We'll put it back later, though.
      for (int i = 1; i < pathCount; i++)
        {
          if (strcmp(pathComponents[i], "..") == 0)
            {
              if (pathStack.size() > 0)
                {
                  pathStack.pop();
                }
            }
          else
            {
              pathStack.push(pathComponents[i]);
            }
        }
      while (pathStack.size() > 0)
        {
          pathList.push_front(pathStack.top());
          pathStack.pop();
          len += pathList.front().size() + 1;
        }
      len += strlen(pathComponents[0]);
      delete newPath;
      newPath = new char[len];
      strcpy(newPath, pathComponents[0]);
      offset = strlen(newPath);
      for (it = pathList.begin(); it != pathList.end(); it++)
        {
          // The following line leaves the string without a NULL terminator,
          // but the line after that puts one back.
          newPath[offset++] = '/';
          strcpy(&newPath[offset], it->c_str());
          offset += it->size();
        }
      deleteStringArray(pathComponents, pathCount);
    }
  if (strstr(newPath, "//") != NULL)
    {
      char *tempPath = stringByReplacingSubstring(newPath, "//", "/");
      delete[] newPath;
      newPath = tempPath;
    }
#ifdef WIN32
  replaceStringCharacter(newPath, '/', '\\');
#endif // WIN32
  return newPath;
}

void replaceStringCharacter(char* string, char oldChar, char newChar, int
                            repeat)
{
  char* spot;

  if ((spot = strchr(string, oldChar)) != NULL)
    {
      *spot = newChar;
      if (repeat)
        {
          while ((spot = strchr(spot, oldChar)) != NULL)
            {
              *spot = newChar;
            }
        }
    }
}

void stripTrailingPathSeparators(char* path)
{
  if (path)
    {
      size_t length = strlen(path);

#ifdef WIN32
      while (length > 0 && (path[length-1] == '\\' || path[length-1] == '/'))
#else
      while (length > 0 && path[length-1] == '/')
#endif
	{
	  path[--length] = 0;
	}
    }
}

char* componentsJoinedByString(char** array, int count, const char* separator)
{
  size_t length = 0;
  int i;
  size_t separatorLength = strlen(separator);
  char* string;

  for (i = 0; i < count; i++)
    {
      length += strlen(array[i]);
      if (i < count - 1)
        {
          length += separatorLength;
        }
    }
  string = new char[length + 1];
  string[0] = 0;
  for (i = 0; i < count - 1; i++)
    {
      strcat(string, array[i]);
      strcat(string, separator);
    }
  if (count)
    {
      strcat(string, array[count - 1]);
    }
  return string;
}

char** componentsSeparatedByString(const char* string, const char* separator,
                                   int &count)
{
  int i;
  char* spot = (char*)string;
  char* tokenEnd = NULL;
  size_t separatorLength = strlen(separator);
  char** components;
  char* stringCopy;

  if (strlen(string) == 0)
    {
      count = 0;
      return NULL;
    }
  for (i = 0; (spot = strstr(spot, separator)) != NULL; i++)
    {
      spot += separatorLength;
    }
  count = i + 1;
  components = new char*[count];
  stringCopy = new char[strlen(string) + 1];
  strcpy(stringCopy, string);
  tokenEnd = strstr(stringCopy, separator);
  if (tokenEnd)
    {
      *tokenEnd = 0;
    }
  spot = stringCopy;
  for (i = 0; i < count; i++)
    {
      if (spot)
        {
          components[i] = new char[strlen(spot) + 1];
          strcpy(components[i], spot);
        }
      else
        {
          components[i] = new char[1];
          components[i][0] = 0;
        }
      if (tokenEnd)
        {
          spot = tokenEnd + separatorLength;
        }
      else
        {
          spot = NULL;
        }
      if (spot)
        {
          tokenEnd = strstr(spot, separator);
          if (tokenEnd)
            {
              *tokenEnd = 0;
            }
        }
      else
        {
          tokenEnd = NULL;
        }
    }
  delete stringCopy;
  return components;
}

char *stringByReplacingSubstring(const char* string, const char* oldSubstring,
                                 const char* newSubstring, bool repeat)
{
  char *newString = NULL;

  if (repeat)
    {
      int count;
      char **components = componentsSeparatedByString(string, oldSubstring,
                                                      count);

      newString = componentsJoinedByString(components, count,
                                           newSubstring);
      deleteStringArray(components, count);
    }
  else
    {
      const char *oldSpot = strstr(string, oldSubstring);

      if (oldSpot)
        {
          size_t oldSubLength = strlen(oldSubstring);
          size_t newSubLength = strlen(newSubstring);
          size_t preLength = oldSpot - string;

          newString = new char[strlen(string) + newSubLength - oldSubLength +
              1];
          strncpy(newString, string, preLength);
          strcpy(newString + preLength, newSubstring);
          strcpy(newString + preLength + newSubLength,
                 string + preLength + oldSubLength);
        }
      else
        {
          newString = new char[strlen(string) + 1];
          strcpy(newString, string);
        }
    }
  return newString;
}

char* directoryFromPath(const char* path)
{
  if (path)
    {
      const char* slashSpot = strrchr(path, '/');
#ifdef WIN32
      const char* backslashSpot = strrchr(path, '\\');

      if (backslashSpot > slashSpot)
        {
          slashSpot = backslashSpot;
        }
#endif // WIN32
      if (slashSpot)
        {
          size_t length = slashSpot - path;
          char* directory = new char[length + 1];

          strncpy(directory, path, length);
          directory[length] = 0;
          return directory;
        }
      else
        {
          return copyString("");
        }
    }
  else
    {
      return NULL;
    }
}

