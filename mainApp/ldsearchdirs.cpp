/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

#include "ldsearchdirs.h"
#include "ldrawini.h"
#include "threadworkers.h"
#include "lpub_preferences.h"

char *LDSearchDirs::sm_systemLDrawDir = NULL;
char *LDSearchDirs::sm_defaultLDrawDir = NULL;
LDrawIniS *LDSearchDirs::sm_lDrawIni = NULL;

LDLFileCaseCallback LDSearchDirs::fileCaseCallback = NULL;
LDSearchDirs::LDLPartsCleanup LDSearchDirs::sm_cleanup;

StringList LDSearchDirs::sm_checkDirs;

LDSearchDirs::LDLPartsCleanup::~LDLPartsCleanup(void)
{
  delete LDSearchDirs::sm_systemLDrawDir;
  delete LDSearchDirs::sm_defaultLDrawDir;
  LDSearchDirs::sm_systemLDrawDir = NULL;
  if (LDSearchDirs::sm_lDrawIni)
    {
      LDrawIniFree(LDSearchDirs::sm_lDrawIni);
    }
}

LDSearchDirs::LDSearchDirs(void){}

// NOTE: static function.
bool LDSearchDirs::verifyLDrawDir(const char *value)
{
  char currentDir[1024];
  bool retValue = false;

  if (value && getcwd(currentDir, sizeof(currentDir)))
    {
      if (chdir(value) == 0)
        {
          if (chdir("parts") == 0 && chdir("..") == 0 && chdir("p") == 0)
            {
              retValue = true;
            }
        }
      if (chdir(currentDir) != 0)
        {
          qDebug("Error going back to original directory.\n");
          qDebug("currentDir before: <%s>\n", currentDir);
          if (getcwd(currentDir, sizeof(currentDir)) != NULL)
            {
              qDebug("currentDir  after: <%s>\n", currentDir);
            }
        }
    }
  return retValue;
}

// NOTE: static function. (NOT USED)
bool LDSearchDirs::verifyExtraDir(const char *value)
{
  char currentDir[1024];
  bool retValue = false;

  if (value && getcwd(currentDir, sizeof(currentDir)))
    {
      if (chdir(value) == 0)
        {
            retValue = true;
        }
      if (chdir(currentDir) != 0)
        {
          qDebug("Error going back to original directory.\n");
          qDebug("currentDir before: <%s>\n", currentDir);
          if (getcwd(currentDir, sizeof(currentDir)) != NULL)
            {
              qDebug("currentDir  after: <%s>\n", currentDir);
            }
        }
    }
  return retValue;
}

// NOTE: static function.
bool LDSearchDirs::verifyExcludedDir(const char *value){
  value = value;
  //TODO
  //get list from preferences
  /*  bool validated;
   *  foreach(QString validDir, Preferences::listValidDirs){
   *      // this
   *
   *      validated = false;
   *      if (value == validDir.toLatin1().constData())
   *         validated = true;
   *
   *      //or
   *
   *      validated = value == validDir.toLatin1().constData();
   * }
   * return validated;
   */
  return true;
}

// NOTE: static function.
void LDSearchDirs::setFileCaseCallback(LDLFileCaseCallback value)
{
  fileCaseCallback = value;
  // If bool isn't 1 byte, then we can't support the case callback, so don't
  // even try.  This will trigger an error in LDLMainModel if the P and/or
  // PARTS directory inside the LDraw directory aren't capitalized.
  if (sizeof(bool) == sizeof(char))
    {
      if (sm_lDrawIni)
        {
          LDrawIniSetFileCaseCallback(fileCaseCallback);
        }
    }
}

// NOTE: static function.
void LDSearchDirs::setLDrawDir(const char *value)
{
  if (value != sm_systemLDrawDir || !value)
    {
      delete sm_systemLDrawDir;
      if (value)
        {
          sm_systemLDrawDir = cleanedUpPath(value);
        }
      else
        {
          sm_systemLDrawDir = NULL;
        }
      if (sm_lDrawIni)
        {
          LDrawIniFree(sm_lDrawIni);
        }
      sm_lDrawIni = LDrawIniGet(sm_systemLDrawDir, NULL, NULL);
      if (sm_lDrawIni)
        {
          if (fileCaseCallback)
            {
              LDrawIniSetFileCaseCallback(fileCaseCallback);
            }
          if (!sm_systemLDrawDir)
            {
              sm_systemLDrawDir = copyString(sm_lDrawIni->LDrawDir);
            }
          if (sm_systemLDrawDir)
            {
              stripTrailingPathSeparators(sm_systemLDrawDir);
              LDrawIniComputeRealDirs(sm_lDrawIni, 1, 0, NULL);
            }
        }
    }
}

void LDSearchDirs::initCheckDirs()
{
  const char *value = getenv("LDRAWDIR");

  if (value)
    {
      sm_checkDirs.push_back(value);
    }

#ifdef WIN32
  char buf[1024];

  const char *ldraw = "LDraw";
  const char *baseDirectory = "BaseDirectory";
  const char *empty = "";
  const char *ldrawIni = "ldraw.ini";

  if (GetPrivateProfileString((const wchar_t*)ldraw, (const wchar_t*)baseDirectory,
                              (const wchar_t*)empty, (wchar_t*)buf, 1024, (const wchar_t*)ldrawIni))
    {
      buf[1023] = 0;
    }

  if (buf[0])
    {
      sm_checkDirs.push_back(buf);
    }

  sm_checkDirs.push_back("C:\\ldraw");

  delete ldraw;
  delete baseDirectory;
  delete empty;
  delete ldrawIni;

#else // WIN32
#ifdef __APPLE__
  const char *libDir = "/Library/ldraw";
  const char *homeDir = getenv("HOME");

  if (homeDir != NULL)
    {
      char *homeLib = copyString(homeDir, strlen(libDir));

      stripTrailingPathSeparators(homeLib);
      strcat(homeLib, libDir);
      sm_checkDirs.push_back(homeLib);
      delete homeLib;
    }
  sm_checkDirs.push_back(libDir);
  sm_checkDirs.push_back("/Applications/Bricksmith/LDraw");
#else // __APPLE__
  sm_checkDirs.push_back("/usr/share/ldraw");
  const char *homeDir = getenv("HOME");
  if (homeDir != NULL)
    {
      char *cleanHome = cleanedUpPath(homeDir);
      std::string homeLDraw;

      stripTrailingPathSeparators(cleanHome);
      homeLDraw = cleanHome;
      delete cleanHome;
      homeLDraw += "/ldraw";
      sm_checkDirs.push_back(homeLDraw);
    }
#endif // __APPLE__
#endif // WIN32
  char *lpub3dDir = copyString(Preferences::lpub3dPath.toLatin1().constData());
  stripTrailingPathSeparators(lpub3dDir);
  char *lpub3dLDrawDir = copyString(lpub3dDir, 10);

  // lpub3d Dir/ldraw
  strcat(lpub3dLDrawDir, "/ldraw");
  sm_checkDirs.push_back(lpub3dLDrawDir);
#ifndef COCOA
  delete lpub3dLDrawDir;
  char *lpub3dParentDir = directoryFromPath(lpub3dDir);
  stripTrailingPathSeparators(lpub3dParentDir);
  lpub3dLDrawDir = copyString(lpub3dParentDir, 10);
  delete lpub3dParentDir;
  // lpub3d Dir/../ldraw
  strcat(lpub3dLDrawDir, "/ldraw");
  sm_checkDirs.push_back(lpub3dLDrawDir);
#endif // COCOA
  delete lpub3dDir;
  delete lpub3dLDrawDir;
}

// NOTE: static function.
const char* LDSearchDirs::lDrawDir(bool defaultValue /*= false*/)
{
  char *origValue = NULL;

  if (defaultValue)
    {
      if (sm_defaultLDrawDir)
        {
          return sm_defaultLDrawDir;
        }
      origValue = copyString(sm_systemLDrawDir);
      if (sm_systemLDrawDir)
        {
          setLDrawDir(NULL);
        }
    }
  if (!sm_systemLDrawDir)
    {
      bool found = false;

      if (sm_checkDirs.size() == 0)
        {
          initCheckDirs();
        }
      for (StringList::const_iterator it = sm_checkDirs.begin(); !found &&
           it != sm_checkDirs.end(); it++)
        {
          const char *dir = it->c_str();

          if (verifyLDrawDir(dir))
            {
              setLDrawDir(dir);
              found = true;
            }
        }
      if (!found)
        {
          sm_systemLDrawDir = copyString("");
        }
    }
  if (defaultValue)
    {
      sm_defaultLDrawDir = copyString(sm_systemLDrawDir);
      setLDrawDir(origValue);
      delete origValue;
      return sm_defaultLDrawDir;
    }
  else
    {
      return sm_systemLDrawDir;
    }
}


LDPartsDirs::LDPartsDirs(){}

void LDPartsDirs::setExtraSearchDirs(const char *value)
{
//TODO
//  if (m_extraSearchDirs != value)
//    {
//      m_extraSearchDirs->clear();
//      m_extraSearchDirs = value;
//    }
}

bool LDPartsDirs::initLDrawSearchDirs()
{
  // set default ldraw Directory
  setLDrawDir(Preferences::ldrawPath.toLatin1().constData());

  // initialize ldrawIni and check for error
  lDrawDir();

  if (sm_lDrawIni)
    {
      m_searchDirsOrigin = strdup(sm_lDrawIni->SearchDirsOrigin);
//      qDebug() << "LDrawini Initialized - m_searchDirsOrigin: " << m_searchDirsOrigin;
      return true;
    } else {
//      qDebug() << "Unable to initialize LDrawini: ";
      return false;
    }
}

bool LDPartsDirs::loadLDrawSearchDirs(const char *filename) //send default arbitrary file name
{
  bool retValue = false;

  if (!initLDrawSearchDirs()) {
      qDebug() << "Could not initialize LDrawini ";
      return retValue;
    }

  if (sm_lDrawIni)
    {
      LDrawIniComputeRealDirs(sm_lDrawIni, 1, 0, filename);
    }

  if (!strlen(lDrawDir()))
    {
      return retValue;
    }
  // end initialize

  //search dirs from ldraw.ini
  if (sm_lDrawIni && sm_lDrawIni->nSearchDirs > 0)
    {
      int i;

      qDebug() << "";

      for (i = 0; i < sm_lDrawIni->nSearchDirs; i++)
        {
          LDrawSearchDirS *searchDir = &sm_lDrawIni->SearchDirs[i];

          if ((searchDir->Flags & LDSDF_SKIP) == 0)  //Add function to verifyExcludedDirs()
            {
              m_ldrawSearchDirs.push_back(searchDir->Dir);
              logInfo() << "LDRAW SEARCH DIR PUSHED: " << searchDir->Dir;
            }
        }

      qDebug() << "";

      //process extra seach directories (NOT USED)
      bool found = false;
      for (StringList::const_iterator it = m_extraSearchDirs.begin(); !found &&
           it != m_extraSearchDirs.end(); it++)
        {
          const char *dir = it->c_str();

          if (verifyExtraDir(dir))
            {
              m_ldrawSearchDirs.push_back(dir);
              qDebug() << "LDRAW EXTRA SEARCH DIR PUSHED: " << dir;
              found = true;
            }
        }
      retValue = chdir(Preferences::lpub3dPath.toLatin1().constData()) == 0;
    }


  // The ancestor map has done its job; may as well free up the memory it
  // was using.
  m_ancestorMap.clear();
  return retValue;
}
