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


/** default regular expression for valid characters at the purpose field. */
static const QString DEFAULT_REGEX_PURPOSE = "[-+ .,/*&%0-9A-Za-z]";

/** default regular expression for valid characters at the recipient and bankname field. */
static const QString DEFAULT_REGEX_RECIPIENT = "[-+ .,/*&%0-9A-Za-z]";


/** \brief saves and restores settings which can be modified by the user */
class abt_settings : public QObject
{
Q_OBJECT
private:
	QString m_dataDir;
	QString m_recipientsFilename;
	QString m_accountdataFilename;
	QString m_historyFilename;
	QString m_autoExportFilename;
	QSettings *settings;
	QList<abt_EmpfaengerInfo*>* m_recipientsList;
	QHash<int, QString> *m_textKeyDescr;

	void loadTextKeyDescriptions();
	void setFilePermissions();

public:
	explicit abt_settings(QObject *parent = nullptr);
	~abt_settings();

	//get und setter für die Dateinamen
	const QString getRecipientsFilename() const { return this->m_recipientsFilename; }
	const QString getAccountDataFilename() const { return this->m_accountdataFilename; }
	const QString getHistoryFilename() const { return this->m_historyFilename; }
	const QString getDataDir() const { return this->m_dataDir; }
	const QString autoExportFilename() const { return this->m_autoExportFilename; }

	void setRecipientsFilename(const QString &filename);
	void setAccountDataFilename(const QString &filename);
	void setHistoryFilename(const QString &filename);
	void setDataDir(const QString &dirname);
	void setAutoExportFilename(const QString &filename);

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

	bool autoExportEnabled() const;
	void setAutoExportEnabled(bool value);
	const QString autoExportProfileName() const;
	void setAutoExportProfileName(const QString name) const;
	const QString autoExportPluginName() const;
	void setAutoExportPluginName(const QString name) const;
	bool autoExportAsTransaction() const;
	void setAutoExportAsTransaction(bool value);

	QStringList getAllProfileFavorites() const;
	bool isProfileFavorit(const QString &name) const;
	void setProfileFavorit(const QString &name, bool favorit);
	void deleteProfileFavorit(const QString &name);

	void setAdvancedOptionEnabled(bool enable);
	bool isAdvancedEnabled() const;
	bool isAdvancedOptionSet(const QString &option) const;
	void setAdvancedOption(const QString &option, bool value);
	void setAdvancedOption(const QString &option, QString value);
	QString getAdvancedOption(const QString &option, const QString defValue = "") const;
	void deleteAdvancedOption(const QString &option);

	QString allowedCharsPurposeRegex() const;
	QString allowedCharsRecipientRegex() const;

	QString language() const;

	void saveColWidth(const QString &name, int col, int width);
	int getColWidth(const QString &name, int col, int def = 100);

	//! returns 1 if the supplied JobType \a type is supported by AB-Transfers
	static int supportedByAbtransfers(const AB_JOB_TYPE type);

	static void resizeColToContentsFor(QTreeWidget *w);


signals:
	void recipientsListChanged();

public slots:
	void onReplaceKnownRecipient(int position, abt_EmpfaengerInfo *newRecipient);
	void addKnownRecipient(abt_EmpfaengerInfo* recipientInfo);
	void deleteKnownRecipient(abt_EmpfaengerInfo* recipientInfo);

	void setLanguage(const QString &language);
};

#endif // TRANS_SETTINGS_H
