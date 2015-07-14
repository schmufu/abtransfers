/******************************************************************************
 * Copyright (C) 2011-2013 Patrick Wacker
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 ******************************************************************************
 * Dont forget: svn propset svn:keywords "Date Author Rev HeadURL" filename
 ******************************************************************************
 * $HeadURL$
 * $Author$
 * $Date$
 * $Rev$
 *
 * description:
 *
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#include <QApplication>
#include <QTextCodec>
#include <QSharedMemory>
#include <QMessageBox>
#include <QTranslator>
#include <QLibraryInfo>
#include "mainwindow.h"
#include "aqb_banking.h"
#include "abt_transaction_base.h"
#include "abt_conv.h"
#include <QDebug>

//global object for AqBanking Access
#define DEFINEGLOBALSHERE
#include "globalvars.h"
#undef DEFINEGLOBALSHERE


/** a message handler to display qDebug(), qWarning() etc. in the debug-Dialog */
#if QT_VERSION >= 0x050000
void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#else
void myMessageHandler(QtMsgType type, const char *msg);
#endif

int main(int argc, char *argv[])
{
	int apprv;
	QApplication app(argc, argv);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#warning "compiling with Qt version >= 5.0.0. This is not tested well!"
#else
	//setCodecForTr() and setCodedForCString() are removed in Qt 5.0.0
	//see http://qt-project.org/doc/qt-5.0/qtdoc/sourcebreaks.html
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	//QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
	QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));
	//QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

	//create the DebugDialogWidget, that displays all qDebug(), qWarning()
	//etc. messages and install the MsgHandler that redirect the messages.
	debugDialog = new DebugDialogWidget();
#if QT_VERSION >= 0x050000
	qInstallMessageHandler(myMessageHandler);
#else
	qInstallMsgHandler(myMessageHandler);
#endif

	#ifdef ABTRANSFER_VERSION
		app.setApplicationVersion(QString::fromUtf8(ABTRANSFER_VERSION));
	#else
	#warning "ABTRANSFER_VERSION not set! Compiling without version information!"
	#endif
	app.setOrganizationName(QString::fromUtf8("Patrick Wacker"));
	app.setOrganizationDomain(QString::fromUtf8("schmufu.dyndns.org"));
	app.setApplicationName(QString::fromUtf8("AB-Transfers"));

	//On MacOS the QSharedMemory seems not to work as assumed.
	//Therefore, as a fast workaround, we do not use the protection on MacOS.
#if !defined(Q_OS_MAC)
	//The program should be started only once!
	//Therefore we create a sharedMemory and check if it already exists.
	//As key for the sharedMemory the program name and the "Key-ID" from
	//my the PGP-Key is used (Patrick Wacker <schmufu.s@gmx.net>)
	QString smKey = app.applicationName();
	smKey.append(QString::fromUtf8("-49E8D03B0700F6C4")); //PGP-Key-ID from Patrick Wacker
	QSharedMemory myMem(smKey);
	if ( !myMem.create(sizeof(int)) ) {
		qDebug() << Q_FUNC_INFO << "SharedMemoryError:" << myMem.errorString();
		if (myMem.error() == QSharedMemory::AlreadyExists) {
			//The program is already running or the last instance
			//was aborted and couldnt free the SharedMemory.
			//We could check if another AB-Transfers process is
			//running, but this differs to much between Linux and
			//Windows. (Windows: SM is freed after crash, Linux:
			//SM is not freed).
			//We simple ask the user.
			int msgRet = QMessageBox::critical(
					NULL,
					QObject::tr("%1 bereits gestartet")
					     .arg(app.applicationName()),
					QObject::tr("Es sieht so aus als würde "
					   "%1 bereits gestartet sein!<br />"
					   "%1 sollte auf keinen Fall mehrmals "
					   "gestartet werden, dies könnte zu "
					   "nicht vorhersagbaren Fehlern "
					   "führen!<br />"
					   "<i>Wenn %1 beim letzten ausführen "
					   "abgestürzt ist, ist es sicher diese "
					   "Abfrage mit \"Ja\" zu quittieren.</i>"
					   "<br /><br />"
					   "Soll %1 wirklich gestartet werden?")
					     .arg(app.applicationName()),
					QMessageBox::Yes | QMessageBox::No,
					QMessageBox::No);

			if (msgRet != QMessageBox::Yes) {
				//cleanup and cancel execution
#if QT_VERSION >= 0x050000
				qInstallMessageHandler(NULL);
#else
				qInstallMsgHandler(NULL);
#endif
				delete debugDialog;
				return 9;
			}
			//else: the program should be started anyway
		}

		//we take over the sharedMemory, so that it is freed at the end
		if (!myMem.attach()) {
			qWarning() << "could not attach SharedMemory!"
				   << "Error:" << myMem.errorString();
		}
	}
#endif // !defined(Q_OS_MAC)


	//creation of global objects
	settings = new abt_settings(); //for program wide settings
	banking = new aqb_banking(); //Initialise and using of AqBanking

	MainWindow *w = new MainWindow();

	qDebug("BEFORE SHOW");
	w->show();
	qDebug("AFTER SHOW");
	apprv = app.exec(); //execute the application
	qDebug("AFTER EXEC");

	//save the current state of the form
	settings->saveWindowStateGeometry(w->saveState(1), w->saveGeometry());
	qDebug("AFTER SAVING STATE");
	delete w; //the MainWindow is no longer needed
	delete banking; //AqBanking is no longer used
	delete settings; //and the settings also.

	//free all created GWEN_STRINGLIST and GWEN_TIME objects
	abt_conv::freeAllGwenLists();

#if QT_VERSION >= 0x050000
	qInstallMessageHandler(NULL);
#else
	qInstallMsgHandler(NULL); //uninstall the MsgHandler
#endif
	delete debugDialog; //and the corresponding dialog

	return apprv;
}

#if QT_VERSION >= 0x050000
void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	//always show the messages at stderr
	fprintf(stderr, "[%s] %s\n", context.function, msg.toStdString().c_str());

	//only show the messages if wanted
	//! @todo implement the option to deactivate debug messges
	//if (!settings->displayDebugMessages()) return;

	switch(type) {
	case QtDebugMsg:
		debugDialog->appendMsg(QString::fromUtf8("DEBUG: ").append(msg);
		break;
	case QtWarningMsg:
		debugDialog->appendMsg(QString::fromUtf8("WARNING: ").append(msg);
		break;
	case QtCriticalMsg:
		debugDialog->appendMsg(QString::fromUtf8("CRITICAL: ").append(msg);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s\n", msg.toStdString().c_str());
		abort();
	}
}
#else
void myMessageHandler(QtMsgType type, const char *msg)
{
	//always show the messages at stderr
	fprintf(stderr, "%s\n", QString::fromUtf8(msg).toStdString().c_str());

	//only show the messages if wanted
	//! @todo implement the option to deactivate debug messges
	//if (!settings->displayDebugMessages()) return;

	switch(type) {
	case QtDebugMsg:
		debugDialog->appendMsg(QString::fromUtf8("DEBUG: ").
				       append(QString::fromUtf8(msg)));
		break;
	case QtWarningMsg:
		debugDialog->appendMsg(QString::fromUtf8("WARNING: ").
				       append(QString::fromUtf8(msg)));
		break;
	case QtCriticalMsg:
		debugDialog->appendMsg(QString::fromUtf8("CRITICAL: ").
				       append(QString::fromUtf8(msg)));
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s\n", msg);
		abort();
	}
}
#endif
