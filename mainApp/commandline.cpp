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

#include "application.h"
#include "lpub.h"

int Gui::processCommandLine()
{
  // Declarations
  bool processExport = false;
  bool processFile   = false;
  QString pageRange,exportOption,commandlineFile,preferredRenderer;

  #// Process parameters
  QStringList Arguments = Application::instance()->arguments();

  const int NumArguments = Arguments.size();
  for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
    {
      const QString& Param = Arguments[ArgIdx];

      if (Param[0] != '-')
        {
          commandlineFile = Param;
          continue;
        }

      auto ParseString = [&ArgIdx, &Arguments, NumArguments](QString& Value, bool Required)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
            {
              ArgIdx++;
              Value = Arguments[ArgIdx];
            }
          else if (Required)
            printf("Not enough parameters for the '%s' argument.\n", Arguments[ArgIdx].toLatin1().constData());
        };

      if (Param == QLatin1String("-pf") || Param == QLatin1String("--process-file"))
        processFile = true;
      else if (Param == QLatin1String("-pe") || Param == QLatin1String("--process-export"))
        processExport = true;
      else if (Param == QLatin1String("-x") || Param == QLatin1String("--clear-cache"))
        resetCache = true;
      else if (Param == QLatin1String("-p") || Param == QLatin1String("--preferred-renderer"))
        ParseString(preferredRenderer, false);
      else if (Param == QLatin1String("-o") || Param == QLatin1String("--export-option"))
        ParseString(exportOption, false);
      else if (Param == QLatin1String("-f") || Param == QLatin1String("--pdf-output-file"))
        ParseString(saveFileName, false);
      else if (Param == QLatin1String("-d") || Param == QLatin1String("--image-output-directory"))
        ParseString(saveFileName, false);
      else if (Param == QLatin1String("-r") || Param == QLatin1String("--range"))
        ParseString(pageRange, false);
      else
        emit messageSig(true,QString("Unknown commandline parameter: '%1'").arg(Param));
    }

  if (!preferredRenderer.isEmpty()){
      //QSettings Settings;
      QString renderer = Preferences::preferredRenderer;
      if (preferredRenderer.toLower() == "ldview"){
          renderer = RENDERER_LDVIEW;
      }
      else
      if (preferredRenderer.toLower() == "ldview-sc"){
          renderer = RENDERER_LDVIEW;
          Preferences::useLDViewSingleCall = true;
          //Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"),Preferences::useLDViewSingleCall);
      }
      else
      if (preferredRenderer.toLower() == "ldglite"){
          renderer = RENDERER_LDGLITE;
      }
      else
      if (preferredRenderer.toLower() == "povray"){
          renderer = RENDERER_POVRAY;
      }
      else {
          emit messageSig(true,QString("Invalid renderer specified: '%1'").arg(renderer));
          return 1;
      }
      if (Preferences::preferredRenderer != renderer) {
          Preferences::preferredRenderer = renderer;
          Render::setRenderer(Preferences::preferredRenderer);
          //Settings.setValue(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"),Preferences::preferredRenderer);

          QString message = QString("Renderer is %1 %2").arg(renderer).arg(Preferences::useLDViewSingleCall ? "(Single Call)" : "");
          logInfo() << message;
          emit messageSig(true,message);
      }
    }

  if (!loadFile(commandlineFile))
      return 1;

  if (processPageRange(pageRange)) {
      if (processFile){
          Preferences::pageDisplayPause = 1;
          continuousPageDialog(PAGE_NEXT);
        } else if (processExport) {
          if (exportOption == "pdf")
            exportAsPdfDialog();
          else
          if (exportOption == "png")
            exportAsPngDialog();
          else
          if (exportOption == "jpg")
            exportAsJpgDialog();
          else
          if (exportOption == "bmp")
            exportAsBmpDialog();
          else
            exportAsPdfDialog();
        } else {
          continuousPageDialog(PAGE_NEXT);
        }
    } else
       return 1;
  return 0;
}
