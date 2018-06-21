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

#include "LDVWidget.h"

#include <QPainter>
#include <QDateTime>
#include <QFileInfo>
#include <QApplication>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QDesktopServices>
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#endif // __APPLE__

#include <TCFoundation/mystring.h>
#include <TCFoundation/TCAutoreleasePool.h>
#include <TCFoundation/TCUserDefaults.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/TCStringArray.h>
#include <TCFoundation/TCAlertManager.h>
#include <LDLib/LDUserDefaultsKeys.h>
#include <LDLib/LDConsoleAlertHandler.h>
#include <LDLoader/LDLModel.h>
#include <LDLib/LDrawModelViewer.h>
#include <TRE/TREMainModel.h>
#include <TRE/TREGLExtensions.h>
#include <LDViewExportOption.h>
#include <LDVAlertHandler.h>
#include <misc.h>
#include <vector>
#include <string>
#include <assert.h>

#include "lpub_messages.h"
#include "lpub_preferences.h"
#include "version.h"
#include "paths.h"

#define PNG_IMAGE_TYPE_INDEX 1
#define BMP_IMAGE_TYPE_INDEX 2
#define JPG_IMAGE_TYPE_INDEX 3
#define WIN_WIDTH 640
#define WIN_HEIGHT 480

LDVWidget* ldvWidget;
IniFlag LDVWidget::iniFlag;

static QGLFormat ldvFormat;

LDVWidget::LDVWidget(QWidget *parent, IniFlag iniflag)
        :QGLWidget(QGLFormat(QGL::SampleBuffers),parent),
        modelViewer(new LDrawModelViewer(100, 100)),
        snapshotTaker(NULL),
        alertHandler(new AlertHandler(this)),
        exportType(LDrawModelViewer::ETPov)
{

  setupLDVFormat();

#ifdef WIN32
  runningWithConsole();
#endif // WIN32

  QString appName = Preferences::lpub3dAppName;
  QString extrasPath = QString("%1/extras").arg(Preferences::lpubDataPath);

#ifndef _LDV_LOG
#define _LDV_LOG
#endif

#ifdef _LDV_LOG
  char *logFile = TCUserDefaults::pathForKey(LOGFILE_KEY);
  if (logFile == NULL)
  {
         QString ldvLogFile = QDir::toNativeSeparators(QString("%1/ldv.log").arg(QDir::currentPath()));
         TCUserDefaults::setPathForKey(ldvLogFile.toLatin1().constData(),LOGFILE_KEY);
  }
#endif

  iniFlag = iniflag;
  if (!TCUserDefaults::isIniFileSet())
  {
         QString iniFile;
         switch (iniFlag)
         {
              case NativePOVIni:
                  iniFile = Preferences::nativePOVIni;
                  break;
              case LDViewPOVIni:
                  iniFile = Preferences::ldviewPOVIni;
                  break;
              case LDViewIni:
                  iniFile = Preferences::ldviewIni;
                  break;
              default:
                  fprintf(stdout, "Ini file not specified!");
         }
         if (!TCUserDefaults::setIniFile(iniFile.toLatin1().constData()))
              fprintf(stdout, QString( QString("Could not set native POV INI file: %1").arg(iniFile)).toLatin1().constData());
  }

  TCUserDefaults::setAppName(appName.toLatin1().constData());

  QImage studImage(":/resources/studlogo.png");
  TREMainModel::setRawStudTextureData(studImage.bits(),studImage.byteCount());

  QFile file("LDViewMessages.ini");
  if (!file.exists())
  {
         fprintf(stdout, QString("First Messages File Check Failed: " + QDir::currentPath() + "/" + file.fileName()).toLatin1().constData());
         QDir::setCurrent(QDir(extrasPath).absolutePath());
  }
  if (!file.exists())
  {
         fprintf(stdout, QString("Second Messages File Check Failed: " + QDir::currentPath() + "/" + file.fileName()).toLatin1().constData());
  }

  if (!TCLocalStrings::loadStringTable(QString(QDir::currentPath() + "/" + file.fileName()).toLatin1().constData()))
  {
         fprintf(stdout, QString("Could not load LDViewMessages.ini file.").toLatin1().constData());
  }

  QFile fontFile(":/resources/sansserif.fnt");
  if (fontFile.exists())
  {
      int len = fontFile.size();
      if (len > 0)
      {
          char *buffer = (char*)malloc(len);
          if ( fontFile.open( QIODevice::ReadOnly ))
          {
              QDataStream stream( &fontFile );
              stream.readRawData(buffer,len);
              modelViewer->setFontData((TCByte*)buffer,len);
          }
      }
  }

  QImage fontImage2x(":/resources/sanserif@2x.png");
  long len = fontImage2x.byteCount();
  modelViewer->setRawFont2xData(fontImage2x.bits(),len);

  // Needed to display preferences
  char *sessionName;
  sessionName = TCUserDefaults::getSavedSessionNameFromKey(PREFERENCE_SET_KEY);
  if (sessionName && sessionName[0])
  {
          TCUserDefaults::setSessionName(sessionName, NULL, false);
  }
  delete sessionName;

  ldvPreferences = new LDVPreferences(parent, this);
  ldvPreferences->doApply();

  setFocusPolicy(Qt::StrongFocus);

  ldvWidget = this;

}

LDVWidget::~LDVWidget(void)
{
	TCObject::release(snapshotTaker);
	TCObject::release(modelViewer);
	delete ldvPreferences;

	TCObject::release(alertHandler);
	alertHandler = NULL;

	ldvWidget = NULL;
}

void LDVWidget::setAppArgs(QStringList &argv)
{
    appArgs = argv;
    modelViewer->setProgramPath(appArgs.at(0).toLatin1().constData());
}

void LDVWidget::showLDVExportOptions()
{
    LDViewExportOption exportOption(this,modelViewer);

    if (exportOption.exec() == QDialog::Rejected)
    {
        modelViewer->getExporter((LDrawModelViewer::ExportType)0, true);
    }
}

void LDVWidget::showLDVPreferences()
{
  ldvPreferences->show();
}

void LDVWidget::setupLDVFormat(void)
{
    ldvFormat.setAlpha(true);
    ldvFormat.setStencil(true);
//    ldvFormat.setDepthBufferSize(24);
//    ldvFormat.setRedBufferSize(8);
//    ldvFormat.setGreenBufferSize(8);
//    ldvFormat.setBlueBufferSize(8);
//    ldvFormat.setAlphaBufferSize(8);
//    ldvFormat.setStencilBufferSize(8);
    QGLFormat::setDefaultFormat(ldvFormat);
}

bool LDVWidget::doCommand(QStringList &arguments)
{	
    QString debugMessage = QString("Native command In: %1 %2")
                                   .arg(appArgs.at(0))
                                   .arg(arguments.join(" "));

    // Construct complete command line
    std::vector<std::string> commandLine;
    commandLine.push_back(QString(appArgs.at(0)).toStdString());
    foreach(QString arg, arguments)
        commandLine.push_back(arg.toStdString());
    std::vector<char*> Argv;
    for(const auto& arg : commandLine)
        Argv.push_back((char*)arg.data());
    Argv.push_back(nullptr);

    char *commandLineArgs[Argv.size()];

    convertArguments((int)(Argv.size() - 1), reinterpret_cast<char**>(Argv.data()), commandLineArgs);

    TCUserDefaults::setCommandLine(commandLineArgs);

    // Perform an export
    bool retValue;
    if ((retValue = LDSnapshotTaker::doCommandLine(false, true)) != true)
         fprintf(stdout, QString(QString("Failed to processs Native command: %1").arg(debugMessage)).toLatin1().constData());
    return retValue;

}

void LDVWidget::convertArguments(int Argc, char **Argv, char *argv[MAX_NUM_POV_GEN_ARGS]) {
    int argc = 0;
    for (int i = 0; i < Argc; ++i) {
        if (Argv[i] != NULL) {
            if (argc >= MAX_NUM_POV_GEN_ARGS) {
                fprintf(stdout, "More than %d arguments have been specified - aborting\n", MAX_NUM_POV_GEN_ARGS);
                return;
            }
            // printf("Arg %d: %s\n", argc, Argv[i]);
            argv[argc++] = Argv[i];
        }
    }
}

void LDVWidget::modelViewerAlertCallback(TCAlert *alert)
{
    if (alert)
    {
        QMessageBox::warning(this,VER_PRODUCTNAME_STR,alert->getMessage(),
            QMessageBox::Ok, QMessageBox::NoButton);
    }
}

bool LDVWidget::getUseFBO()
{
    return snapshotTaker != NULL && snapshotTaker->getUseFBO();
}

void LDVWidget::snapshotTakerAlertCallback(TCAlert *alert)
{
    if (strcmp(alert->getAlertClass(), "LDSnapshotTaker") == 0)
    {
        if (strcmp(alert->getMessage(), "MakeCurrent") == 0)
        {
              makeCurrent();
        }

        if (strcmp(alert->getMessage(), "PreFbo") == 0)
        {
            if (getUseFBO())
            {
                return;
            }
            else
            {
                makeCurrent();
                TREGLExtensions::setup();
                snapshotTaker = (LDSnapshotTaker*)alert->getSender()->retain();
                if (TREGLExtensions::haveFramebufferObjectExtension())
                {
                    snapshotTaker->setUseFBO(true);
                }
                if (!snapshotTaker->getUseFBO())
                {
                    setupSnapshotBackBuffer(ldvPreferences->getWindowWidth(), ldvPreferences->getWindowHeight());
                }
            }
        }
    }
}

void LDVWidget::initializeGL(void)
{
    makeCurrent();
    TREGLExtensions::setup();
    ldvPreferences->doCancel();
}

void LDVWidget::resizeGL(int width, int height)
{
    width =  TCUserDefaults::longForKey(SAVE_ACTUAL_SIZE_KEY, 1, false) ?
             TCUserDefaults::longForKey(WINDOW_WIDTH_KEY, WIN_WIDTH, false) :
             TCUserDefaults::longForKey(SAVE_WIDTH_KEY, 1024, false);
    height = TCUserDefaults::longForKey(SAVE_ACTUAL_SIZE_KEY, 1, false) ?
             TCUserDefaults::longForKey(WINDOW_HEIGHT_KEY, WIN_HEIGHT, false) :
             TCUserDefaults::longForKey(SAVE_HEIGHT_KEY, 768, false);

    resize(width, height);

    modelViewer->setWidth(width);
    modelViewer->setHeight(height);

    glViewport(0, 0, width, height);

    QSize surfaceWindowSize = size();
    ldvPreferences->setWindowSize(surfaceWindowSize.width(),
                                  surfaceWindowSize.height());
}

void LDVWidget::paintGL(void)
{
    glEnable(GL_DEPTH_TEST);
    makeCurrent();
    if (!TREGLExtensions::haveFramebufferObjectExtension())
    {
         glDrawBuffer(GL_BACK);
         glReadBuffer(GL_BACK);
    }
    modelViewer->update();
}

void LDVWidget::setupSnapshotBackBuffer(int width, int height)
{
    modelViewer->setSlowClear(true);
    modelViewer->setWidth(width);
    modelViewer->setHeight(height);
    modelViewer->setup();
    glReadBuffer(GL_BACK);
}
