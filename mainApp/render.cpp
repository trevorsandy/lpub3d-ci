
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * This class encapsulates the external renderers.  For now, this means
 * only ldglite.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QtWidgets>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "lpub.h"
#include "render.h"
#include "resolution.h"
#include "meta.h"
#include "math.h"
#include "lpub_preferences.h"

#include "paths.h"

//**3D Viewer
#include "lc_global.h"
#include "lc_mainwindow.h"
#include "lc_file.h"
#include "project.h"
#include "pieceinf.h"
#include "view.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent>
#endif
//**

#ifdef Q_OS_WIN
#include <windows.h>
#endif

Render *renderer;

LDGLite ldglite;
LDView  ldview;
POVRay  povray;
Native  native;


//#define LduDistance 5729.57
#define CA "-ca0.01"
#define USE_ALPHA "+UA"

static double pi = 4*atan(1.0);
// the default camera distance for real size
static float LduDistance = 10.0/tan(0.005*pi/180);

// renderer timeout in milliseconds
static int rendererTimeout(){
    if (Preferences::rendererTimeout == -1)
        return -1;
    else
        return Preferences::rendererTimeout*60*1000;
}

QString fixupDirname(const QString &dirNameIn) {
#ifdef Q_OS_WIN
    long     length = 0;
    TCHAR*   buffer = nullptr;
//  30/11/2014 Generating "invalid conversion from const ushort to const wchar" compile error:
//  LPCWSTR dirNameWin = dirNameIn.utf16();
    LPCWSTR dirNameWin = reinterpret_cast<LPCWSTR>(dirNameIn.utf16());

// First obtain the size needed by passing NULL and 0.

    length = GetShortPathName(dirNameWin, nullptr, 0);
    if (length == 0){
                QString message = QString("Couldn't get length of short path name length, lastError is %1, trying long path name").arg(GetLastError());
#ifdef QT_DEBUG_MODE
                qDebug() << message << "\n";
#else
                emit gui->messageSig(LOG_STATUS, message);
#endif
                return dirNameIn;
     }
// Dynamically allocate the correct size
// (terminating null char was included in length)

    buffer = new TCHAR[length];

// Now simply call again using same long path.

    length = GetShortPathName(dirNameWin, buffer, length);
    if (length == 0){
                QString message = QString("Couldn't get length of short path name length, lastError is %1, trying long path name").arg(GetLastError());
#ifdef QT_DEBUG_MODE
                qDebug() << message << "\n";
#else
                emit gui->messageSig(LOG_STATUS, message);
#endif
		return dirNameIn;
	}

	QString dirNameOut = QString::fromWCharArray(buffer);

    delete [] buffer;
        return dirNameOut;
#else
        return dirNameIn;
#endif
}

QString const Render::getRenderer()
{
  if (renderer == &ldglite)
  {
    return RENDERER_LDGLITE;
  }
  else
  if (renderer == &ldview)
  {
    return RENDERER_LDVIEW;
  }
  else
  if (renderer == &povray)
  {
    return RENDERER_POVRAY;
  }
  else
  {
    return RENDERER_NATIVE;
  }
}

void Render::setRenderer(QString const &name)
{
  if (name == RENDERER_LDGLITE)
  {
    renderer = &ldglite;
  }
  else
  if (name == RENDERER_LDVIEW)
  {
    renderer = &ldview;
  }
  else
  if (name == RENDERER_POVRAY)
  {
    renderer = &povray;
  }
  else
  {
    renderer = &native;
  }
}

const QString Render::getRotstepMeta(RotStepMeta &rotStep){
  QString rotstepMeta = QString("0 // ROTSTEP %1 %2 %3 %4")
                                .arg(rotStep.value().type)
                                .arg(rotStep.value().rots[0])
                                .arg(rotStep.value().rots[1])
                                .arg(rotStep.value().rots[2]);
  return rotstepMeta;
}

bool Render::useLDViewSCall(bool override){
  if (override)
    return override;
  else
    return Preferences::useLDViewSingleCall;
}

void clipImage(QString const &pngName) {

    QImage toClip(QDir::toNativeSeparators(pngName));
    QRect clipBox;

    int minX = toClip.width(); int maxX = 0;
    int minY = toClip.height();int maxY = 0;

    for(int x=0; x < toClip.width(); x++)
        for(int y=0; y < toClip.height(); y++)
            if (qAlpha(toClip.pixel(x, y)))
            {
                minX = qMin(x, minX);
                minY = qMin(y, minY);
                maxX = qMax(x, maxX);
                maxY = qMax(y, maxY);
            }

    if (minX > maxX || minY > maxY) {
        emit gui->messageSig(LOG_STATUS, qPrintable("No opaque content in " + pngName));
        return;
    } else {
        clipBox.setCoords(minX, minY, maxX, maxY);
    }

    //save clipBox;
    QImage clipped = toClip.copy(clipBox);
    QString clipMsg = QString("%1 (w:%2 x h:%3)")
                              .arg(pngName)
                              .arg(clipped.width())
                              .arg(clipped.height());

    if (clipped.save(QDir::toNativeSeparators(pngName))) {
        emit gui->messageSig(LOG_STATUS, qPrintable("Clipped " + clipMsg));
    } else {
        emit gui->messageSig(LOG_ERROR, qPrintable("Failed to save clipped image " + clipMsg));
    }
 }

// Shared calculations
float stdCameraDistance(Meta &meta, float scale) {
	float onexone;
	float factor;

	// Do the math in pixels

	onexone  = 20*meta.LPub.resolution.ldu(); // size of 1x1 in units
	onexone *= meta.LPub.resolution.value();  // size of 1x1 in pixels
	onexone *= scale;
	factor   = gui->pageSize(meta.LPub.page, 0)/onexone; // in pixels;

//	logDebug() << qPrintable(QString("LduDistance                      : %1").arg(LduDistance));
//	logDebug() << qPrintable(QString("Page Size (width in pixels)      : %1").arg(gui->pageSize(meta.LPub.page, 0)));
//	logDebug() << qPrintable(QString("Resolution Ldu                   : %1").arg(QString::number(meta.LPub.resolution.ldu(), 'f' ,10)));
//	logDebug() << qPrintable(QString("Resolution pixel                 : %1").arg(meta.LPub.resolution.value()));
//	logDebug() << qPrintable(QString("Scale                            : %1").arg(scale));
//	logDebug() << qPrintable(QString("1x1 [20*res.ldu*res.pix*scale]   : %1").arg(QString::number(onexone, 'f' ,10)));
//	logDebug() << qPrintable(QString("Factor [Page size/OnexOne]       : %1").arg(QString::number(factor, 'f' ,10)));
//	logDebug() << qPrintable(QString("Cam Distance [Factor*LduDistance]: %1").arg(QString::number(factor*LduDistance, 'f' ,10)));

	return factor*LduDistance;
}



/***************************************************************************
 *
 * The math for zoom factor.  1.0 is true size.
 *
 * 1 LDU is 1/64 of an inch
 *
 * LDGLite produces 72 DPI
 *
 * Camera angle is 0.01
 *
 * What distance do we need to put the camera, given a user chosen DPI,
 * to get zoom factor of 1.0?
 *
 **************************************************************************/


/***************************************************************************
 *
 * POVRay renderer
 *
 **************************************************************************/

float POVRay::cameraDistance(
    Meta &meta,
    float scale)
{
  if (getRenderer() == RENDERER_LDVIEW)
    return stdCameraDistance(meta, scale)*0.455;
  else
    return stdCameraDistance(meta,scale);
}

int POVRay::renderCsi(
    const QString     &addLine,
    const QStringList &csiParts,
    const QStringList &csiKeys,
    const QString     &pngName,
    Meta              &meta)
{

  /* Create the CSI DAT file */
  QString ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
  QString povName = ldrName + ".pov";
  QStringList list;
  QString message;

  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  QStringList povArguments;
  if (Preferences::povrayDisplay){
      povArguments << QString("+d");
  } else {
      povArguments << QString("-d");
  }

  QString O = QString("+O\"%1\"").arg(QDir::toNativeSeparators(pngName));
  QString I = QString("+I\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(povName)));
  QString W = QString("+W%1").arg(width);
  QString H = QString("+H%1").arg(height);

  povArguments << I;
  povArguments << O;
  povArguments << W;
  povArguments << H;
  povArguments << USE_ALPHA;

  list = meta.LPub.assem.povrayParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
      if (list[i] != "" && list[i] != " ") {
          povArguments << list[i];
          //logInfo() << qPrintable("-PARM META: " + list[i]);
      }
  }

  bool hasSTL       = Preferences::lgeoStlLib;
  bool hasLGEO      = Preferences::lgeoPath != "";
  bool hasPOVRayIni = Preferences::povrayIniPath != "";
  bool hasPOVRayInc = Preferences::povrayIncPath != "";

  if(hasPOVRayInc){
      QString povinc = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::povrayIncPath)));
      povArguments << povinc;
  }
  if(hasPOVRayIni){
      QString povini = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::povrayIniPath)));
      povArguments << povini;
  }
  if(hasLGEO){
      QString lgeoLg = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/lg")));
      QString lgeoAr = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/ar")));
      povArguments << lgeoLg;
      povArguments << lgeoAr;
      if (hasSTL){
          QString lgeoStl = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/stl")));
          povArguments << lgeoStl;
        }
    }

//#ifndef __APPLE__
//  povArguments << "/EXIT";
//#endif

  // LDView block begin
  if (Preferences::povGenRenderer == RENDERER_LDVIEW) {
      int rc;
      if ((rc = rotateParts(addLine,meta.rotStep, csiParts, ldrName, QString())) < 0) {
          return rc;
        }

      /* determine camera distance */
      int cd = cameraDistance(meta,meta.LPub.assem.modelScale.value())*1700/1000;

      bool hasLDViewIni = Preferences::ldviewPOVIni != "";

      //QString cg = QString("-cg0.0,0.0,%1") .arg(cd);
      QString cg = QString("-cg%1,%2,%3")
          .arg(meta.LPub.assem.angle.value(0))
          .arg(meta.LPub.assem.angle.value(1))
          .arg(cd);

      QString w  = QString("-SaveWidth=%1") .arg(width);
      QString h  = QString("-SaveHeight=%1") .arg(height);
      QString f  = QString("-ExportFile=%1") .arg(povName);  // -ExportSuffix not required
      QString l  = QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawPath)));
      QString o  = QString("-HaveStdOut=1");
      QString v  = QString("-vv");

      QStringList arguments;
      arguments << CA;
      arguments << cg;
      arguments << w;
      arguments << h;
      arguments << f;
      arguments << l;
      arguments << o;
      arguments << v;

      if (Preferences::enableFadeSteps)
        arguments <<  QString("-SaveZMap=1");

      list = meta.LPub.assem.ldviewParms.value().split(' ');
      for (int i = 0; i < list.size(); i++) {
          if (list[i] != "" && list[i] != " ") {
              arguments << list[i];
              //logInfo() << qPrintable("-PARM META: " + list[i]);
            }
        }
      if(hasLDViewIni){
          QString ini  = QString("-IniFile=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldviewPOVIni)));
          arguments << ini;
        }
      if (!Preferences::altLDConfigPath.isEmpty()) {
          arguments << "-LDConfig=" + Preferences::altLDConfigPath;
          //logDebug() << qPrintable("-LDConfig=" + Preferences::altLDConfigPath);
        }

      arguments << ldrName;

      emit gui->messageSig(LOG_STATUS, "POVRay render CSI...");

      QProcess    ldview;
      ldview.setEnvironment(QProcess::systemEnvironment());
      ldview.setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir);
      ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldviewpov");
      ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldviewpov");

      message = QString("LDView POV file generate CSI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qPrintable(message);
#else
      emit gui->messageSig(LOG_STATUS, message);
#endif

      ldview.start(Preferences::ldviewExe,arguments);
      if ( ! ldview.waitForFinished(rendererTimeout())) {
          if (ldview.exitCode() != 0 || 1) {
              QByteArray status = ldview.readAll();
              QString str;
              str.append(status);
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView POV file generate failed with exit code %1\n%2") .arg(ldview.exitCode()) .arg(str));
              return -1;
            }
        }
    }
  else
  // Native block begin
  if (Preferences::povGenRenderer == RENDERER_NATIVE) {
       // Renderer options
       NativeOptions Options;
       Options.ImageType         = CSI;
       Options.PovFileName       = povName;
       Options.ImageWidth        = width;
       Options.ImageHeight       = height;
       Options.Latitude          = meta.LPub.assem.angle.value(0);
       Options.Longitude         = meta.LPub.assem.angle.value(1);
       Options.HighlightNewParts = gui->suppressColourMeta(); //Preferences::enableHighlightStep;
       Options.CameraDistance    = -cameraDistance(meta,meta.LPub.assem.modelScale.value())/11659;
       Options.PovGenCommand     = QString("%1 %2").arg(Preferences::povrayExe).arg(povArguments.join(" "));;

       // Set and load new project
       Project* PovGenCsiProject = new Project();

       if (PovGenCsiProject->Load(ldrName))
       {
         gApplication->SetProject(PovGenCsiProject);
         gMainWindow->UpdateAllViews();
       }
       else
       {
         emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not load Native POV CSI ldr file."));
         delete PovGenCsiProject;
         return -1;
       }

       // Generate pov file
       if (!CreateNativePovFile(PovGenCsiProject, Options))
       {
         emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not export Native POV CSI file."));
         delete PovGenCsiProject;
         return -1;
       }
    }

  emit gui->messageSig(LOG_STATUS, "Executing POVRay render CSI - please wait...");

  QProcess povray;
  QStringList povEnv = QProcess::systemEnvironment();
  povray.setEnvironment(povEnv);
  povray.setWorkingDirectory(QDir::currentPath()+ "/" + Paths::assemDir); // pov win console app will not write to dir different from cwd or source file dir
  povray.setStandardErrorFile(QDir::currentPath() + "/stderr-povray");
  povray.setStandardOutputFile(QDir::currentPath() + "/stdout-povray");

  message = QString("POVRay CSI Arguments: %1 %2").arg(Preferences::povrayExe).arg(povArguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  povray.start(Preferences::povrayExe,povArguments);
  if ( ! povray.waitForFinished(rendererTimeout())) {
      if (povray.exitCode() != 0) {
          QByteArray status = povray.readAll();
          QString str;
          str.append(status);
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("POVRay CSI render failed with code %1\n%2").arg(povray.exitCode()) .arg(str));
          return -1;
        }
    }

  // image matting stub
  if (Preferences::enableFadeSteps) {
      QString previousPngFile = imageMatting.previousStepCSIImage(csiKeys.first());
      if (!previousPngFile.isEmpty()) { // first entry returns "" so check first
          //logDebug() << qPrintable(QString("Previous CSI pngFile: %1").arg(previousPngFile));
      }
  }

  clipImage(pngName);

  return 0;

}

int POVRay::renderPli(
    const QStringList &ldrNames ,
    const QString     &pngName,
    Meta    	      &meta,
    bool     	      bom)
{
  QString povName = ldrNames.first() +".pov";  
  PliMeta &metaType = bom ? meta.LPub.bom : meta.LPub.pli;
  QStringList list;
  QString message;

  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  bool hasSTL       = Preferences::lgeoStlLib;
  bool hasLGEO      = Preferences::lgeoPath != "";
  bool hasPOVRayIni = Preferences::povrayIniPath != "";
  bool hasPOVRayInc = Preferences::povrayIncPath != "";

  // LDView block begin
  if (Preferences::povGenRenderer == RENDERER_LDVIEW) {
      /* determine camera distance */
      int cd = cameraDistance(meta,metaType.modelScale.value())*1700/1000;

      bool hasLDViewIni = Preferences::ldviewPOVIni != "";

      //qDebug() << "LDView (Native) Camera Distance: " << cd;

      QString cg = QString("-cg%1,%2,%3") .arg(metaType.angle.value(0))
          .arg(metaType.angle.value(1))
          .arg(cd);

      QString w  = QString("-SaveWidth=%1")  .arg(width);
      QString h  = QString("-SaveHeight=%1") .arg(height);
      QString f  = QString("-ExportFile=%1") .arg(povName);  // -ExportSuffix not required
      QString l  = QString("-LDrawDir=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldrawPath)));
      QString o  = QString("-HaveStdOut=1");
      QString v  = QString("-vv");

      QStringList arguments;
      arguments << CA;
      arguments << cg;
      arguments << w;
      arguments << h;
      arguments << f;
      arguments << l;
      arguments << o;
      arguments << v;

      list = meta.LPub.pli.ldviewParms.value().split(' ');
      for (int i = 0; i < list.size(); i++) {
          if (list[i] != "" && list[i] != " ") {
              arguments << list[i];
              //logInfo() << qPrintable("-PARM META: " + list[i]);
            }
        }
      if(hasLDViewIni){
          QString ini  = QString("-IniFile=%1") .arg(fixupDirname(QDir::toNativeSeparators(Preferences::ldviewPOVIni)));
          arguments << ini;
        }
      if (!Preferences::altLDConfigPath.isEmpty()) {
          arguments << "-LDConfig=" + Preferences::altLDConfigPath;
          //logDebug() << qPrintable("-LDConfig=" + Preferences::altLDConfigPath);
        }

      arguments << ldrNames.first();

      emit gui->messageSig(LOG_STATUS, "POVRay render PLI...");

      QProcess    ldview;
      ldview.setEnvironment(QProcess::systemEnvironment());
      ldview.setWorkingDirectory(QDir::currentPath());
      ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldviewpov");
      ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldviewpov");

      message = QString("POVRay (LDView POV file generate PLI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
      qDebug() << qPrintable(message);
#else
      emit gui->messageSig(LOG_STATUS, message);
#endif

      ldview.start(Preferences::ldviewExe,arguments);
      if ( ! ldview.waitForFinished()) {
          if (ldview.exitCode() != 0) {
              QByteArray status = ldview.readAll();
              QString str;
              str.append(status);
              emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView POV file generate failed with exit code %1\n%2") .arg(ldview.exitCode()) .arg(str));
              return -1;
            }
        }
    }
  else
  // Native block begin
  if (Preferences::povGenRenderer == RENDERER_NATIVE) {
      // Renderer options
      NativeOptions Options;
      Options.ImageType         = PLI;
      Options.PovFileName       = povName;
      Options.ImageWidth        = width;
      Options.ImageHeight       = height;
      Options.Latitude          = metaType.angle.value(0);
      Options.Longitude         = -metaType.angle.value(1);                                   // switch from -45
      Options.CameraDistance    = -cameraDistance(meta,metaType.modelScale.value())/11659;    // use assembly setting

      // Set and load new project
      Project* PovGenPliProject = new Project();

      if (PovGenPliProject->Load(ldrNames.first()))
      {
        gApplication->SetProject(PovGenPliProject);
        gMainWindow->UpdateAllViews();
      }
      else
      {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not create Native POV PLI file generate project."));
        delete PovGenPliProject;
        return -1;
      }

      // Generate pov file
      if (!CreateNativePovFile(PovGenPliProject, Options))
      {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not export Native POV PLI file."));
        delete PovGenPliProject;
        return -1;
      }
  }

  QStringList povArguments;
  if (Preferences::povrayDisplay){
      povArguments << QString("+d");
  } else {
      povArguments << QString("-d");
  }

  QString O = QString("+O\"%1\"").arg(QDir::toNativeSeparators(pngName));
  QString I = QString("+I\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(povName)));
  QString W = QString("+W%1").arg(width);
  QString H = QString("+H%1").arg(height);

  povArguments << I;
  povArguments << O;
  povArguments << W;
  povArguments << H;
  povArguments << USE_ALPHA;

  list = meta.LPub.assem.povrayParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
      if (list[i] != "" && list[i] != " ") {
          povArguments << list[i];
          //logInfo() << qPrintable("-PARM META: " + list[i]);
      }
  }
  if(hasPOVRayInc){
      QString povinc = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::povrayIncPath)));
      povArguments << povinc;
  }
  if(hasPOVRayIni){
      QString povini = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::povrayIniPath)));
      povArguments << povini;
  }
  if(hasLGEO){
      QString lgeoLg = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/lg")));
      QString lgeoAr = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/ar")));
      povArguments << lgeoLg;
      povArguments << lgeoAr;
      if (hasSTL){
          QString lgeoStl = QString("+L\"%1\"").arg(fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/stl")));
          povArguments << lgeoStl;
        }
    }

//#ifndef __APPLE__
//  povArguments << "/EXIT";
//#endif

  emit gui->messageSig(LOG_STATUS, "Executing POVRay render PLI - please wait...");

  QProcess povray;
  QStringList povEnv = QProcess::systemEnvironment();
  povray.setEnvironment(povEnv);
  povray.setWorkingDirectory(QDir::currentPath()+ "/" + Paths::partsDir); // pov win console app will not write to dir different from cwd or source file dir
  povray.setStandardErrorFile(QDir::currentPath() + "/stderr-povray");
  povray.setStandardOutputFile(QDir::currentPath() + "/stdout-povray");

  message = QString("POVRay PLI Arguments: %1 %2").arg(Preferences::povrayExe).arg(povArguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  povray.start(Preferences::povrayExe,povArguments);
  if ( ! povray.waitForFinished(rendererTimeout())) {
      if (povray.exitCode() != 0) {
          QByteArray status = povray.readAll();
          QString str;
          str.append(status);
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("POVRay PLI render failed with code %1\n%2") .arg(povray.exitCode()) .arg(str));
          return -1;
        }
    }

  clipImage(pngName);

  return 0;
}


/***************************************************************************
 *
 * LDGLite renderer
 *
 **************************************************************************/

float LDGLite::cameraDistance(
  Meta &meta,
  float scale)
{
	return stdCameraDistance(meta,scale);
}

int LDGLite::renderCsi(
  const QString     &addLine,
  const QStringList &csiParts,
  const QStringList &csiKeys,
  const QString     &pngName,
        Meta        &meta)
{
  /* Create the CSI DAT file */
  QString ldrPath, ldrName, ldrFile;
  int rc;
  ldrName = "csi.ldr";
  ldrPath = QDir::currentPath() + "/" + Paths::tmpDir;
  ldrFile = ldrPath + "/" + ldrName;
  if ((rc = rotateParts(addLine,meta.rotStep, csiParts, ldrFile,QString())) < 0) {
     return rc;
  }

  /* determine camera distance */

  int cd = cameraDistance(meta,meta.LPub.assem.modelScale.value());

  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  QString v  = QString("-v%1,%2")   .arg(width)
                                    .arg(height);
  QString o  = QString("-o0,-%1")   .arg(height/6);
  QString mf = QString("-mF%1")     .arg(pngName);

  int lineThickness = resolution()/150+0.5;
  if (lineThickness == 0) {
    lineThickness = 1;
  }

  QString w  = QString("-W%1")      .arg(lineThickness); // ldglite always deals in 72 DPI

  //QString cg = QString("-cg0.0,0.0,%1") .arg(cd);
  QString cg = QString("-cg%1,%2,%3") .arg(meta.LPub.assem.angle.value(0))
                                      .arg(meta.LPub.assem.angle.value(1))
                                      .arg(cd);

  QStringList arguments;
  arguments << CA;                  // camera FOV angle in degrees
  arguments << cg;                  // camera globe - scale factor
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the center X across and Y down
  arguments << w;                   // line thickness

  QStringList list;
  // First, load parms from meta
  list = meta.LPub.assem.ldgliteParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
     if (list[i] != "" && list[i] != " ") {
         arguments << list[i];
         //logInfo() << qPrintable("-PARM META: " + list[i]);
      }
  }
  // Add ini parms if not already added from meta
  for (int i = 0; i < Preferences::ldgliteParms.size(); i++) {
      if (list.indexOf(QRegExp("^" + QRegExp::escape(Preferences::ldgliteParms[i]))) < 0) {
        arguments << Preferences::ldgliteParms[i];
        //logInfo() << qPrintable("-PARM INI : " + Preferences::ldgliteParms[i]);
      }
  }

  // add custom colour file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-ldcF" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("=" + Preferences::altLDConfigPath);
  }

  arguments << mf;                  // .png file name
  arguments << ldrFile;             // csi.ldr (input file)

  emit gui->messageSig(LOG_STATUS, "Executing LDGLite render CSI - please wait...");

  QProcess    ldglite;
  QStringList env = QProcess::systemEnvironment();
  env << "LDRAWDIR=" + Preferences::ldrawPath;
  //logDebug() << qPrintable("LDRAWDIR=" + Preferences::ldrawPath);

  if (!Preferences::ldgliteSearchDirs.isEmpty()) {
    env << "LDSEARCHDIRS=" + Preferences::ldgliteSearchDirs;
    //logDebug() << qPrintable("LDSEARCHDIRS: " + Preferences::ldgliteSearchDirs);
  }

  ldglite.setEnvironment(env);
  //logDebug() << qPrintable("ENV: " + env);

  ldglite.setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir);
  ldglite.setStandardErrorFile(QDir::currentPath() + "/stderr-ldglite");
  ldglite.setStandardOutputFile(QDir::currentPath() + "/stdout-ldglite");

  QString message = QString("LDGLite CSI Arguments: %1 %2").arg(Preferences::ldgliteExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  ldglite.start(Preferences::ldgliteExe,arguments);
  if ( ! ldglite.waitForFinished(rendererTimeout())) {
    if (ldglite.exitCode() != 0) {
      QByteArray status = ldglite.readAll();
      QString str;
      str.append(status);
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDGlite failed\n%1") .arg(str));
      return -1;
    }
  }

  // image matting stub
  if (Preferences::enableFadeSteps) {
      QString previousPngFile = imageMatting.previousStepCSIImage(csiKeys.first());
      if (!previousPngFile.isEmpty()) { // first entry returns "" so check first
          //logDebug() << qPrintable(QString("Previous CSI pngFile: %1").arg(previousPngFile));
      }
  }

  return 0;
}


int LDGLite::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  bool               bom)
{
  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  int lineThickness = resolution()/72.0+0.5;

  /* determine camera distance */

  PliMeta &metaType = bom ? meta.LPub.bom : meta.LPub.pli;

  int cd = cameraDistance(meta,metaType.modelScale.value());

  QString cg = QString("-cg%1,%2,%3") .arg(metaType.angle.value(0))
                                      .arg(metaType.angle.value(1))
                                      .arg(cd);

  QString v  = QString("-v%1,%2")   .arg(width)
                                    .arg(height);
  QString o  = QString("-o0,-%1")   .arg(height/6);
  QString mf = QString("-mF%1")     .arg(pngName);

  QString w  = QString("-W%1")      .arg(lineThickness);  // ldglite always deals in 72 DPI

  QStringList arguments;
  arguments << CA;                  // camera FOV angle in degrees
  arguments << cg;                  // camera globe - scale factor
  arguments << v;                   // display in X wide by Y high window
  arguments << o;                   // changes the center X across and Y down
  arguments << w;                   // line thickness

  QStringList list;
  // First, load parms from meta
  list = meta.LPub.pli.ldgliteParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
     if (list[i] != "" && list[i] != " ") {
         arguments << list[i];
         //logInfo() << qPrintable("-PARM META: " + list[i]);
      }
  }
  // Add ini parms if not already added from meta
  for (int i = 0; i < Preferences::ldgliteParms.size(); i++) {
      if (list.indexOf(QRegExp("^" + QRegExp::escape(Preferences::ldgliteParms[i]))) < 0) {
        arguments << Preferences::ldgliteParms[i];
        //logInfo() << qPrintable("-PARM INI : " + Preferences::ldgliteParms[i]);
      }
  }

  // add custom colour file if exist
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-ldcF" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("=" + Preferences::altLDConfigPath);
  }

  arguments << mf;
  arguments << ldrNames.first();

  emit gui->messageSig(LOG_STATUS, "Executing LDGLite render PLI - please wait...");

  QProcess    ldglite;
  QStringList env = QProcess::systemEnvironment();
  env << "LDRAWDIR=" + Preferences::ldrawPath;
  //logDebug() << qPrintable("LDRAWDIR=" + Preferences::ldrawPath);

  if (!Preferences::ldgliteSearchDirs.isEmpty()){
    env << "LDSEARCHDIRS=" + Preferences::ldgliteSearchDirs;
    //logDebug() << qPrintable("LDSEARCHDIRS: " + Preferences::ldgliteSearchDirs);
  }

  ldglite.setEnvironment(env);
  ldglite.setWorkingDirectory(QDir::currentPath());
  ldglite.setStandardErrorFile(QDir::currentPath() + "/stderr-ldglite");
  ldglite.setStandardOutputFile(QDir::currentPath() + "/stdout-ldglite");

  QString message = QString("LDGLite PLI Arguments: %1 %2").arg(Preferences::ldgliteExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  ldglite.start(Preferences::ldgliteExe,arguments);
  if (! ldglite.waitForFinished()) {
    if (ldglite.exitCode()) {
      QByteArray status = ldglite.readAll();
      QString str;
      str.append(status);
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDGlite failed\n%1") .arg(str));
      return -1;
    }
  }
  return 0;
}


/***************************************************************************
 *
 * LDView renderer
 *                                  6x6                    5990
 *      LDView               LDView    LDGLite       LDView
 * 0.1    8x5     8x6         32x14    40x19  0.25  216x150    276x191  0.28
 * 0.2   14x10   16x10                              430x298    552x381
 * 0.3   20x14   20x15                              644x466    824x571
 * 0.4   28x18   28x19                              859x594   1100x762
 * 0.5   34x22   36x22                             1074x744   1376x949  0.28
 * 0.6   40x27   40x28                             1288x892
 * 0.7   46x31   48x32                            1502x1040
 * 0.8   54x35   56x37
 * 0.9   60x40   60x41
 * 1.0   66x44   68x46       310x135  400x175 0.29
 * 1.1   72x48
 * 1.2   80x53
 * 1.3   86x57
 * 1.4   92x61
 * 1.5   99x66
 * 1.6  106x70
 * 2.0  132x87  132x90       620x270  796x348 0.28
 * 3.0  197x131 200x134      930x404 1169x522
 * 4.0  262x174 268x178     1238x539 1592x697 0.29
 * 5.0  328x217 332x223     1548x673
 *
 *
 **************************************************************************/

float LDView::cameraDistance(
  Meta &meta,
  float scale)
{
	return stdCameraDistance(meta, scale)*0.775;
}

int LDView::renderCsi(
  const QString     &addLine,
  const QStringList &csiParts,
  const QStringList &csiKeys,
  const QString     &pngName,
        Meta        &meta)
{
  /* determine camera distance */
  int cd = cameraDistance(meta,meta.LPub.assem.modelScale.value())*1700/1000;

  /* page size */
  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  /* edge thickness */
  int edgeThickness = 1;

  bool hasLDViewIni = Preferences::ldviewIni != "";

  /* Create the CSI DAT file(s) */
  QString f;
  QStringList ldrNames;
  if (useLDViewSCall()) {
      ldrNames = csiParts;
      QString snapShotList = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
      QFile snapShotListFile(snapShotList);
      if ( ! snapShotListFile.open(QFile::Append | QFile::Text)) {
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView (Single Call) CSI Snapshots list creation failed!"));
          return -1;
      }
      QTextStream out(&snapShotListFile);
      for (int i = 0; i < ldrNames.size(); i++) {
          QString smLine = ldrNames[i];
          out << smLine << endl;
          //logInfo() << qPrintable(QString("CSI Snapshots line: %1").arg(smLine));
      }
      snapShotListFile.close();
      f  = QString("-SaveSnapshotsList=%1").arg(snapShotList);
  } else {
      f  = QString("-SaveSnapShot=%1") .arg(pngName);
      int rc;
     ldrNames << QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
      if ((rc = rotateParts(addLine, meta.rotStep,csiParts,ldrNames.first(),QString())) < 0) {
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView (Single Call) CSI rotate parts failed!"));
          return rc;
      }
  }

// TODO - REMOVE
//  int rc;
//  ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";
//  if ((rc = rotateParts(addLine, meta.rotStep, csiParts, ldrName)) < 0) {
//      return rc;
//    }

//  QString cg = QString("-cg0.0,0.0,%1") .arg(cd);
  QString cg = QString("-cg%1,%2,%3") .arg(meta.LPub.assem.angle.value(0))
                                      .arg(meta.LPub.assem.angle.value(1))
                                      .arg(cd);

  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
//  QString f  = QString("-SaveSnapShot=%1") .arg(pngName); // -SnapshotSuffix not required
  QString l  = QString("-LDrawDir=%1").arg(Preferences::ldrawPath);
  QString o  = QString("-HaveStdOut=1");
  QString e  = QString("-EdgeThickness=%1").arg(edgeThickness);
  QString v  = QString("-vv");

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;
  arguments << o;
  arguments << e;
  arguments << v;

  if (Preferences::enableFadeSteps)
    arguments <<  QString("-SaveZMap=1");

  QStringList list;
  list = meta.LPub.assem.ldviewParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
    if (list[i] != "" && list[i] != " ") {
      arguments << list[i];
      //logInfo() << qPrintable("-PARM META: " + list[i]);
    }
  }
  if(hasLDViewIni){
      QString ini  = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
      arguments << ini;
  }
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-LDConfig=" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("-LDConfig=" + Preferences::altLDConfigPath);
  }
  arguments << ldrNames.first();

  emit gui->messageSig(LOG_STATUS, "Executing LDView render CSI - please wait...");

  QProcess    ldview;
  ldview.setEnvironment(QProcess::systemEnvironment());
  ldview.setWorkingDirectory(QDir::currentPath() + "/" + Paths::tmpDir);
  ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldview");
  ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldview");

  QString message = QString("LDView (Native) CSI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  ldview.start(Preferences::ldviewExe,arguments);
  if ( ! ldview.waitForFinished(rendererTimeout())) {
    if (ldview.exitCode() != 0 || 1) {
      QByteArray status = ldview.readAll();
      QString str;
      str.append(status);
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView CSI render failed with code %1\n%2").arg(ldview.exitCode()) .arg(str));
      return -1;
    }
  }

  // move generated CSI images
  if (useLDViewSCall()){
    QString ldrName;
    QDir dir(QDir::currentPath() + "/" + Paths::tmpDir);
    foreach(ldrName, ldrNames){
        QFileInfo pngFileInfo(ldrName.replace(".ldr",".png"));
        QString pngFilePath = QDir::currentPath() + "/" +
            Paths::assemDir + "/" + pngFileInfo.fileName();
        if (! dir.rename(pngFileInfo.absoluteFilePath(), pngFilePath)){
            QFile pngFile(pngFilePath);
            if (! pngFile.exists()){ // file not found failure
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView CSI image file move failed for\n%1")
                                     .arg(pngFilePath));
                return -1;
              } else {                // file exist failure
                //file exist so delete and retry
                QFile pngFile(pngFilePath);
                if (pngFile.remove()) {
                    //retry
                    if (! dir.rename(pngFileInfo.absoluteFilePath(), pngFilePath)){
                        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView CSI image file move failed after old file removal for\n%1")
                                             .arg(pngFilePath));
                        return -1;
                      }
                  } else {
                    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView could not remove old CSI image file \n%1")
                                         .arg(pngFilePath));
                    return -1;
                  }
              }
          }
      }

    // image mapping stub
    if (Preferences::enableFadeSteps) {
          for (int i = 0; i < csiKeys.size(); i++) {
              QString previousPngFile = imageMatting.previousStepCSIImage(csiKeys[i]);
              if (!previousPngFile.isEmpty()) { // first entry returns "" so check first
                  //logDebug() << qPrintable(QString("Previous CSI pngFile: %1").arg(previousPngFile));
              }
          }
      }
  }
  else
  {
      // image matting stub
      if (Preferences::enableFadeSteps) {
          QString previousPngFile = imageMatting.previousStepCSIImage(csiKeys.first());
          if (!previousPngFile.isEmpty()) { // first entry returns "" so check first
              //logDebug() << qPrintable(QString("Previous CSI pngFile: %1").arg(previousPngFile));
          }
      }
  }

  return 0;
}

int LDView::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  bool              bom)
{
  QFileInfo fileInfo(ldrNames.first());
  if ( ! fileInfo.exists()) {
    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("File ldrNames does not exist!"));
    return -1;
  }

  PliMeta &metaType = bom ? meta.LPub.bom : meta.LPub.pli;

  /* determine camera distance */
  int cd = cameraDistance(meta,metaType.modelScale.value())*1700/1000;

  /* page size */
  int width  = gui->pageSize(meta.LPub.page, 0);
  int height = gui->pageSize(meta.LPub.page, 1);

  /* edge thickness */
  int edgeThickness = 1;

  bool hasLDViewIni = Preferences::ldviewIni != "";

  //qDebug() << "LDView (Native) Camera Distance: " << cd;

  /* Create the CSI DAT file(s) */
  QString f;
  if (useLDViewSCall()) {
      QString snapShotList = QDir::currentPath() + "/" + Paths::tmpDir + "/pli.ldr";
      QFile snapShotListFile(snapShotList);
      if ( ! snapShotListFile.open(QFile::Append | QFile::Text)) {
          emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView (Single Call) PLI Snapshots list creation failed!"));
          return -1;
      }
      QTextStream out(&snapShotListFile);
      for (int i = 0; i < ldrNames.size(); i++) {
          QString smLine = ldrNames[i];
          out << smLine << endl;
          //logInfo() << qPrintable(QString("PLI Snapshots line: %1").arg(smLine));
      }
      snapShotListFile.close();
      f  = QString("-SaveSnapshotsList=%1").arg(snapShotList);
  } else {
      f  = QString("-SaveSnapShot=%1") .arg(pngName);
  }

  QString cg = QString("-cg%1,%2,%3") .arg(metaType.angle.value(0))
                                      .arg(metaType.angle.value(1))
                                      .arg(cd);

  QString w  = QString("-SaveWidth=%1")  .arg(width);
  QString h  = QString("-SaveHeight=%1") .arg(height);
//  QString f  = QString("-SaveSnapShot=%1") .arg(pngName); // -SnapshotSuffix not required
  QString l  = QString("-LDrawDir=%1").arg(Preferences::ldrawPath);
  QString o  = QString("-HaveStdOut=1");
  QString e  = QString("-EdgeThickness=%1").arg(edgeThickness);
  QString v  = QString("-vv");

  QStringList arguments;
  arguments << CA;
  arguments << cg;
  arguments << w;
  arguments << h;
  arguments << f;
  arguments << l;
  arguments << o;
  arguments << e;
  arguments << v;

  QStringList list;
  list = meta.LPub.pli.ldviewParms.value().split(' ');
  for (int i = 0; i < list.size(); i++) {
    if (list[i] != "" && list[i] != " ") {
      arguments << list[i];
      //logInfo() << qPrintable("-PARM META: " + list[i]);
    }
  }
  if(hasLDViewIni){
      QString ini  = QString("-IniFile=%1") .arg(Preferences::ldviewIni);
      arguments << ini;
  }
  if (!Preferences::altLDConfigPath.isEmpty()) {
    arguments << "-LDConfig=" + Preferences::altLDConfigPath;
    //logDebug() << qPrintable("-LDConfig=" + Preferences::altLDConfigPath);
  }
  arguments << ldrNames.first();

  emit gui->messageSig(LOG_STATUS, "Executing LDView render PLI - please wait...");

  QProcess    ldview;
  ldview.setEnvironment(QProcess::systemEnvironment());
  ldview.setWorkingDirectory(QDir::currentPath());
  ldview.setStandardErrorFile(QDir::currentPath() + "/stderr-ldview");
  ldview.setStandardOutputFile(QDir::currentPath() + "/stdout-ldview");

  QString message = QString("LDView (Native) PLI Arguments: %1 %2").arg(Preferences::ldviewExe).arg(arguments.join(" "));
#ifdef QT_DEBUG_MODE
  qDebug() << qPrintable(message);
#else
  emit gui->messageSig(LOG_STATUS, message);
#endif

  ldview.start(Preferences::ldviewExe,arguments);
  if ( ! ldview.waitForFinished()) {
    if (ldview.exitCode() != 0) {
      QByteArray status = ldview.readAll();
      QString str;
      str.append(status);
      emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView PLI render failed with exit code %1\n%2") .arg(ldview.exitCode()) .arg(str));
      return -1;
    }
  }

  // move generated CSI images
  if (useLDViewSCall()){
    QString ldrName;
    QDir dir(QDir::currentPath() + "/" + Paths::tmpDir);
    foreach(ldrName, ldrNames){
        QFileInfo fInfo(ldrName.replace(".ldr",".png"));
        QString imageFilePath = QDir::currentPath() + "/" +
            Paths::partsDir + "/" + fInfo.fileName();
        if (! dir.rename(fInfo.absoluteFilePath(), imageFilePath)){
            //in case failure because file exist
            QFile pngFile(imageFilePath);
            if (! pngFile.exists()){
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView PLI image file move failed for\n%1")
                           .arg(imageFilePath));
                return -1;
              }else {
                //file exist so delete and retry
                QFile pngFile(imageFilePath);
                if (pngFile.remove()) {
                    //retry
                    if (! dir.rename(fInfo.absoluteFilePath(), imageFilePath)){
                        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView PLI image file move failed after old file removal for\n%1")
                                             .arg(imageFilePath));
                        return -1;
                      }
                  } else {
                    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("LDView could not remove old PLI image file \n%1")
                                         .arg(imageFilePath));
                    return -1;
                  }
              }
          }
      }
  }

  return 0;
}

/***************************************************************************
 *
 * Native renderer
 *
 **************************************************************************/

float Native::cameraDistance(
    Meta &meta,
    float scale)
{
  return stdCameraDistance(meta,scale);
}

int Native::renderCsi(
  const QString     &addLine,
  const QStringList &csiParts,
  const QStringList &csiKeys,
  const QString     &pngName,
        Meta        &meta)
{
  Q_UNUSED(addLine);
  Q_UNUSED(csiParts);
  Q_UNUSED(csiKeys);

  QString ldrName = QDir::currentPath() + "/" + Paths::tmpDir + "/csi.ldr";

  // Renderer options
  NativeOptions Options;
  Options.ImageType         = CSI;
  Options.ImageFileName     = pngName;
  Options.ImageWidth        = gui->pageSize(meta.LPub.page, 0);
  Options.ImageHeight       = gui->pageSize(meta.LPub.page, 1);
  Options.Latitude          = meta.LPub.assem.angle.value(0);
  Options.Longitude         = meta.LPub.assem.angle.value(1);
  Options.HighlightNewParts = gui->suppressColourMeta(); //Preferences::enableHighlightStep;
  Options.CameraDistance    = -cameraDistance(meta,meta.LPub.assem.modelScale.value())/11659;

  // Set new project
  Project* CsiImageProject = new Project();

  // Load new project
  if (CsiImageProject->Load(ldrName))
  {
    gApplication->SetProject(CsiImageProject);
    gMainWindow->UpdateAllViews();
  }
  else
  {
    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not create Native CSI image project."));
    delete CsiImageProject;
    return -1;
  }

  // Generate image
  emit gui->messageSig(LOG_STATUS, "Executing Native render CSI - please wait...");

  CreateNativeImage(Options);

  return 0;
}

int Native::renderPli(
  const QStringList &ldrNames,
  const QString     &pngName,
  Meta              &meta,
  bool               bom)
{
  // Line Width
  int lineThickness = (int(resolution()/lineThickness));
  Q_UNUSED(lineThickness);

  // Select meta type
  PliMeta &metaType = bom ? meta.LPub.bom : meta.LPub.pli;

  // Renderer options
  NativeOptions Options;
  Options.ImageType         = PLI;
  Options.ImageFileName     = pngName;
  Options.ImageWidth        = gui->pageSize(meta.LPub.page, 0);
  Options.ImageHeight       = gui->pageSize(meta.LPub.page, 1);
  Options.Latitude          = metaType.angle.value(0);
  Options.Longitude         = metaType.angle.value(1);
  Options.CameraDistance    = -cameraDistance(meta,metaType.modelScale.value())/11659;

  // Set and load new project
  Project* PliImageProject = new Project();

  // Load project
  if (PliImageProject->Load(ldrNames.first()))
  {
    gApplication->SetProject(PliImageProject);
    gMainWindow->UpdateAllViews();
  }
  else
  {
    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not create Native PLI image project."));
    delete PliImageProject;
    return -1;
  }

  // Generate image
  emit gui->messageSig(LOG_STATUS, "Executing Native render PLI - please wait...");

  CreateNativeImage(Options);

  return 0;
}

void Render::CreateNativeImage(const NativeOptions &Options)
{
        View* ActiveView = gMainWindow->GetActiveView();
        ActiveView->MakeCurrent();

        lcModel* Model = ActiveView->mModel;

        lcStep CurrentStep = Model->GetCurrentStep();

        lcContext* Context = ActiveView->mContext;

        lcCamera* Camera = gMainWindow->GetActiveView()->mCamera;

        //Camera->SetAngles(Options.Latitude,Options.Longitude);

        Camera->SetOrtho(Options.Orthographic);

        Camera->Zoom(Options.CameraDistance,CurrentStep,true);

        const int ImageWidth = Options.ImageWidth;
        const int ImageHeight = Options.ImageHeight;

        View View(Model);
        View.SetHighlight(Options.HighlightNewParts);
        View.SetCamera(Camera, false);
        View.SetContext(Context);

        QString imageType = Options.ImageType == CSI ? "CSI" : "PLI";

        if (!View.BeginRenderToImage(ImageWidth, ImageHeight))
        {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not begin RenderToImage for Native %1 image.").arg(imageType));
                return;
        }

        Model->SetTemporaryStep(CurrentStep);

        View.OnDraw();

        struct NativeImage
        {
                QImage RenderedImage;
                QRect Bounds;
        };

        NativeImage Image;
        Image.RenderedImage = View.GetRenderImage();

        auto CalculateImageBounds = [](NativeImage& Image)
        {
                QImage& RenderedImage = Image.RenderedImage;
                int Width = RenderedImage.width();
                int Height = RenderedImage.height();

                int MinX = Width;
                int MinY = Height;
                int MaxX = 0;
                int MaxY = 0;

                for (int x = 0; x < Width; x++)
                {
                        for (int y = 0; y < Height; y++)
                        {
                                if (qAlpha(RenderedImage.pixel(x, y)))
                                {
                                        MinX = qMin(x, MinX);
                                        MinY = qMin(y, MinY);
                                        MaxX = qMax(x, MaxX);
                                        MaxY = qMax(y, MaxY);
                                }
                        }
                }

                Image.Bounds = QRect(QPoint(MinX, MinY), QPoint(MaxX, MaxY));
        };

        CalculateImageBounds(Image);

        QImageWriter Writer(Options.ImageFileName);

        if (Writer.format().isEmpty())
                Writer.setFormat("PNG");

        if (!Writer.write(QImage(Image.RenderedImage.copy(Image.Bounds))))
        {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not write to Native %1 image file '%2': %3.")
                                     .arg(imageType).arg(Options.ImageFileName).arg(Writer.errorString()));
                return;
        }

        View.EndRenderToImage();
        Context->ClearResources();

        Model->SetTemporaryStep(CurrentStep);

        if (!Model->mActive)
                Model->CalculateStep(LC_STEP_MAX);
}

bool Render::CreateNativePovFile(Project * PovGenProject, const NativeOptions& Options)
{

         QString Type = Options.ImageType == CSI ? "CSI" : "PLI";

         lcCamera* Camera = gMainWindow->GetActiveView()->mCamera;
         Camera->SetOrtho(Options.Orthographic);
         Camera->Zoom(Options.CameraDistance,PovGenProject->mModels[0]->GetCurrentStep(),true);

         lcArray<lcModelPartsEntry> ModelParts;

         PovGenProject->GetModelParts(ModelParts);

         if (ModelParts.IsEmpty())
         {
                 emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Nothing to export - %1 parts list is empty.").arg(Type));
                 return false;
         }

        QString SaveFileName = PovGenProject->GetExportFileName(Options.PovFileName, QLatin1String("pov"), QMessageBox::tr("Export POV-Ray"), QMessageBox::tr("POV-Ray Files (*.pov);;All Files (*.*)"));

	if (SaveFileName.isEmpty())
		return false;

	lcDiskFile POVFile(SaveFileName);

	if (!POVFile.Open(QIODevice::WriteOnly))
	{
		emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not open %1 pov file '%2' for writing.").arg(Type).arg(SaveFileName));
		return false;
	}

	char Line[1024];

	static const QString fmtDateTime("ddd MMM d hh:mm:ss yyyy");

	POVFile.WriteLine("// Generated by: " VER_PRODUCTNAME_STR " Native Renderer (C) 2018 Trevor SANDY\n");
	POVFile.WriteLine("// See: " VER_COMPANYDOMAIN_STR "\n");
	sprintf(Line, "// Date: %s\n", QDateTime::currentDateTime().toString(fmtDateTime).toLatin1().constData());
	POVFile.WriteLine(Line);
	sprintf(Line, "// %s Command: %s\n\n", Type.toLatin1().constData(), Options.PovGenCommand.toLatin1().constData());
	POVFile.WriteLine(Line);
	POVFile.WriteLine("// This file was automatically generated from an LDraw file by " VER_PRODUCTNAME_STR "\n\n");

	POVFile.WriteLine("#version 3.6;\n\n");

        POVFile.WriteLine("#declare POVQual = 3;	// Quality (0 = Bounding Box; 1 = No Refraction; 2 = Normal; 3 = Stud Logos)\n");
        POVFile.WriteLine("#declare POVStuds = 1;	// Show studs? (1 = YES; 0 = NO)\n");
        POVFile.WriteLine("#declare POVRefls = 1;	// Reflections? (1 = YES; 0 = NO)\n");
        POVFile.WriteLine("#declare POVShads = 1;	// Shadows? (1 = YES; 0 = NO)\n");
        POVFile.WriteLine("#declare POVShads = 1;	// Shadows? (1 = YES; 0 = NO)\n\n");

	lcPiecesLibrary* Library = lcGetPiecesLibrary();
	std::map<PieceInfo*, std::pair<char[LC_PIECE_NAME_LEN], int>> PieceTable;
	int NumColors = gColorList.GetSize();
	std::vector<std::array<char, LC_MAX_COLOR_NAME>> ColorTable(NumColors);

	enum
	{
		LGEO_PIECE_LGEO  = 0x01,
		LGEO_PIECE_AR    = 0x02,
		LGEO_PIECE_SLOPE = 0x04
	};

	enum
	{
		LGEO_COLOR_SOLID       = 0x01,
		LGEO_COLOR_TRANSPARENT = 0x02,
		LGEO_COLOR_CHROME      = 0x04,
		LGEO_COLOR_PEARL       = 0x08,
		LGEO_COLOR_METALLIC    = 0x10,
		LGEO_COLOR_RUBBER      = 0x20,
		LGEO_COLOR_GLITTER     = 0x40
	};

	QString LGEOPath = Preferences::lgeoPath;

	if (LGEOPath.isEmpty())
		POVFile.WriteLine("global_settings {\n  assumed_gamma 1.0\n}\n\n");

	POVFile.WriteLine("#declare lg_quality = POVQual;\n#if (lg_quality = 3)\n#declare lg_quality = 4;\n#end\n\n");

	POVFile.WriteLine("#declare lg_studs = POVStuds;\n\n");

	if (!LGEOPath.isEmpty())
	{
		lcDiskFile TableFile(QFileInfo(QDir(LGEOPath), QLatin1String("lg_elements.lst")).absoluteFilePath());

		if (!TableFile.Open(QIODevice::ReadOnly))
		{
			emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not find LGEO lg_elements.lst file in folder '%1'.").arg(LGEOPath));
			return false;
		}

		while (TableFile.ReadLine(Line, sizeof(Line)))
		{
			char Src[1024], Dst[1024], Flags[1024];

			if (*Line == ';')
				continue;

			if (sscanf(Line,"%s%s%s", Src, Dst, Flags) != 3)
				continue;

			strcat(Src, ".dat");

			PieceInfo* Info = Library->FindPiece(Src, nullptr, false, false);
			if (!Info)
				continue;

			if (strchr(Flags, 'L'))
			{
				std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[Info];
				Entry.second |= LGEO_PIECE_LGEO;
				sprintf(Entry.first, "lg_%s", Dst);
			}

			if (strchr(Flags, 'A'))
			{
				std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[Info];
				Entry.second |= LGEO_PIECE_AR;
				sprintf(Entry.first, "ar_%s", Dst);
			}

			if (strchr(Flags, 'S'))
			{
				std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[Info];
				Entry.second |= LGEO_PIECE_SLOPE;
				Entry.first[0] = 0;
			}
		}

		lcDiskFile ColorFile(QFileInfo(QDir(LGEOPath), QLatin1String("lg_colors.lst")).absoluteFilePath());

		if (!ColorFile.Open(QIODevice::ReadOnly))
		{
			emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not find LGEO lg_colors.lst file in folder '%1'.").arg(LGEOPath));
			return false;
		}

		while (ColorFile.ReadLine(Line, sizeof(Line)))
		{
			char Name[1024], Flags[1024];
			int Code;

			if (*Line == ';')
				continue;

			if (sscanf(Line,"%d%s%s", &Code, Name, Flags) != 3)
				continue;

			int Color = lcGetColorIndex(Code);
			if (Color >= NumColors)
				continue;

			strcpy(ColorTable[Color].data(), Name);
		}
	}

	if (!LGEOPath.isEmpty())
	{
		POVFile.WriteLine("#include \"lg_defs.inc\"\n#include \"lg_color.inc\"\n\n");

		for (int PartIdx = 0; PartIdx < ModelParts.GetSize(); PartIdx++)
		{
			PieceInfo* Info = ModelParts[PartIdx].Info;

			for (int CheckIdx = 0; CheckIdx < ModelParts.GetSize(); CheckIdx++)
			{
				if (ModelParts[CheckIdx].Info != Info)
					continue;

				if (CheckIdx != PartIdx)
					break;

				auto Search = PieceTable.find(Info);

				if (Search != PieceTable.end())
				{
					const std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = Search->second;
					if (Entry.first[0])
					{
						sprintf(Line, "#include \"%s.inc\"\n", Entry.first);
						POVFile.WriteLine(Line);
					}
				}

				break;
			}
		}

		POVFile.WriteLine("\n");
	}

	for (int ColorIdx = 0; ColorIdx < gColorList.GetSize(); ColorIdx++)
	{
		lcColor* Color = &gColorList[ColorIdx];

		for (int PartIdx = 0; PartIdx < ModelParts.GetSize(); PartIdx++)
		{
			int PartColorIdx;
			PartColorIdx = ModelParts[PartIdx].ColorIndex;

			if (PartColorIdx == ColorIdx)
			{
				if (ColorTable[ColorIdx][0])
				{
					  sprintf(Line, "#ifndef (POVColor_%d) // %s\n"
							"#declare POVColor_%d = #if (version >= 3.1) material { #end texture { %s } #if (version >= 3.1) } #end\n"
							"#end\n",
							ColorIdx, ColorTable[ColorIdx].data(), ColorIdx, ColorTable[ColorIdx].data());
				}
				else
				if (lcIsColorTranslucent(ColorIdx))
				{
					  sprintf(Line, "#ifndef (POVColor_%d) // %s\n"
							"#declare POVColor_%d = #if (version >= 3.1) material { #end texture { pigment { rgb <%f, %f, %f> filter 0.9 } finish { ambient 0.3 diffuse 0.2 reflection 0.25 phong 0.3 phong_size 60 } } #if (version >= 3.1) } #end\n"
							"#end\n",
							ColorIdx, Color->SafeName, ColorIdx, Color->Value[0], Color->Value[1], Color->Value[2]);
				}
				else
				{
					  sprintf(Line, "#ifndef (POVColor_%d) // %s\n"
							"#declare POVColor_%d = #if (version >= 3.1) material { #end texture { pigment { rgb <%f, %f, %f> } finish { ambient 0.1 phong 0.2 phong_size 20 } } #if (version >= 3.1) } #end\n"
							"#end\n",
							ColorIdx, Color->SafeName, ColorIdx, Color->Value[0], Color->Value[1], Color->Value[2]);
				}

				POVFile.WriteLine(Line);
			}
		}

		if (!ColorTable[ColorIdx][0])
			sprintf(ColorTable[ColorIdx].data(), "lc_%s", Color->SafeName);
	}

	POVFile.WriteLine("\n");

	lcArray<const char*> ColorTablePointer;
	ColorTablePointer.SetSize(NumColors);
	for (int ColorIdx = 0; ColorIdx < NumColors; ColorIdx++)
		ColorTablePointer[ColorIdx] = ColorTable[ColorIdx].data();

	for (int PartIdx = 0; PartIdx < ModelParts.GetSize(); PartIdx++)
	{
		PieceInfo* Info = ModelParts[PartIdx].Info;
		lcMesh* Mesh = Info->GetMesh();
		std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[Info];

		if (!Mesh || Entry.first[0])
			continue;

		char Name[LC_PIECE_NAME_LEN];
		char* Ptr;

		strcpy(Name, Info->mFileName);
		while ((Ptr = strchr(Name, '-')))
			*Ptr = '_';
		while ((Ptr = strchr(Name, '.')))
			*Ptr = '_';

		sprintf(Entry.first, "lc_%s", Name);

		Mesh->ExportPOVRay(POVFile, Name, &ColorTablePointer[0]);

		sprintf(Line, "#declare lc_%s_clear = lc_%s\n\n", Name, Name);
		POVFile.WriteLine(Line);
	}

        const lcVector3& Position = Camera->mPosition;
        const lcVector3& Target = Camera->mTargetPosition;
        const lcVector3& Up = Camera->mUpVector;
        const lcModelProperties& Properties = PovGenProject->mModels[0]->GetProperties();

	POVFile.WriteLine("// Camera Settings\n");
	sprintf(Line, "#declare POVCameraLoc = < %f,%f,%f >;\n", Position[1] / 25.0f, Position[0] / 25.0f, Position[2] / 25.0f);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVCameraLookAt =< %f,%f,%f >;\n", Target[1] / 25.0f, Target[0] / 25.0f, Target[2] / 25.0f);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVCameraSky = < %f,%f,%f >;\n", Up[1], Up[0], Up[2]);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVCameraFov = %f;\n\n", Camera->m_fovy);
	POVFile.WriteLine(Line);

	POVFile.WriteLine("// Camera\n");
	sprintf(Line, "#ifndef (POVSkipCamera)\n"
		      "camera {\n"
		      "  #declare POVCamAspect = image_width/image_height;\n"
		      "  perspective\n"
		      "  right x * image_width / image_height\n"
		      "  location POVCameraLoc\n"
		      "  sky POVCameraSky\n"
		      "  look_at POVCameraLookAt\n"
		      "  angle POVCameraFov * POVCamAspect\n"
		      "}\n"
		      "#end\n\n");
	POVFile.WriteLine(Line);

	sprintf(Line, "background { color rgb <%1g, %1g, %1g> }\n\n", Properties.mBackgroundSolidColor[0], Properties.mBackgroundSolidColor[1], Properties.mBackgroundSolidColor[2]);
	POVFile.WriteLine(Line);

	lcVector3 Min(FLT_MAX, FLT_MAX, FLT_MAX);
	lcVector3 Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const lcModelPartsEntry& ModelPart : ModelParts)
	{
		lcVector3 Points[8];

		lcGetBoxCorners(ModelPart.Info->GetBoundingBox(), Points);

		for (int PointIdx = 0; PointIdx < 8; PointIdx++)
		{
			lcVector3 Point = lcMul31(Points[PointIdx], ModelPart.WorldMatrix);

			Min = lcMin(Point, Min);
			Max = lcMax(Point, Max);
		}
	}

	lcVector3 Center = (Min + Max) / 2.0f;
	float Radius = (Max - Center).Length() / 25.0f;
	Center = lcVector3(Center[1], Center[0], Center[2]) / 25.0f;

	POVFile.WriteLine("// Model bounds information\n");
	sprintf(Line, "#declare POVMinX = %f;\n",Min[1]);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVMinY = %f;\n",Min[0]);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVMinZ = %f;\n",Min[2]);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVMaxX = %f;\n",Max[1]);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVMaxY = %f;\n",Max[0]);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVMaxZ = %f;\n",Max[2]);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVCenterX = %f;\n",Center[1]);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVCenterY = %f;\n",Center[0]);
	POVFile.WriteLine(Line);
	sprintf(Line, "#declare POVCenterZ = %f;\n",Center[2]);
	POVFile.WriteLine(Line);
	POVFile.WriteLine("#declare POVCenter = <POVCenterX,POVCenterY,POVCenterZ>;\n");
	sprintf(Line, "#declare POVRadius = %f;\n\n", Radius);
	POVFile.WriteLine(Line);

	POVFile.WriteLine("// Lights\n");
	sprintf(Line, "#ifndef (POVSkipLight_1)\nlight_source {\n  <%f*POVRadius, %f*POVRadius, %f*POVRadius> + POVCenter\n  color rgb <1,1,1>\n}\n#end\n", 0.0f, -1.414214f, -1.414214f);
	POVFile.WriteLine(Line); // Latitude,Longitude: 45,0,POVRadius*2
	sprintf(Line, "#ifndef (POVSkipLight_2)\nlight_source {\n  <%f*POVRadius, %f*POVRadius, %f*POVRadius> + POVCenter\n  color rgb <1,1,1>\n}\n#end\n", 1.5f, -1.0f, 0.866026f);
	POVFile.WriteLine(Line); // Latitude,Longitude: 30,120,POVRadius*2
	sprintf(Line, "#ifndef (POVSkipLight_3)\nlight_source {\n  <%f*POVRadius, %f*POVRadius, %f*POVRadius> + POVCenter\n  color rgb <1,1,1>\n}\n#end\n", -0.866025f, -1.732051f, 0.5f);
	POVFile.WriteLine(Line); // Latitude,Longitude: 60,-120,POVRadius*2
	sprintf(Line, "#ifndef (POVSkipLight_4)\nlight_source {\n  <%f*POVRadius, %f*POVRadius, %f*POVRadius> + POVCenter\n  color rgb <1,1,1>\n}\n#end\n\n", -2.0f, 0.0f, -2.0f);
	POVFile.WriteLine(Line);

	for (int PartIdx = 0; PartIdx < ModelParts.GetSize(); PartIdx++)
	{
		std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[ModelParts[PartIdx].Info];
		int ColorIdx;

		ColorIdx = ModelParts[PartIdx].ColorIndex;
		const char* Suffix = lcIsColorTranslucent(ColorIdx) ? "_clear" : "";

		const float* f = ModelParts[PartIdx].WorldMatrix;

		if (Entry.second & LGEO_PIECE_SLOPE)
		{
			sprintf(Line, "merge {\n"
				      " object {\n"
				      "   %s%s\n"
				      "   #if (version >= 3.1) material #else texture #end { POVColor_%d }\n"
				      " }\n"
				      " object {\n"
				      "   %s_slope\n"
				      "   #if (version >= 3.1) material { #end\n"
				      "     texture {\n"
				      "         %s\n"
				      "         #if (POVQual > 1) normal { bumps 0.3 scale 0.02 } #end\n"
				      "    }\n"
				      "   #if (version >= 3.1) } #end\n"
				      " }\n"
				      " matrix <%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f>\n}\n",
				      Entry.first, Suffix, ColorIdx, Entry.first, ColorTable[ColorIdx].data(),
				      -f[5], -f[4], -f[6], -f[1], -f[0], -f[2], f[9], f[8], f[10], f[13] / 25.0f, f[12] / 25.0f, f[14] / 25.0f);
		}
		else
		{
			sprintf(Line, "object {\n %s%s\n #if (version >= 3.1) material #else texture #end { POVColor_%d }\n matrix <%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f>\n}\n",
				      Entry.first, Suffix, ColorIdx, -f[5], -f[4], -f[6], -f[1], -f[0], -f[2], f[9], f[8], f[10], f[13] / 25.0f, f[12] / 25.0f, f[14] / 25.0f);
		}

		POVFile.WriteLine(Line);
	}

	POVFile.Close();

	return true;
}

bool Render::LoadViewer(const ViewerOptions &Options){

    QString viewerCsiName = Options.ViewerCsiName;

    Project* StepProject = new Project();
    if (LoadStepProject(StepProject, viewerCsiName)){
        gApplication->SetProject(StepProject);
        gMainWindow->UpdateAllViews();
    }
    else
    {
        emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not load step project for %1.")
                             .arg(Options.ViewerCsiName));
        delete StepProject;
        return false;
    }

    gui->setViewerCsiName(viewerCsiName);

    View* ActiveView = gMainWindow->GetActiveView();

    ActiveView->SetProjection(Options.Orthographic);

//--    ActiveView->SetCameraAngles(Options.Latitude, Options.Longitude);

    return true;
}

bool Render::LoadStepProject(Project* StepProject, const QString& viewerCsiName)
{
        QString FileName = gui->getViewerStepFilePath(viewerCsiName);

        if (FileName.isEmpty())
        {
               emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive 3DViewer CSI path for %1.").arg(FileName));
               return false;
        }

        QStringList CsiContent = gui->getViewerStepContents(viewerCsiName);
        if (CsiContent.isEmpty())
        {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Did not receive 3DViewer CSI content for %1.").arg(FileName));
                return false;
        }

#ifdef QT_DEBUG_MODE
        QFileInfo outFileInfo(FileName);
        QString outfileName = QString("%1/%2_%3.ldr")
               .arg(outFileInfo.absolutePath())
               .arg(outFileInfo.baseName().replace(".ldr",""))
               .arg(QString(viewerCsiName).replace(";","_"));
        QFile file(outfileName);
        if ( ! file.open(QFile::WriteOnly | QFile::Text)) {
                emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Cannot open 3DViewer file %1 for writing: %2")
                                 .arg(outfileName) .arg(file.errorString()));
        }
        QTextStream out(&file);
        for (int i = 0; i < CsiContent.size(); i++) {
                QString line = CsiContent[i];
                out << line << endl;
        }
        file.close();
#endif

	StepProject->mModels.DeleteAll();
	StepProject->SetFileName(FileName);

	QByteArray QBA;
	foreach(QString line, CsiContent){
	       QBA.append(line);
	       QBA.append(QString("\n"));
	}

	if (StepProject->mFileName.isEmpty())
	{
		emit gui->messageSig(LOG_ERROR,QMessageBox::tr("3DViewer file name not set!"));
		return false;
	}
	QFileInfo FileInfo(StepProject->mFileName);

	QBuffer Buffer(&QBA);
	Buffer.open(QIODevice::ReadOnly);

        while (!Buffer.atEnd())
        {
                lcModel* Model = new lcModel(QString());
                Model->SplitMPD(Buffer);

                if (StepProject->mModels.IsEmpty() || !Model->GetProperties().mName.isEmpty())
                {
                        StepProject->mModels.Add(Model);
                        Model->CreatePieceInfo(StepProject);
                }
                else
                        delete Model;
        }

        Buffer.seek(0);

        for (int ModelIdx = 0; ModelIdx < StepProject->mModels.GetSize(); ModelIdx++)
        {
                lcModel* Model = StepProject->mModels[ModelIdx];
                Model->LoadLDraw(Buffer, StepProject);
                Model->SetSaved();
        }


	if (StepProject->mModels.IsEmpty())
		return false;

	if (StepProject->mModels.GetSize() == 1)
	{
		lcModel* Model = StepProject->mModels[0];

		if (Model->GetProperties().mName.isEmpty())
		{
			Model->SetName(FileInfo.fileName());
			lcGetPiecesLibrary()->RenamePiece(Model->GetPieceInfo(), FileInfo.fileName().toLatin1());
		}
	}

	lcArray<lcModel*> UpdatedModels;
	UpdatedModels.AllocGrow(StepProject->mModels.GetSize());

	for (lcModel* Model : StepProject->mModels)
	{
		Model->UpdateMesh();
		Model->UpdatePieceInfo(UpdatedModels);
	}

	StepProject->mModified = false;

	return true;
}
