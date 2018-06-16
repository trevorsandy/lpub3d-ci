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
#include <QPaintEvent>
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

#define PNG_IMAGE_TYPE_INDEX 1
#define BMP_IMAGE_TYPE_INDEX 2
#define JPG_IMAGE_TYPE_INDEX 3
#define WIN_WIDTH 640
#define WIN_HEIGHT 480

LDVWidget* ldvWidget;

static QGLFormat defaultFormat;

LDVWidget::LDVWidget(QWidget *parent)
        :QGLWidget(parent),
        lockCount(0),
        painting(false), loading(false), saving(false),	printing(false),
        modelViewer(new LDrawModelViewer(100, 100)),
        viewMode(LDInputHandler::VMExamine),
        snapshotTaker(NULL),
        alertHandler(new AlertHandler(this)),
        exportType(LDrawModelViewer::ETPov),
        commandLineSnapshotSave(false)
{
  setupDefaultFormat();

  runningWithConsole();

  LDLModel::setFileCaseCallback(staticFileCaseCallback);

  QString appName = Preferences::lpub3dAppName;
  QString iniFile = Preferences::nativePOVIni;
  QString extrasPath = QString("%1/extras").arg(Preferences::lpubDataPath);

  if (!TCUserDefaults::isIniFileSet())
  {
          if (!TCUserDefaults::setIniFile(iniFile.toLatin1().constData()))
                 fprintf(stdout, QString( QString("Could not set INI file: %1").arg(iniFile)).toLatin1().constData());
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
//  setViewMode(LDVPreferences::getViewMode(),
//              LDVPreferences::getLatLongMode(),
//              LDVPreferences::getKeepRightSideUp());

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

void LDVWidget::setupDefaultFormat(void)
{
    defaultFormat.setAlpha(true);
    defaultFormat.setStencil(true);
    defaultFormat.setSampleBuffers(false);
    QGLFormat::setDefaultFormat(defaultFormat);
}

bool LDVWidget::doCommand(QStringList &arguments)
{	
    QString debugMessage = QString("Native command In: %1 %2")
                                   .arg(appArgs.at(0))
                                   .arg(arguments.join(" "));
    // TEMP - REMOVE BEFORE FLIGHT
    fprintf(stdout, debugMessage.toLatin1().constData());

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
    // Copy over the command line arguments (passed in)
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

void LDVWidget::snapshotTakerAlertCallback(TCAlert *alert)
{
	if (alert->getSender() == snapshotTaker)
	{
		if (strcmp(alert->getMessage(), "MakeCurrent") == 0)
		{
			makeCurrent();
			//glEnable(GL_DEPTH_TEST);
		}
	}
}

void LDVWidget::lock(void)
{
    if (lockCount == 0)
    {
        //app->lock();
    }
    lockCount++;
}

void LDVWidget::unlock(void)
{
    lockCount--;
    if (lockCount == 0)
    {
        //app->unlock();
    }
}

void LDVWidget::initializeGL(void)
{
    lock();
    TREGLExtensions::setup();
    ldvPreferences->doCancel();
    if (saving || printing)
    {
        modelViewer->setup();
        modelViewer->openGlWillEnd();
    }
    unlock();
}

void LDVWidget::resizeGL(int width, int height)       // simulate mainWindow Size
{
    lock();
    if (!loading && !saving && !printing)
    {
        width = ldvPreferences->getWindowWidth();
        height = ldvPreferences->getWindowHeight();
        resize(width, height);

        QSize surfaceWindowSize = size();

        modelViewer->setWidth(mwidth=width);
        modelViewer->setHeight(mheight=height);
        glViewport(0, 0, width, height);
        ldvPreferences->setWindowSize(surfaceWindowSize.width(),
                                      surfaceWindowSize.height());
    }
    unlock();
}

void LDVWidget::swap_Buffers(void)
{
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);
    glDisable(GL_MULTISAMPLE_ARB);
    glDrawBuffer(GL_FRONT);
    glDrawBuffer(GL_BACK);
    glFlush();
    glEnable(GL_MULTISAMPLE_ARB);
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
}

void LDVWidget::paintGL(void)
{
    lock();
    if (!painting && (saving || printing || !loading))
    {
        painting = true;
        glEnable(GL_DEPTH_TEST);
        if (saving || printing)
        {
            if (!TREGLExtensions::haveFramebufferObjectExtension())
            {
                glDrawBuffer(GL_BACK);
                glReadBuffer(GL_BACK);
            }
            if (saving) {
                saveImageResult = snapshotTaker->saveImage(saveImageFilename,
                    saveImageWidth, saveImageHeight, saveImageZoomToFit);
            }
            if (printing) {
            }
        }
        else
        {
            makeCurrent();
            redrawRequested = false;   // May not need - not used
            modelViewer->update();
// Not needed
            if (!redrawRequested)
            {
//                killPaintTimer();
//                fps = -1.0f;
            }
            else
            {
//                startPaintTimer();
            }
//            updateFPS();
//            updateLatlong();
              //swap_Buffers();

        }
        painting = false;
    }
    unlock();
}

void LDVWidget::paintEvent(QPaintEvent *event)
{
    lock();
    if (loading && !saving && !printing)
    {
        int r, g, b;

        ldvPreferences->getBackgroundColor(r, g, b);

        // former Qt bug 79310 caused problem with the next 2 lines
        QPainter painter(this);
        painter.fillRect(event->rect(), QColor(r, g, b));
    }
    else if (!saving && !printing)
    {
        QGLWidget::paintEvent(event);
    }
    unlock();
}

bool LDVWidget::staticFileCaseLevel(QDir &dir, char *filename)
{
	int i;
	int len = strlen(filename);
	QString wildcard;
	QStringList files;

	if (!dir.isReadable())
	{
		return false;
	}
	for (i = 0; i < len; i++)
	{
		QChar letter = filename[i];

		if (letter.isLetter())
		{
			wildcard.append('[');
			wildcard.append(letter.toLower());
			wildcard.append(letter.toUpper());
			wildcard.append(']');
		}
		else
		{
			wildcard.append(letter);
		}
	}
	dir.setNameFilters(QStringList(wildcard));
	files = dir.entryList();
	if (files.count())
	{
		QString file = files[0];

		if (file.length() == (int)strlen(filename))
		{
			// This should never be false, but just want to be sure.
			strcpy(filename, file.toLatin1().constData());
			return true;
		}
	}
	return false;
}

bool LDVWidget::staticFileCaseCallback(char *filename)
{
	char *shortName;
	QDir dir;
	char *firstSlashSpot;

	dir.setFilter(QDir::AllEntries | QDir::Readable | QDir::Hidden | QDir::System);
	replaceStringCharacter(filename, '\\', '/');
	firstSlashSpot = strchr(filename, '/');
	if (firstSlashSpot)
	{
		char *lastSlashSpot = strrchr(filename, '/');
		int dirLen;
		char *dirName;

		while (firstSlashSpot != lastSlashSpot)
		{
			char *nextSlashSpot = strchr(firstSlashSpot + 1, '/');

			dirLen = firstSlashSpot - filename + 1;
			dirName = new char[dirLen + 1];
			*nextSlashSpot = 0;
			strncpy(dirName, filename, dirLen);
			dirName[dirLen] = 0;
			if (dirLen)
			{
				dir.setPath(dirName);
				delete dirName;
				if (!staticFileCaseLevel(dir, firstSlashSpot + 1))
				{
					return false;
				}
			}
			firstSlashSpot = nextSlashSpot;
			*firstSlashSpot = '/';
		}
		dirLen = lastSlashSpot - filename;
		dirName = new char[dirLen + 1];
		strncpy(dirName, filename, dirLen);
		dirName[dirLen] = 0;
		dir.setPath(dirName);
		shortName = lastSlashSpot + 1;
		delete dirName;
	}
	else
	{
		shortName = filename;
	}
	return staticFileCaseLevel(dir, shortName);
}

void LDVWidget::setViewMode(LDInputHandler::ViewMode value,
bool examine, bool keep, bool /*saveSettings*/)
{
        viewMode = value;
        if (viewMode == LDInputHandler::VMExamine)
        {
                LDrawModelViewer::ExamineMode examineMode = ( examine ?
                                LDrawModelViewer::EMLatLong : LDrawModelViewer::EMFree );
                inputHandler->setViewMode(LDInputHandler::VMExamine);
                modelViewer->setConstrainZoom(true);
                modelViewer->setExamineMode(examineMode);
        }
        else if (viewMode == LDInputHandler::VMFlyThrough)
        {
                inputHandler->setViewMode(LDInputHandler::VMFlyThrough);
                modelViewer->setConstrainZoom(false);
                modelViewer->setKeepRightSideUp(keep);
        }
        else if (viewMode == LDInputHandler::VMWalk)
        {
                inputHandler->setViewMode(LDInputHandler::VMWalk);
                modelViewer->setKeepRightSideUp(true);
        }
        LDVPreferences::setViewMode(viewMode);
}
