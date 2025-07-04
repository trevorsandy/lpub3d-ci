 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PATHS_H
#define PATHS_H

#include <QString>
#include <QStringList>

class Paths {
public:
    static void mkDirs();
    static void mkPovrayDir();
    static void mkBlenderDir();
    static void mkCustomDirs();
    static QString lpubDir;
    static QString tmpDir;
    static QString assemDir;
    static QString partsDir;
    static QString bomDir;
    static QString submodelDir;
    static QString logsDir;
    static QString htmlStepsDir;
    static QString extrasDir;
    static QString libraryDir;
    static QString customDir;
    static QString customPartDir;
    static QString customSubDir;
    static QString customTextureDir;
    static QString customPrimDir;
    static QString customPrim8Dir;
    static QString customPrim48Dir;
    static QString povrayRenderDir;
    static QString blenderRenderDir;
    static QStringList customDirs;
};

extern class Paths paths;

#endif
