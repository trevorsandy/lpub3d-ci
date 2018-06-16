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

#ifndef LDVWIDGET_H
#define LDVWIDGET_H

#include <QGL>
#include <QGLWidget>
#include <QDateTime>
#include <QFileDialog>

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCAlertManager.h>

#include <LDLib/LDrawModelViewer.h>
#include <LDLib/LDSnapshotTaker.h>
#include <LDVQt/LDVPreferences.h>

#include <LDLib/LDInputHandler.h>   // May not need - linked to setViewMode

#include "name.h"

class LDrawModelViewer;
class AlertHandler;
class TCStringArray;               // May not need this

class LDVWidget : public QGLWidget
{

  Q_OBJECT

public:
  LDVWidget(QWidget *parent=NULL);
  ~LDVWidget(void);

  void setAppArgs(QStringList &argv);
  // May not need this...
  void setViewMode(LDInputHandler::ViewMode value, bool examineLatLong,
               bool keepRightSideUp, bool saveSettings=true);
  // End may not need this...

  LDrawModelViewer *getModelViewer(void) { return modelViewer; }

  static void setupDefaultFormat(void);
  static void convertArguments(int Argc, char **Argv, char *argv[MAX_NUM_POV_GEN_ARGS]);

  void modelViewerAlertCallback(TCAlert *alert);
  void snapshotTakerAlertCallback(TCAlert *alert);

  // May not need these...
  static bool staticFileCaseCallback(char *filename);
  static bool staticFileCaseLevel(QDir &dir, char *filename);
  // End may not need these...

  void showLDVExportOptions(void);
  void showLDVPreferences(void);

  bool doCommand(QStringList &arguments);

protected:
  // GL Widget overrides
  void initializeGL(void);
  void resizeGL(int width, int height);
  void swap_Buffers(void);
  void paintGL(void);

  // Events
  void paintEvent(QPaintEvent *event);

  void lock(void);
  void unlock(void);

  int lockCount;
  bool painting, loading, saving, printing;
  LDrawModelViewer *modelViewer;
  LDInputHandler::ViewMode viewMode;    // may not need this
  LDSnapshotTaker *snapshotTaker;
  AlertHandler *alertHandler;

  int exportType;
  int mwidth, mheight;
  bool redrawRequested;                  // don't need this
  int saveImageWidth;
  int saveImageHeight;
  const char *saveImageFilename;
  bool saveImageZoomToFit;
  bool saveImageResult;
  bool commandLineSnapshotSave;          // do not need this


  LDVPreferences *ldvPreferences;
  QStringList appArgs;

  LDPreferences::SaveOp curSaveOp;       // do not need this
  LDInputHandler *inputHandler;          // do not need this
  int saveDigits;                        // do not need this
  int saveImageType;                     // do not need this


};

extern LDVWidget* ldvWidget;

#endif // LDVWIDGET_H
