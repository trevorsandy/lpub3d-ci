#include <QApplication>
#include "application.h"
#include "version.h"
#include "lc_application.h"

#if defined AUTO_RESTART && AUTO_RESTART == 1
static int lpub3dMain(int &argc, char **argv)
#else
int main(int argc, char** argv)
#endif // AUTO_RESTART
{
	QCoreApplication::setOrganizationDomain(QLatin1String(VER_COMPANYDOMAIN_STR));
	QCoreApplication::setOrganizationName(  QLatin1String(VER_COMPANYNAME_STR));
	QCoreApplication::setApplicationVersion(QLatin1String(VER_PRODUCTVERSION_STR));

	lcCommandLineOptions Options;

	Application::initializeSurfaceFormat(argc, argv, Options);

	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

#ifdef Q_OS_MACOS
	QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

	Application app(argc, argv);

	int rc = EXIT_SUCCESS;

	try
	{
		rc = app.initialize(Options);
	}
	catch(const InitException &ex)
	{
		fprintf(stderr, "%s\n", qUtf8Printable(QObject::tr("Could not initialize %1. Exception: %2.").arg(VER_PRODUCTNAME_STR).arg(ex.what())));
		rc = EXIT_FAILURE;
	}
	catch(...)
	{
		fprintf(stderr, "%s\n", qUtf8Printable(QObject::tr("A fatal %1 error ocurred.").arg(VER_PRODUCTNAME_STR)));
		rc = EXIT_FAILURE;
	}

	if (rc == RUN_APPLICATION)
	{
		return app.run();
	}
	else
	{
		return rc;
	}
}

// https://github.com/KubaO/stackoverflown/tree/master/questions/appmonitor-37524491
#if defined AUTO_RESTART && AUTO_RESTART == 1

static char const kRunLPub3D[] = "run__lpub3d";
static char const kRunLPub3DValue[] = "lpub3d__running";

#if defined(Q_OS_WIN32)
static QString getWindowsCommandLineArguments()
{
	const wchar_t *args = GetCommandLine();
	bool oddBackslash = false, quoted = false, whitespace = false;
	// skip the executable name according to Windows command line parsing rules
	while (auto c = *args) {
		if (c == L'\\')
			oddBackslash ^= 1;
		else if (c == L'"')
			quoted ^= !oddBackslash;
		else if (c == L' ' || c == L'\t')
			whitespace = !quoted;
		else if (whitespace)
			break;
		else
			oddBackslash = false;
		args++;
	}

	return QString::fromRawData(reinterpret_cast<const QChar*>(args), lstrlen(args));
}
#endif

static int lpub3dMonitorMain(int &argc, char **argv)
{
#if !defined(Q_OS_WIN32)
	QStringList args;
	args.reserve(argc-1);
	for (int i = 1; i < argc; ++i)
		args << QString::fromLocal8Bit(argv[i]);
#else
	QString args = getWindowsCommandLineArguments();  // pass command line arguments natively
#endif

	QCoreApplication app { argc, argv };
	QProcess proc;
	auto onFinished = [&](int retcode, QProcess::ExitStatus status) {
		bool abend = status == QProcess::CrashExit;
		QString const message = QObject::tr("%1 ExitStatus: %2(%3)")
											.arg(VER_PRODUCTNAME_STR)
											.arg(abend ? "Abnormal" : "Normal")
											.arg(status);
#ifdef QT_DEBUG_MODE
		qDebug() << qUtf8Printable(message);
#else
		fprintf(stdout, "%s\n", qUtf8Printable(message));
#endif
		if (abend) {
#if defined(Q_OS_WIN32)
			proc.setNativeArguments(args + " " + RESTART_NOTICE);
#else
			proc.setArguments(QStringList(args) << RESTART_NOTICE);
#endif
			proc.start();                             // restart the app if the app crashed
		} else
			app.exit(retcode);                        // no restart required
	};
	QObject::connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), onFinished);

	auto env = QProcessEnvironment::systemEnvironment();
	env.insert(kRunLPub3D, kRunLPub3DValue);
	proc.setProgram(app.applicationFilePath());       // lpub3d and lpub3dMonitor are the same executable
#if defined(Q_OS_WIN32)
	SetErrorMode(SEM_NOGPFAULTERRORBOX);              // disable Windows error reporting
	proc.setNativeArguments(args);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
	env.insert("QT_ASSUME_STDERR_HAS_CONSOLE ", "1"); // ensure that the debug output gets passed along
#else
	env.insert("QT_LOGGING_TO_CONSOLE", "1");
#endif
#else
	proc.setArguments(args);
#endif
	proc.setProcessEnvironment(env);
	proc.setProcessChannelMode(QProcess::ForwardedChannels);
	proc.start();

	return app.exec();
}

int main(int argc, char **argv)
{
	bool ok;
	bool autoRestart = QString(qgetenv("LPUB3D_AUTO_RESTART_ENABLED")).toInt(&ok);
	if (ok && autoRestart && qgetenv(kRunLPub3D) != kRunLPub3DValue)
		return lpub3dMonitorMain(argc, argv);
	else
		return qunsetenv(kRunLPub3D), lpub3dMain(argc, argv);
}
#endif // AUTO_RESTART
