#ifndef PCH_ENABLED
	#include <QCoreApplication>
	#include <QApplication>
	#include <QLocale>
	#include <QFile>
	#include <QString>
	#include <QResource>
	#include <QDir>
	#include <QStringList>
	#include <QSystemTrayIcon>
	#include <QStringList>

	#include <exception>
	#include <iostream>
	#include <cassert>
	#include <stdlib.h>
	#include <stdio.h>
#endif

#include <QApplication>
#include <QProcess>
#include <csignal>

#if !defined(__APPLE__) && !defined(_WIN32)
	#include <sys/prctl.h>
#endif


#ifdef _WIN32
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
	#include <process.h>
#else
	#include <unistd.h>
#endif

#include <AmbilightappConfig.h>

#include <utils/Logger.h>
#include <commandline/Parser.h>
#include <commandline/IntOption.h>
#include <utils/DefaultSignalHandler.h>
#include <db/AuthTable.h>

#include "detectProcess.h"
#include "AmbilightAppDaemon.h"
#include "systray.h"

using namespace commandline;

#if defined(WIN32)

void CreateConsole()
{
	if (!AllocConsole()) {
		return;
	}

	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	SetConsoleTitle(TEXT("Ambilight App"));
}

#endif

#define PERM0664 QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther | QFileDevice::WriteOwner | QFileDevice::WriteGroup

AmbilightAppDaemon* ambilightappd = nullptr;

QCoreApplication* createApplication(int& argc, char* argv[])
{
	bool isGuiApp = false;
	bool forceNoGui = false;

	// command line
	for (int i = 1; i < argc; ++i)
	{
		if (qstrcmp(argv[i], "--desktop") == 0)
		{
			isGuiApp = true;
		}
		else if (qstrcmp(argv[i], "--service") == 0)
		{
			isGuiApp = false;
			forceNoGui = true;
		}
	}

	// on osx/windows gui always available
#if defined(__APPLE__) || defined(_WIN32)
	isGuiApp = true && !forceNoGui;
#else
	if (!forceNoGui)
	{
		isGuiApp = (getenv("DISPLAY") != NULL &&
			((getenv("XDG_SESSION_TYPE") != NULL && strcmp(getenv("XDG_SESSION_TYPE"), "tty") != 0) || getenv("WAYLAND_DISPLAY") != NULL));
		std::cout << ((isGuiApp) ? "GUI" : "Console") << " application: " << std::endl;
	}
#endif

	if (isGuiApp)
	{
		QApplication* app = new QApplication(argc, argv);
		// add optional library path
		app->addLibraryPath(QApplication::applicationDirPath() + "/../lib");
		app->setApplicationDisplayName("AmbilightApp");
#if defined(__APPLE__)
		app->setWindowIcon(QIcon(":/ambilightapp-icon-64px.png"));
#else
		app->setWindowIcon(QIcon(":/ambilightapp-icon-32px.png"));
#endif
		return app;
	}

	QCoreApplication* app = new QCoreApplication(argc, argv);
	app->setApplicationName("AmbilightApp");
	app->setApplicationVersion(AMBILIGHTAPP_VERSION);
	// add optional library path
	app->addLibraryPath(QApplication::applicationDirPath() + "/../lib");

	return app;
}

#ifdef _WIN32
	bool isRunning(const QString& processName) {
		QProcess process;
		process.start("tasklist", QStringList() << "/FI" << QString("IMAGENAME eq %1").arg(processName));
		process.waitForFinished();
		QString output = process.readAllStandardOutput();
		return output.contains(processName, Qt::CaseInsensitive);
	}
#endif

int main(int argc, char** argv)
{

	QStringList params;

	// check if we are running already an instance
	// TODO Allow one session per user
#ifdef _WIN32
	const char* processName = "ambilightapp.exe";
#else
	const char* processName = "ambilightapp";
#endif

	// Initialising QCoreApplication
	QScopedPointer<QCoreApplication> app(createApplication(argc, argv));

	bool isGuiApp = (qobject_cast<QApplication*>(app.data()) != 0 && QSystemTrayIcon::isSystemTrayAvailable());

	DefaultSignalHandler::install();

#ifdef _WIN32
	QProcess::execute("taskkill", QStringList() << "/F" << "/IM" << "MusicLedStudio.exe");

	if (!isRunning("HyperionScreenCap.exe"))
	{
		QString szHyperionScreenCapPath = "C:\\Program Files\\Ambilight App\\Hyperion Screen Capture\\HyperionScreenCap.exe";
		QProcess::startDetached(szHyperionScreenCapPath);
	}
#else
	const char* command = "killall MusicLedStudio";
	system(command);
#endif

	// force the locale
	setlocale(LC_ALL, "C");
	QLocale::setDefault(QLocale::c());

	Parser parser("Ambilight App Daemon");
	parser.addHelpOption();

	BooleanOption& versionOption = parser.add<BooleanOption>(0x0, "version", "Show version information");
	Option& userDataOption = parser.add<Option>('u', "userdata", "Overwrite user data path, defaults to home directory of current user (%1)", QDir::homePath() + "/.ambilightapp");
	BooleanOption& resetPassword = parser.add<BooleanOption>(0x0, "resetPassword", "Lost your password? Reset it with this option back to 'ambilightapp'");
	BooleanOption& deleteDB = parser.add<BooleanOption>(0x0, "deleteDatabase", "Start all over? This Option will delete the database");
	BooleanOption& silentOption = parser.add<BooleanOption>('s', "silent", "Do not print any outputs");
	BooleanOption& verboseOption = parser.add<BooleanOption>('v', "verbose", "Increase verbosity");
	BooleanOption& debugOption = parser.add<BooleanOption>('d', "debug", "Show debug messages");
#ifdef ENABLE_PIPEWIRE
	BooleanOption& pipewireOption = parser.add<BooleanOption>(0x0, "pipewire", "Force pipewire screen grabber if it's available");
#endif
#ifdef WIN32
	BooleanOption& consoleOption = parser.add<BooleanOption>('c', "console", "Open a console window to view log output");
#endif
	parser.add<BooleanOption>(0x0, "desktop", "Show systray on desktop");
	parser.add<BooleanOption>(0x0, "service", "Force AmbilightApp to start as console service");

	/* Internal options, invisible to help */
	BooleanOption& waitOption = parser.addHidden<BooleanOption>(0x0, "wait-ambilightapp", "Do not exit if other AmbilightApp instances are running, wait them to finish");

	parser.process(*qApp);

	if (parser.isSet(versionOption))
	{
		std::cout
			<< "AmbilightApp Ambient Light Deamon" << std::endl
			<< "\tVersion   : " << AMBILIGHTAPP_VERSION << " (" << AMBILIGHTAPP_BUILD_ID << ")" << std::endl
			<< "\tBuild Time: " << __DATE__ << " " << __TIME__ << std::endl;

		return 0;
	}

	if (!parser.isSet(waitOption))
	{
		if (getProcessIdsByProcessName(processName).size() > 1)
		{
			std::cerr << "The Ambilight App Daemon is already running, abort start";
			return 1;
		}
	}
	else
	{
		while (getProcessIdsByProcessName(processName).size() > 1)
		{
			QThread::msleep(100);
		}
	}

#ifdef WIN32
	if (parser.isSet(consoleOption))
	{
		CreateConsole();
	}
#endif

	// initialize main logger and set global log level
	Logger* log = Logger::getInstance("MAIN");
	Logger::setLogLevel(Logger::INFO);

	int logLevelCheck = 0;
	if (parser.isSet(silentOption))
	{
		Logger::setLogLevel(Logger::OFF);
		logLevelCheck++;
	}

	if (parser.isSet(verboseOption))
	{
		Logger::forceVerbose();
		Logger::setLogLevel(Logger::INFO);
		logLevelCheck++;
	}

	if (parser.isSet(debugOption))
	{
		Logger::setLogLevel(Logger::DEBUG);
		logLevelCheck++;
	}

	if (logLevelCheck > 1)
	{
		Error(log, "aborting, because options --silent --verbose --debug can't be used together");
		return 0;
	}

#ifdef ENABLE_PIPEWIRE
	if (parser.isSet(pipewireOption))
	{
		params.append("pipewire");
	}
#endif

	int rc = 1;
	bool readonlyMode = false;

	QString userDataPath(userDataOption.value(parser));

	QDir userDataDirectory(userDataPath);

	QFileInfo dbFile(userDataDirectory.absolutePath() + "/db/ambilightapp.db");

	try
	{
		if (dbFile.exists())
		{
			if (!dbFile.isReadable())
			{
				throw std::runtime_error("Configuration database '" + dbFile.absoluteFilePath().toStdString() + "' is not readable. Please setup permissions correctly!");
			}
			else
			{
				if (!dbFile.isWritable())
				{
					readonlyMode = true;
				}

				Info(log, "Database path: '%s', readonlyMode = %s", QSTRING_CSTR(dbFile.absoluteFilePath()), (readonlyMode) ? "enabled" : "disabled");
			}
		}
		else
		{
			if (!userDataDirectory.mkpath(dbFile.absolutePath()))
			{
				if (!userDataDirectory.isReadable() || !dbFile.isWritable())
				{
					throw std::runtime_error("The user data path '" + userDataDirectory.absolutePath().toStdString() + "' can't be created or isn't read/writeable. Please setup permissions correctly!");
				}
			}
		}

		DBManager::initializeDatabaseFilename(dbFile);

		// reset Password without spawning daemon
		if (parser.isSet(resetPassword))
		{
			if (readonlyMode)
			{
				Error(log, "Password reset is not possible. The user data path '%s' is not writeable.", QSTRING_CSTR(userDataDirectory.absolutePath()));
				throw std::runtime_error("Password reset failed");
			}
			else
			{
				std::unique_ptr<AuthTable> table = std::unique_ptr<AuthTable>(new AuthTable(false));
				if (table->resetAmbilightappUser()) {
					Info(log, "Password reset successful");
					exit(0);
				}
				else {
					Error(log, "Failed to reset password!");
					exit(1);
				}
			}
		}

		// delete database before start
		if (parser.isSet(deleteDB))
		{
			if (readonlyMode)
			{
				Error(log, "Deleting the configuration database is not possible. The user data path '%s' is not writeable.", QSTRING_CSTR(dbFile.absolutePath()));
				throw std::runtime_error("Deleting the configuration database failed");
			}
			else
			{
				if (QFile::exists(dbFile.absoluteFilePath()))
				{
					if (!QFile::remove(dbFile.absoluteFilePath()))
					{
						Info(log, "Failed to delete Database!");
						exit(1);
					}
					else
					{
						Info(log, "Configuration database deleted successfully.");
					}
				}
				else
				{
					Warning(log, "Configuration database [%s] does not exist!", QSTRING_CSTR(dbFile.absoluteFilePath()));
				}
			}
		}

		Info(log, "Starting AmbilightApp - %s", AMBILIGHTAPP_VERSION);
		Debug(log, "QtVersion [%s]", QT_VERSION_STR);

		if (!readonlyMode)
		{
			Info(log, "Set user data path to '%s'", QSTRING_CSTR(userDataDirectory.absolutePath()));
		}
		else
		{
			Warning(log, "The user data path '%s' is not writeable. AmbilightApp starts in read-only mode. Configuration updates will not be persisted!", QSTRING_CSTR(userDataDirectory.absolutePath()));
		}
		
		try
		{
			ambilightappd = new AmbilightAppDaemon(userDataDirectory.absolutePath(), qApp, bool(logLevelCheck), readonlyMode, params, isGuiApp);
		}
		catch (std::exception& e)
		{
			Error(log, "AmbilightApp Daemon aborted: %s", e.what());
			throw;
		}

		// run the application
		if (isGuiApp)
		{
			Info(log, "start systray");
			QApplication::setQuitOnLastWindowClosed(false);
			SysTray* tray = new SysTray(ambilightappd, ambilightappd->getWebPort());
			QObject::connect(qApp, &QGuiApplication::aboutToQuit, tray, &SysTray::deleteLater);
			rc = (qobject_cast<QApplication*>(app.data()))->exec();
		}
		else
		{
			rc = app->exec();
		}
		Info(log, "Application closed with code %d", rc);
		delete ambilightappd;
		ambilightappd = nullptr;
	}
	catch (std::exception& e)
	{
		Error(log, "AmbilightApp aborted: %s", e.what());
	}

	// delete components
	Logger::deleteInstance();

#ifdef _WIN32
	if (parser.isSet(consoleOption))
	{
		system("pause");
	}
#endif

	return rc;
}
