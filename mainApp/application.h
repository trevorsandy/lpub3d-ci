/****************************************************************************
**
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
#ifndef APPLICATION_H
#define APPLICATION_H

#include <QScopedPointer>
#include <QException>
#include <QtWidgets>
#include <QFile>
#include <string>
#include <vector>

#ifdef Q_OS_WIN
  #include <Windows.h>
#endif

#include "QsLog.h"
#include "name.h"

#include "lc_global.h"

#include "lc_math.h"

#include "ldrawfiles.h"
#include "metatypes.h"
#include "ranges.h"
#include "meta.h"
#include "step.h"

class NativeOptions;
class InitException: public QException
{
public:
    void raise() const {throw InitException{};}
};

/// The Application class is responsible for further initialization of the app
/// and provides acessors to the current instance and internal resources. It
/// also take cares of shutdown cleanup. An Application class must be
/// instantiaded only once.
class Application : public QObject
{
     Q_OBJECT

public:
    /// Creates the Application.
    Application(int& argc, char **argv);
    ~Application();

    /// Returns a pointer to the current Application instance;
    static Application* instance();

    /// Returns the command line arguments
    QStringList arguments();

    /// Initialize the Application and process the command line arguments.
    int initialize();

    /// This is the equivalent of the main function.
    void mainApp();

    /// Runs the Application and returns the exit code.
    int run();

    /// Clear any allocated memory and close the application
    void shutdown();

    /// Return applicaion launch mode
    bool modeGUI();

    /// Sets the theme
    void setTheme(bool appStarted = true);

    /// Gets the theme
    QString getTheme();

    /// Open project to enable native visual editor or image render
    bool OpenProject(const NativeOptions*, int = NATIVE_VIEW, bool = false);

    /// Flip page size per orientation and return size in pixels
    static int pageSize(PageMeta  &, int which);

    /// Process viewer key to return model, line number and step number
    QStringList getViewerStepKeys(bool modelName = true, bool pliPart = false, const QString &key = "");

    /// Stud sytle and automated edge color
    int     GetStudStyle();
    float   GetPartEdgeContrast();
    float   GetPartColorLightDarkIndex();
    bool    GetAutomateEdgeColor();
    quint32 GetStudCylinderColor();
    quint32 GetPartEdgeColor();
    quint32 GetBlackEdgeColor();
    quint32 GetDarkEdgeColor();

    void    SetStudStyle(const NativeOptions*, bool);
    void    SetAutomateEdgeColor(const NativeOptions*);

    bool setFadeStepsFromCommand();
    bool setHighlightStepFromCommand();

#ifdef Q_OS_WIN
    /// Console redirection for Windows
    void RedirectIOToConsole();

    /// Release the windows console
    int ReleaseConsole(void);

    /// Windows dmp file generation
    static void lcSehInit();
    static LONG WINAPI lcSehHandler(PEXCEPTION_POINTERS exceptionPointers);
#endif

    /// Initialize the splash screen
    QSplashScreen *splash;

    /// lpub3D Preferences
    Preferences lpub3dPreferences;

    /// meta command container
    Meta meta;

    /// Abstract version of page contents
    Page page;

    /// Current step as loaded in the Visual Editor
    Step *currentStep;

    /// Contains MPD or all files used in model
    LDrawFile ldrawFile;

    /// currently loaded CSI in Visual Editor
    QString viewerStepKey;

public slots:
    /// Splash message function to display message updates during startup
    void splashMsg(QString message)
    {
      if (m_console_mode)
      {
          logInfo() << message;
          return;
      }
#ifdef QT_DEBUG_MODE
      logInfo() << message;
#else
      logStatus() << message;
#endif
      splash->showMessage(QSplashScreen::tr(message.toLatin1().constData()),Qt::AlignBottom | Qt::AlignLeft, QColor(QString(SPLASH_FONT_COLOUR)));
      m_application.processEvents();
    }

signals:
    /// Splash message signal to pass messages
    void splashMsgSig(QString message);

private:
    /// Run command line execution
    int processCommandLine();

    /// Run visual editor command line execution
    int Process3DViewerCommandLine();

    bool setPreferredRendererFromCommand(const QString &);

    /// Qt application
    QApplication m_application;

    /// Current application instance
    static Application* m_instance;

    /// Command console mode flag;
    bool m_console_mode;

    /// Print details flag
    bool m_print_output;

    /// File specified on via commandline
    QString m_commandline_file;

    /// Theme set at startup
    QString m_theme;

#ifdef Q_OS_WIN
    /// Windows console information
    CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

    /// Windows standard output handle
    HANDLE h_ConsoleStdOut;
    FILE *f_ConsoleStdOut;

    /// Windows standard error handle
    HANDLE h_ConsoleStdErr;
    FILE *f_ConsoleStdErr;

    /// Windows standard input handle
    HANDLE h_ConsoleStdIn;
    FILE *f_ConsoleStdIn;

    /// Windows console save attributes
    WORD m_currentConsoleAttr;

    /// indicate if using allocated console
    BOOL m_allocate_new_console;

    /// indicate if using parent console
    BOOL m_parent_console;
#endif
};

void clearAndReloadModelFile(bool global = false);
void reloadCurrentPage();
void restartApplication();

extern Application* LPub;

inline Meta& getMetaRef()
{
    return LPub->meta;
}

#endif // APPLICATION_H
