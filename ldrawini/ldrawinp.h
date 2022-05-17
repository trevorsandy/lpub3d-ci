/* LDrawInP.h  Include file for LDrawIni.c private data and functions.
You can use this file if you really need to access the private data
or if you find some of the internal functions useful.                        */

/* Copyright (C) 2004-2008  Lars C. Hassing (SP.lars@AM.hassings.dk)
If you make any changes to this file, please contact me, and I'll probably
adopt the changes for the benefit of other users.                            */

/*****************************************************************************
040319 lch Added LDrawInP.h (Really LDrawIniP.h but 8+3 name...)
040513 lch Added LDrawIniParseSymbolicSearchDir
050527 lch Added defines LDRAWINI_BEGIN_STDC/LDRAWINI_END_STDC for extern "C"
080412 lch Added LDrawIniSetFileCaseCallback from Travis Cobbs
080910 lch Try typical locations for LDrawDir and see if they have P and PARTS
160801 tds Added "ldraw\unofficial\" to GetDefaultLDrawSearch (for LPub3D)
******************************************************************************/

#ifndef LDRAWINP_INCLUDED
#define LDRAWINP_INCLUDED

/* Hide curly brackets {} in defines to avoid Beautifier indenting whole file */
#ifdef __cplusplus
#define LDRAWINI_BEGIN_STDC extern "C" {
#define LDRAWINI_END_STDC   }
#else
#define LDRAWINI_BEGIN_STDC
#define LDRAWINI_END_STDC
#endif

LDRAWINI_BEGIN_STDC

struct LDrawIniPrivateDataS
{
   /* The LDrawSearch directories as read */
   int            nSymbolicSearchDirs;
   char         **SymbolicSearchDirs;
};

/* Returns 1 if OK, 0 if Section/Key not found or error */
int LDrawIniReadIniFile(const char *IniFile,
                        const char *Section, const char *Key,
                        char *Str, int sizeofStr);

/*
Parse SymbolicSearchDir into Result
Returns 1 if OK, 0 on error.
*/
int LDrawIniParseSymbolicSearchDir(struct LDrawSearchDirS * Result,
                                   const char *SymbolicSearchDir,
                                   const char *LDrawDir,
                                   const char *ModelDir,
                                   const char *HomeDir);

LDRAWINI_END_STDC

#endif
