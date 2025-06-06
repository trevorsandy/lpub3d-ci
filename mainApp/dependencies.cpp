 
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

#include "dependencies.h"
#include "lpub.h"

bool isMpd()
{
  return lpub->ldrawFile.isMpd();
}

bool isOlder(const QString &fileName,
  const QDateTime   &lastModified)
{
  return lpub->ldrawFile.older(fileName,lastModified);
}

bool isOlder(
  const QStringList &parsedStack,
  const QDateTime   &lastModified)
{
  return lpub->ldrawFile.older(parsedStack,lastModified);
}

bool modified(
  const QStringList &parsedStack, bool reset)
{
  return lpub->ldrawFile.modified(parsedStack, reset);
}

bool modified(const QVector<int> &parsedIndexes, bool reset)
{
  return lpub->ldrawFile.modified(parsedIndexes, reset);
}
