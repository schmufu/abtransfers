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
#include "mainwindow.h"
#include "aqb_banking.h"
#include "abt_transaction_base.h"
#include "abt_conv.h"
#include <QDebug>

//global object for AqBanking Access
#define DEFINEGLOBALSHERE
#include "globalvars.h"
#undef DEFINEGLOBALSHERE

int main(int argc, char *argv[])
{
	int apprv;
	QApplication app(argc, argv);

	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));

	//qRegisterMetaType<const abt_transaction*>("const abt_transaction*");

	#ifdef ABTRANSFER_VERSION
		app.setApplicationVersion(ABTRANSFER_VERSION);
	#else
	#warning "ABTRANSFER_VERSION not set! Compiling without version information!"
	#endif
	app.setOrganizationName("Patrick Wacker");
	app.setOrganizationDomain("schmufu.dyndns.org");
	app.setApplicationName("aqBanking Transfers");

	//globale Objecte erzeugen
	settings = new abt_settings();
	banking = new aqb_banking();

	MainWindow w;
	debugDialog = new DebugDialogWidget(&w);
	w.show();
	apprv = app.exec();


	delete banking;
	delete settings;

	//Alle erstellten GWEN_STRINGLIST und GWEN_TIME Objecte wieder löschen
	abt_conv::freeAllGwenLists();

	return apprv;
}
