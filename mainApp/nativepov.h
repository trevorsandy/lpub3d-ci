/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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

#ifndef NATIVEPOV_H
#define NATIVEPOV_H

#include <string.h>

#include "render.h"

class Meta;
class NativeOptions;
class lcDiskFile;

class NativePov
{
public:
  NativePov() {}
  ~NativePov(){}
  bool             CreateNativePovFile(const NativeOptions &);

protected:

  lcDiskFile*      m_pPOVFile;
  char             m_Line[1024];

  friend class Project;
};

#endif // NATIVEPOV_H
