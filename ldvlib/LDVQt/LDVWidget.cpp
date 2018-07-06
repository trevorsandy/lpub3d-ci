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

#include <LDLoader/LDLModel.h>
#include <LDLib/LDrawModelViewer.h>
#include <LDLib/LDSnapshotTaker.h>

#include <TRE/TREMainModel.h>
#ifndef __APPLE__
#include <TRE/TREGLExtensions.h>
#else
#include <LDVSnapshotTaker.h>
#endif

#include <LDVQt/LDVPreferences.h>
#include <LDViewExportOption.h>
#include <LDVAlertHandler.h>

#include <misc.h>
#include <vector>
#include <string>
#include <assert.h>

//#include "lpub_messages.h" // not used
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

LDVWidget::LDVWidget(QWidget *parent)
        :QGLWidget(QGLFormat(QGL::SampleBuffers),parent),
        modelViewer(new LDrawModelViewer(100, 100)),
        snapshotTaker(NULL),
        ldvAlertHandler(new LDVAlertHandler(this)),
        programPath(QCoreApplication::applicationFilePath())
{

  setupLDVFormat();

  QString appName = Preferences::lpub3dAppName;
  TCUserDefaults::setAppName(appName.toLatin1().constData());

  modelViewer->setProgramPath(programPath.toLatin1().constData());

  QFile file(VER_NATIVE_LDV_MESSAGES_FILE);
  QDir::setCurrent(QDir(Preferences::dataLocation).absolutePath());
  if (!file.exists())
  {
        QDir::setCurrent(QDir(QCoreApplication::applicationDirPath()).absolutePath());
  }
  QString messagesPath = QDir::toNativeSeparators(QDir::currentPath() + "/" + file.fileName());
  if (!TCLocalStrings::loadStringTable(messagesPath.toLatin1().constData()))
  {
        fprintf(stdout, "Could not load LDVMessages.ini file %s.\n", messagesPath.toLatin1().constData());
  }

  QFile fontFile(":/resources/SansSerif.fnt");
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
          delete buffer;
      }
  }

  QImage fontImage2x(":/resources/SansSerif@2x.png");
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

  setIniFlag(NativePOVIni, BeforeInit);

  ldvPreferences = new LDVPreferences(parent, this);
  ldvPreferences->doApply();

  LDLModel::setFileCaseCallback(staticFileCaseCallback);

  setFocusPolicy(Qt::StrongFocus);

  ldvWidget = this;

}

LDVWidget::~LDVWidget(void)
{
	TCObject::release(snapshotTaker);
	TCObject::release(modelViewer);
	delete ldvPreferences;

	TCObject::release(ldvAlertHandler);
	ldvAlertHandler = NULL;

	ldvWidget = NULL;
}

bool LDVWidget::setIniFlag(IniFlag iniflag, IniStat iniStat)
{
    iniFlag = iniflag;
    if (!TCUserDefaults::isIniFileSet())
    {
           QString iniFile;
           QString title;
           switch (iniFlag)
           {
                case NativePOVIni:
                    iniFile = Preferences::nativePOVIni;
                    title = "Native POV";
                    break;
                case LDViewPOVIni:
                    iniFile = Preferences::ldviewPOVIni;
                    title = "LDView POV";
                    break;
                case LDViewIni:
                    iniFile = Preferences::ldviewIni;
                    title = "LDView";
                    break;
                default:
                    fprintf(stdout, "Ini file not specified!\n");
                    return false;
           }
           if (!TCUserDefaults::setIniFile(iniFile.toLatin1().constData()))
           {
                fprintf(stdout, "Could not set %s INI file: %s\n",
                        title.toLatin1().constData(),
                        iniFile.toLatin1().constData());
                return false;
           }
           else
           if (iniStat == AfterInit)
           {
                ldvPreferences->doApply();
           }
    }

    return true;
}

bool LDVWidget::doCommand(QStringList &arguments)
{	
    std::string ldvArgs = arguments.join(" ").toStdString();

    QImage studImage(":/resources/StudLogo.png");
    TREMainModel::setRawStudTextureData(studImage.bits(),studImage.byteCount());

    TCUserDefaults::setCommandLine(ldvArgs.c_str());

    LDLModel::setFileCaseCallback(staticFileCaseCallback);

    bool retValue;
#ifndef __APPLE__
    retValue = LDSnapshotTaker::doCommandLine(false, true);
#else
    LDVSnapshotTaker *snapshotTaker = new LDVSnapshotTaker();
    retValue = snapshotTaker->doCommandLine();
            TCObject::release(snapshotTaker);
#endif
    if (!retValue)
    {
         fprintf(stdout, "Failed to processs Native command arguments: %s\n", ldvArgs.c_str());
         fflush(stdout);
    }
    return retValue;
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
  ldvPreferences = new LDVPreferences(this, this);

  if (ldvPreferences->exec() == QDialog::Rejected)
    ldvPreferences->doCancel();
  else
    ldvPreferences->doApply();
}

void LDVWidget::setupLDVFormat(void)
{
    ldvFormat.setAlpha(true);
    ldvFormat.setStencil(true);
    QGLFormat::setDefaultFormat(ldvFormat);
}

void LDVWidget::modelViewerAlertCallback(TCAlert *alert)
{
    if (alert)
    {
        fprintf(stdout, "%s\n", alert->getMessage());
        fflush(stdout);
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

#ifndef __APPLE__
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
#endif
    }
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

void LDVWidget::initializeGL(void)
{
    makeCurrent();
#ifndef __APPLE__
    TREGLExtensions::setup();
    ldvPreferences->doCancel();
#endif
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
#ifndef __APPLE__
    if (!TREGLExtensions::haveFramebufferObjectExtension())
    {
         glDrawBuffer(GL_BACK);
         glReadBuffer(GL_BACK);
    }
#endif
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
