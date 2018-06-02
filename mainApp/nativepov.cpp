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

#include <QtWidgets>

#include "nativepov.h"
#include "lpub_preferences.h"
#include "lpub_messages.h"
#include "lpub_strings.h"
#include "version.h"
#include "paths.h"

#include "lc_file.h"
#include "pieceinf.h"
#include "view.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent>
#endif

NativePov nativePov;

bool NativePov::CreateNativePovFile(const NativeOptions& Options)
{
          QString Type = Options.ImageType == Render::CSI ? "CSI" : "PLI";

//          loadPovSettings();

          // Options
//          m_InputFileName   = Options.InputFileName;
//          m_OutputFileName  = Options.OutputFileName;
//          m_PovGenCommand   = Options.PovGenCommand;
//          m_ImageType       = Options.ImageType;
//          m_height          = Options.ImageWidth;
//          m_width           = Options.ImageHeight;
//          m_Latitude        = Options.Latitude;
//          m_Longitude       = Options.Longitude;
//          m_CameraDistance  = Options.CameraDistance;
//          m_TransBackground = Options.TransBackground;
//          m_Orthographic    = Options.Orthographic;
  return true;
}
