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

#ifndef ABT_SETTINGS_H
#define ABT_SETTINGS_H

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
	QString m_dataDir;
	//QString SettingsFilename;
	QSettings *Settings;
	QList<abt_EmpfaengerInfo*>* EmpfaengerList;
	QHash<int, QString> *m_textKeyDescr;

	void loadTextKeyDescriptions();

public:
	explicit abt_settings(QObject *parent = 0);
	~abt_settings();

	const QList<abt_EmpfaengerInfo*>* loadKnownEmpfaenger();
	void saveKnownEmpfaenger(const QList<abt_EmpfaengerInfo*> *list);

	const QString *getDataDir() const;

	const QHash<int, QString> *getTextKeyDescriptions() const;

	//! Erstellt eine Liste alle bekannten Daueraufträge und gibt einen Pointer hierauf zurück
	//! the caller is responsible for freeing the Objects and the list!
	QList<abt_StandingInfo*> *getStandingOrdersForAccount(const QString &KtoNr, const QString &BLZ);
	//! \overload
	QList<abt_StandingInfo*> *getStandingOrdersForAccount(const aqb_AccountInfo *a);

	void saveStandingOrder(const abt_transaction *t);
	void saveStandingOrder(const AB_TRANSACTION *t);
	void deleteStandingOrder(const abt_transaction *t);
	void deleteStandingOrder(const AB_TRANSACTION *t);
	//! Löscht alle Objekte der Liste sowie die liste selbst
	static void freeStandingOrdersList(QList<abt_StandingInfo*> *list);



	//! Erstellt eine Liste alle bekannten Terminüberweisungen und gibt einen Pointer hierauf zurück
	//! the caller is responsible for freeing the Objects and the list!
	QList<abt_DatedInfo*> *getDatedTransfersForAccount(const QString &KtoNr, const QString &BLZ);
	//! \overload
	QList<abt_DatedInfo*> *getDatedTransfersForAccount(const aqb_AccountInfo *a);

	void saveDatedTransfer(const abt_transaction *t);
	void saveDatedTransfer(const AB_TRANSACTION *t);
	void deleteDatedTransfer(const abt_transaction *t);
	void deleteDatedTransfer(const AB_TRANSACTION *t);
	//! Löscht alle Objekte der Liste sowie die liste selbst
	static void freeDatedTransfersList(QList<abt_DatedInfo*> *list);

	static void resizeColToContentsFor(QTreeWidget *w);

	void saveWindowStateGeometry(const QByteArray state, const QByteArray geometry);
	QByteArray loadWindowState();
	QByteArray loadWindowGeometry();

signals:

public slots:

};

#endif // TRANS_SETTINGS_H
