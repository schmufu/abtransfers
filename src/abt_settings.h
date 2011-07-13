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

#ifndef TRANS_SETTINGS_H
#define TRANS_SETTINGS_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QTreeWidget>

#include "abt_empfaengerinfo.h"
#include "aqb_accountinfo.h"

class abt_settings : public QObject
{
Q_OBJECT
private:
	QString knownEmpfaengerFilename;
	//QString SettingsFilename;
	QSettings *Settings;
	QList<abt_EmpfaengerInfo*>* EmpfaengerList;

public:
	explicit abt_settings(QObject *parent = 0);
	~abt_settings();

	QList<abt_EmpfaengerInfo*>* loadKnownEmpfaenger();
	void saveKnownEmpfaenger(const QList<abt_EmpfaengerInfo*> *list);

	//! Erstellt eine Liste alle bekannten Daueraufträge und gibt einen Pointer hierauf zurück
	//! the caller is responsible for freeing the Objects and the list!
	static QList<abt_DAInfo*> *getDAsForAccount(QString &KtoNr, QString &BLZ);
	//! Speichert alle Einträge der Liste für den entsprechenden Account
	static void saveDAsForAccount(QList<abt_DAInfo*> *list, QString &KtoNr, QString &BLZ);
	//! Löscht alle Objekte der Liste sowie die liste selbst
	static void freeDAsList(QList<abt_DAInfo*> *list);

	static void resizeColToContentsFor(QTreeWidget *w);
signals:

public slots:

};

#endif // TRANS_SETTINGS_H