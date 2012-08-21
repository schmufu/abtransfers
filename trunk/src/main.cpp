/******************************************************************************
 * Copyright (C) 2011 Patrick Wacker
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

#include <QtGui/QApplication>
#include <QTextCodec>
#include <QSharedMemory>
#include <QMessageBox>
#include "mainwindow.h"
#include "aqb_banking.h"
#include "abt_transaction_base.h"
#include "abt_conv.h"
#include <QDebug>

//global object for AqBanking Access
#define DEFINEGLOBALSHERE
#include "globalvars.h"
#undef DEFINEGLOBALSHERE


/** a message handler to display the qDebug(), qWarning() etc. in the debug-Dialog */
void myMessageHandler(QtMsgType type, const char *msg);

int main(int argc, char *argv[])
{
	int apprv;
	QApplication app(argc, argv);

	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));

	//qRegisterMetaType<const abt_transaction*>("const abt_transaction*");

	//hier werden alle über qDebug(), qWarning() etc. ausgegebenen Infos angezeigt
	debugDialog = new DebugDialogWidget();
	//damit die Ausgaben auch richtig umgeleitet werden benötigten wir den MsgHandler
	qInstallMsgHandler(myMessageHandler);

	#ifdef ABTRANSFER_VERSION
		app.setApplicationVersion(ABTRANSFER_VERSION);
	#else
	#warning "ABTRANSFER_VERSION not set! Compiling without version information!"
	#endif
	app.setOrganizationName("Patrick Wacker");
	app.setOrganizationDomain("schmufu.dyndns.org");
	app.setApplicationName("AB-Transfers");

	//Das Programm soll nur ein mal gestartet werden!
	//Als key nutzen wir den Programmnamen sowie die "Key-ID" von meinem
	//PGP-Schlüssel (Patrick Wacker <schmufu.s@gmx.net>)
	QString smKey = app.applicationName();
	smKey.append("-49E8D03B0700F6C4"); //PGP Key-ID von Patrick Wacker
	QSharedMemory myMem(smKey);
	if ( !myMem.create(sizeof(int)) ) {
		//SharedMemory konnte nicht erstellt werden
		qDebug() << Q_FUNC_INFO << "SharedMemoryError:" << myMem.errorString();
		if (myMem.error() == QSharedMemory::AlreadyExists) {
			//Das Programm läuft bereits, oder die letzte Instanz
			//ist abgestürzt und konnte den Speicher nicht wieder
			//freigeben!
			//Existiert noch ein anderer AB-Transfers Prozess?
			//--> Hier wird zu viel zwischen Linux und Windows
			//    unterschiedlich! Wir fragen einfach den user!
			// Windows: beim Crash wird der sharedMemory wieder freigeben
			// Linux: sharedMemory bleibt erhalten und muss durch
			//        die Anwendung wieder freigegeben werden.
			int msgRet = QMessageBox::critical(
					NULL,
					QObject::tr("%1 bereits gestartet").arg(app.applicationName()),
					QObject::tr("Es sieht so aus als würde %1 "
					   "bereits gestartet sein!<br />"
					   "%1 sollte auf keinen Fall mehrmals "
					   "gestartet werden, dies könnte zu "
					   "nicht vorhersagbaren Fehlern "
					   "führen!<br />"
					   "<i>Wenn %1 beim letzten ausführen "
					   "abgestürzt ist, ist es sicher diese "
					   "Abfrage mit \"Ja\" zu quittieren.</i>"
					   "<br /><br />"
					   "Soll %1 wirklich gestartet werden?").arg(app.applicationName()),
					QMessageBox::Yes | QMessageBox::No,
					QMessageBox::No);

			if (msgRet != QMessageBox::Yes) {
				//Aufräumen
				qInstallMsgHandler(NULL);
				delete debugDialog;
				return 9; //Abbruch
			}
			//Programm soll trotzdem gestartet werden!
		}

		//wir übernehmen den SharedMemory, damit er beim beenden wieder
		//freigegeben wird und der nächste Start fehlerfrei sein kann.
		if (!myMem.attach()) {
			qWarning() << "could not attach SharedMemory!"
				   << "Error:" << myMem.errorString();
		}
	}


	//globale Objecte erzeugen
	settings = new abt_settings(); //für Programmweite Einstellungen
	banking = new aqb_banking(); //Initialisierung und Verwendung von AqBanking

	MainWindow w;

	//Letzten Zustand wieder herstellen
	qDebug("RESTORING LAST STATE");
	QByteArray ba = settings->loadWindowGeometry();
	if (!ba.isEmpty()) w.restoreGeometry(ba);
	ba = settings->loadWindowState();
	if (!ba.isEmpty()) w.restoreState(ba, 1);

	qDebug("BEFORE SHOW");
	w.show();
	qDebug("AFTER SHOW");
	apprv = app.exec(); //Ausführung der Anwendung
	qDebug("AFTER EXEC");

	//Den Zustand des Forms beim Beenden speichern
	settings->saveWindowStateGeometry(w.saveState(1), w.saveGeometry());
	qDebug("AFTER SAVING STATE");
	delete banking; //AqBanking wird nicht mehr benötigt
	delete settings; //und auch die Einstellungen nicht

	//Alle erstellten GWEN_STRINGLIST und GWEN_TIME Objecte wieder löschen
	abt_conv::freeAllGwenLists();


	//den msgHandler wieder entfernen
	qInstallMsgHandler(NULL);
	 //sowie den Dialog wo u.a. qDebug() Nachrichten angezeigt werden
	delete debugDialog;

	return apprv;
}


void myMessageHandler(QtMsgType type, const char *msg)
{
	fprintf(stderr, "%s\n", msg); //always show the messages at stderr

	//only show the messages if wanted
	//! \todo implement the option to deactivate debug messges
	//if (!settings->displayDebugMessages()) return;

	switch(type) {
	case QtDebugMsg:
		debugDialog->appendMsg(QString("DEBUG: ").append(msg));
		break;
	case QtWarningMsg:
		debugDialog->appendMsg(QString("WARNING: ").append(msg));
		break;
	case QtCriticalMsg:
		debugDialog->appendMsg(QString("CRITICAL: ").append(msg));
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s\n", msg);
		abort();
	}
}
