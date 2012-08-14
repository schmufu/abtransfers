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
#include "abt_standingorderinfo.h"
#include "abt_datedtransferinfo.h"

/** \brief Speichert und liefert Einstellungen für das gesamte Programm */

class abt_settings : public QObject
{
Q_OBJECT
private:
	QString m_dataDir;
	QString m_recipientsFilename;
	QString m_accountdataFilename;
	QString m_historyFilename;
	QSettings *settings;
	QList<abt_EmpfaengerInfo*>* m_recipientsList;
	QHash<int, QString> *m_textKeyDescr;

	void loadTextKeyDescriptions();
	void setFilePermissions();

public:
	explicit abt_settings(QObject *parent = 0);
	~abt_settings();

	//get und setter für die Dateinamen
	const QString getRecipientsFilename() const { return this->m_recipientsFilename; }
	const QString getAccountDataFilename() const { return this->m_accountdataFilename; }
	const QString getHistoryFilename() const { return this->m_historyFilename; }
	const QString getDataDir() const { return this->m_dataDir; }

	void setRecipientsFilename(const QString &filename);
	void setAccountDataFilename(const QString &filename);
	void setHistoryFilename(const QString &filename);
	void setDataDir(const QString &dirname);

	const QList<abt_EmpfaengerInfo*>* loadKnownEmpfaenger();
	void saveKnownEmpfaenger();
	const QList<abt_EmpfaengerInfo*>* getKnownRecipients() const { return this->m_recipientsList; }

	const QHash<int, QString> *getTextKeyDescriptions() const;

	void saveWindowStateGeometry(const QByteArray state, const QByteArray geometry);
	QByteArray loadWindowState() const;
	QByteArray loadWindowGeometry() const;


	void saveSelAccountInWidget(const QString &widgetName, const aqb_AccountInfo *acc);
	int loadSelAccountInWidget(const QString &widgetName) const;

	bool showDialog(const QString &dialogType) const;
	void setShowDialog(const QString &dialogType, bool show);

	bool appendJobToOutbox(const QString &jobname) const;
	void setAppendJobToOutbox(const QString &jobname, bool get);

	bool autoAddNewRecipients() const;
	void setAutoAddNewRecipients(bool value);

	//! gibt zurück ob der JobType \a type von AB-Transfers unterstützt wird.
	static int supportedByAbtransfers(const AB_JOB_TYPE type);

	static void resizeColToContentsFor(QTreeWidget *w);

signals:
	void recipientsListChanged();

public slots:
	void onReplaceKnownRecipient(int position, abt_EmpfaengerInfo *newRecipient);
	void addKnownRecipient(abt_EmpfaengerInfo* recipientInfo);
	void deleteKnownRecipient(abt_EmpfaengerInfo* recipientInfo);
};

#endif // TRANS_SETTINGS_H
