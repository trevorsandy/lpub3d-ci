/****************************************************************************
**
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtOpenGL>
#include <QPushButton>
#ifdef Q_OS_WIN
#include <tchar.h>
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5,9,0)
#include <QOperatingSystemVersion>
#else
#include <QSysInfo>
#endif
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
//#include "lc_global.h"
#include "declarations.h"
#include "version.h"
#include "lpub_preferences.h"
#include "lpub_object.h"
//#include "qsimpleupdater.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->contentGroupBox->hide();

    ui->version->setTextFormat(Qt::RichText);

    ui->version->setText(tr("%1 <b>%2</b> %3Build <b>%4</b> SHA <b>%5</b>")
                         .arg(QString::fromLatin1(VER_BUILD_TYPE_STR),
                              QString::fromLatin1(VER_PRODUCTVERSION_STR),
                              QString::fromLatin1(VER_REVISION_STR).toInt() ? tr("Revision <b>%1</b> ").arg(QString::fromLatin1(VER_REVISION_STR)) : QString(),
                              QString::fromLatin1(VER_COMMIT_STR),
                              QString::fromLatin1(VER_GIT_SHA_STR)));
    ui->description->setTextFormat(Qt::RichText);
    ui->description->setText(tr("<p><b>%1</b> an LDraw&trade; building instruction editor.<br><br>"
                                "%2</p>").arg(QString::fromLatin1(VER_PRODUCTNAME_STR),
                                              QString::fromLatin1(VER_LEGALCOPYRIGHT_STR)));

    ui->url->setTextFormat(Qt::RichText);
    ui->url->setText(tr("Homepage: <a href=\"%1\">%1</a>").arg(QString::fromLatin1(VER_HOMEPAGE_GITHUB_STR)));

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QString AboutFormat = tr("<table style=\"width:100%\">"
                             "<tr>"
                               "<td>"
                                  "<table>"
                                     "<tr>"
                                       "<td>"
                                         "<b>%1</b> is a free WYSIWYG editing application for<br>"
                                         "creating LEGO&copy; style digital building instructions<br>"
                                         "for LDraw&trade; based digital models.<br><br>"
                                         "%1 uses components from LPub&copy;,<br>"
                                         "LeoCAD&copy; and other 3rd party contributors.<br>"
                                       "</td>"
                                       "<td valign=\"middle\">"
                                         "<a href=\"%2\"><img src=\":/resources/gplv3.png\"></a>"
                                       "</td>"
                                    "</tr>"
                                  "</table>"
                                "</td>"
                             "</tr>"
                             "</table>");
    QString About = AboutFormat.arg(QString::fromLatin1(VER_PRODUCTNAME_STR),
                                    QString::fromLatin1(VER_LICENSE_INFO_STR));

    ui->AppInfo->setTextFormat(Qt::RichText);
    ui->AppInfo->setOpenExternalLinks(true);
    ui->AppInfo->setText(About);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    const
    QString BuildInfoFormat = tr("<table style=""width:100%"">"
                                 "<tr>"
                                   "<td>Compiled on:</td>"
                                   "<td>%1</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>Compiled for:</td>"
                                   "<td>%2</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>Compiled with:</td>"
                                   "<td>%3</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>Dev Environment:</td>"
                                   "<td>%4</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>LeoCAD Version:</td>"
                                   "<td>%5</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>Build Date:</td>"
                                   "<td>%6</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td></td>"
                                   "<td valign=\"right\">"
                                     "<a href=\"%7\"><img src=\":/resources/builtwithqt.png\"></a>"
                                   "</td>"
                                 "</tr>"
                               "</table>");
    const
    QString BuildInfo = BuildInfoFormat.arg(QString::fromLatin1(VER_COMPILED_ON),
                                            QString::fromLatin1(VER_COMPILED_FOR),
                                            QString::fromLatin1(VER_COMPILED_WITH).replace("qtver",qVersion()),
                                            QString::fromLatin1(VER_IDE),
                                            tr("%1 - Commit %2").arg(QString::fromLatin1(VER_LC_VERSION_TEXT),
                                                                     QString::fromLatin1(VER_LC_VERSION_SHA)),
                                            QString::fromLatin1(VER_COMPILE_DATE_STR),
                                            QString::fromLatin1(VER_QT_URL));

    ui->BuildInfo->setTextFormat(Qt::RichText);
    ui->BuildInfo->setText(BuildInfo);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QString osPlatform;
#if defined(Q_OS_WIN)
    int OS = checkOS();
    if (OS == Win_64)
        osPlatform = tr("64-bit system");
    else if(OS == Win_32)
        osPlatform = tr("32-bit system");
    else if (OS == OsOther)
        osPlatform = tr("Other Operating System");
    else if (OS == OsError)
        osPlatform = tr("Error encountered");
    else
        osPlatform = tr("Cannot determine system");
#elif defined(Q_OS_MACX)
    osPlatform = QSysInfo::currentCpuArchitecture();
#elif defined(Q_OS_LINUX)
#if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
    osPlatform = QSysInfo::currentCpuArchitecture();
#endif
#else
    osPlatform = "Cannot determine system";
#endif

    const
    QString OsInfoFormat = tr("<table style=""width:100%"">"
                              "<tr>"
                                "<td>Name:</td>"
                                "<td>%1</td>"
                              "</tr>"
                              "<tr>"
                                "<td>Architecture:</td>"
                                "<td>%2</td>"
                              "</tr>"
                              "<tr>"
                                "<td>OpenGL Version:&nbsp;</td>"
                                "<td>%3<br>%4 - %5</td>"
                              "</tr>"
                            "</table>");
    const
    QString OsInfo = OsInfoFormat.arg(osName(),
                                      osPlatform,
                                      QString((const char*)glGetString(GL_VERSION)),
                                      QString((const char*)glGetString(GL_RENDERER)).replace("(R)", "&reg;"),
                                      QString((const char*)glGetString(GL_VENDOR)));

    ui->OsInfo->setTextFormat(Qt::RichText);
    ui->OsInfo->setText(OsInfo);

    // QTextBrowser content management
    ui->contentEdit->setWordWrapMode(QTextOption::WordWrap);
    ui->contentEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
    ui->contentEdit->setLineWrapColumnOrWidth(LINE_WRAP_WIDTH);
    ui->contentEdit->setOpenExternalLinks(true);

    //buttonBar additions
    detailsButton = new QPushButton(tr("Version Details..."));
    detailsButton->setDefault(true);
    ui->buttonBox->addButton(detailsButton,QDialogButtonBox::ActionRole);

    connect(detailsButton,SIGNAL(clicked(bool)),
            this,SLOT(showChangeLogDetails(bool)));

    creditsButton = new QPushButton(tr("Credits..."));
    creditsButton->setDefault(true);
    ui->buttonBox->addButton(creditsButton,QDialogButtonBox::ActionRole);

    connect(creditsButton,SIGNAL(clicked(bool)),
            this,SLOT(showCreditDetails(bool)));

    changeLogLoaded = false;

    setSizeGripEnabled(true);

    adjustSize();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

// populate readme
void AboutDialog::updateChangelog () {
    ui->contentGroupBox->setTitle(LPub::m_versionInfo);
    if (LPub::m_setReleaseNotesAsText)
        ui->contentEdit->setPlainText(LPub::m_releaseNotesContent);
    else
        ui->contentEdit->setHtml(LPub::m_releaseNotesContent);
    changeLogLoaded = true;
}

void AboutDialog::showChangeLogDetails(bool) {
    if (ui->contentGroupBox->isHidden()) {
        //populate readme
        if (!changeLogLoaded)
            updateChangelog ();
        ui->contentGroupBox->show();
        this->adjustSize();
    } else {
        ui->contentGroupBox->hide();
        this->adjustSize();
    }
}

void AboutDialog::showCreditDetails(bool clicked) {
    Q_UNUSED(clicked);

    //populate credits
    const QString creditsFile = QString("%1/CREDITS.txt").arg(Preferences::lpub3dDocsResourcePath);

    QFile file(creditsFile);
    if (! file.open(QFile::ReadOnly | QFile::Text)) {
        ui->contentEdit->setPlainText( tr("Failed to open Credits file: \n%1:\n%2")
                                               .arg(creditsFile, file.errorString()));
    } else {
        QTextStream in(&file);
        ui->contentEdit->setPlainText(in.readAll());
        changeLogLoaded = false;
    }

    if (ui->contentGroupBox->isHidden()) {
        ui->contentGroupBox->show();
        this->adjustSize();
    } else {
        ui->contentGroupBox->hide();
        this->adjustSize();
    }
}

QString AboutDialog::osName() const
{
#if QT_VERSION >= QT_VERSION_CHECK(5,9,0)
    return QSysInfo::prettyProductName();
#else
  #ifdef Q_OS_LINUX
    #if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
       return QSysInfo::productVersion();
    #else
       return "unknown";
    #endif
  #endif

  #ifdef Q_OS_WIN
    #if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
        switch(QSysInfo::windowsVersion())
        {
          case QSysInfo::WV_5_1: return "Windows XP";
          case QSysInfo::WV_5_2: return "Windows 2003";
          case QSysInfo::WV_6_0: return "Windows Vista";
          case QSysInfo::WV_6_1: return "Windows 7";
          case QSysInfo::WV_6_2: return "Windows 8";
          case QSysInfo::WV_6_3: return "Windows 8.1";
          case QSysInfo::WV_10_0: return "Windows 10";
          default: return "Windows";
        }
    #else
        switch(QSysInfo::windowsVersion())
        {
          case QSysInfo::WV_5_1: return "Windows XP";
          case QSysInfo::WV_5_2: return "Windows 2003";
          case QSysInfo::WV_6_0: return "Windows Vista";
          case QSysInfo::WV_6_1: return "Windows 7";
          case QSysInfo::WV_6_2: return "Windows 8";
          case QSysInfo::WV_6_3: return "Windows 8.1 or later";
          default: return "Windows";
        }
    #endif
  #endif

  #ifdef Q_OS_MACOS
    #if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
       switch(QSysInfo::MacintoshVersion)
       {
         case QSysInfo::MV_LION:         return "OS X Lion";              // Version 10.7
         case QSysInfo::MV_MOUNTAINLION: return "OS X Mountain Lion";     // Version 10.8
         case QSysInfo::MV_MAVERICKS:    return "OS X Mavericks";         // version 10.9
         case QSysInfo::MV_YOSEMITE:     return "OS X Yosemite";          // Version 10.10
         case QSysInfo::MV_ELCAPITAN:    return "OS X El Capitan";        // Version 10.11
         case QSysInfo::MV_SIERRA:       return "MacOS Sierra";           // Version 10.12
         default: return "MacOS";
       }
    #else
       switch(QSysInfo::MacintoshVersion)
       {
         case QSysInfo::MV_LION:         return "OS X Lion";              // Version 10.7
         case QSysInfo::MV_MOUNTAINLION: return "OS X Mountain Lion";     // Version 10.8
         case QSysInfo::MV_MAVERICKS:    return "OS X Mavericks";         // version 10.9
         case QSysInfo::MW_YOSEMITE:     return "OS X Yosemite or later"; // Version 10.10
         default: return "OS X";
       }
    #endif
  #endif
#endif
}

#ifdef Q_OS_WIN
OsType AboutDialog::checkOS()
{
#ifndef Q_OS_WIN32
    return OsOther;
#else
    // An application compiled for 64 bits can only run on a 64 bit os,
    // so no need to check any further.
    if (QSysInfo::WordSize == 64) return Win_64;
    // A 32 bit application may be running on a 64 bit OS.
    BOOL is64 = FALSE;
    // IsWow64Process may not be available in kernel32 on all Windows versions, so we bind to it
    // at runtime.
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)
            GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    // No way it's a 64 bit OS if it doesn't have this API.
    if (fnIsWow64Process == nullptr) return Win_32;
    // Note that GetCurrentProcess() can't fail.
    if (!IsWow64Process(GetCurrentProcess(), &is64)) return OsError; // The check has failed.
    return is64 ? Win_64 : Win_32;
#endif
}
#endif
