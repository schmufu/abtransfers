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

#include "abt_settings.h"

#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QTextCodec>
#include "globalvars.h"

#define SETTINGS_STANDARD_FOLDER "/.abtransfers"
#define SETTINGS_INIFILENAME "settings.ini"
#define SETTINGS_STANDARD_RECIPIENTS_FILENAME "recipients.txt"
#define SETTINGS_STANDARD_ACCOUNTDATA_FILENAME "accountdata.ctx"
#define SETTINGS_STANDARD_HISTORY_FILENAME "history.ctx"


abt_settingsPrivate::abt_settingsPrivate()
{
	qDebug() << Q_FUNC_INFO << "created";

	//the standard folder is .abtransfers in the home directory
	//there is always the settings.ini

	//if the folder does not exist, we create it
	QDir dataStorage(QDir::toNativeSeparators(QDir::homePath() +
						  SETTINGS_STANDARD_FOLDER));
	if (!dataStorage.exists()) {
		bool ret = dataStorage.mkpath(dataStorage.absolutePath());
		if (!ret) {
			qWarning() << Q_FUNC_INFO
				   << "could not create default folder:"
				   << dataStorage.absolutePath();
			//we continue anyway, the default values are useable
		}
	}

	QString iniFilename = QDir::homePath() +
			      SETTINGS_STANDARD_FOLDER + "/" +
			      SETTINGS_INIFILENAME;
	//the path must be useable on all systems
	iniFilename = QDir::toNativeSeparators(iniFilename);

	this->settings = new QSettings(iniFilename, QSettings::IniFormat);
	this->settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

	this->recipientsList = new QList<abt_EmpfaengerInfo*>;
	this->textKeyDescriptions = new QHash<int, QString>;
}

abt_settingsPrivate::~abt_settingsPrivate()
{
	delete this->settings;

	//free all known recipients
	while (this->recipientsList->size()) {
		delete this->recipientsList->takeFirst();
	}
	//as well as the list itself
	delete this->recipientsList;
	delete this->textKeyDescriptions;

	qDebug() << Q_FUNC_INFO << "executed";
}


abt_settings::abt_settings(QObject *parent) :
	QObject(parent),
	d(new abt_settingsPrivate)
{
	qDebug() << Q_FUNC_INFO << "created";

	//depending on the user-selection the data-files can be stored in other
	//places. We load this values or the default ones if they not exists.
	QString defValue;

	//Standard folder to store data
	defValue = QDir::homePath() + SETTINGS_STANDARD_FOLDER + "/";
	defValue = QDir::toNativeSeparators(defValue);
	this->d->dataDir = this->d->settings->value("Main/DataDir", defValue).toString();

	//file for known recipients
	defValue = QDir::homePath() + SETTINGS_STANDARD_FOLDER + "/";
	defValue.append(SETTINGS_STANDARD_RECIPIENTS_FILENAME);
	defValue = QDir::toNativeSeparators(defValue);
	this->d->recipientsFilename =
		this->d->settings->value("Main/RecipientsFilename", defValue).toString();

	//file for account data (balance, standing-/dated-transfers)
	defValue = QDir::homePath() + SETTINGS_STANDARD_FOLDER + "/";
	defValue.append(SETTINGS_STANDARD_ACCOUNTDATA_FILENAME);
	defValue = QDir::toNativeSeparators(defValue);
	this->d->accountdataFilename =
		this->d->settings->value("Main/AccountDataFilename", defValue).toString();

	//file for the history
	defValue = QDir::homePath() + SETTINGS_STANDARD_FOLDER + "/";
	defValue.append(SETTINGS_STANDARD_HISTORY_FILENAME);
	defValue = QDir::toNativeSeparators(defValue);
	this->d->historyFilename =
		this->d->settings->value("Main/HistoryFilename", defValue).toString();

	this->loadTextKeyDescriptions();

	//ensure that all file permissions are correct
	this->setFilePermissions();
}

abt_settings::~abt_settings()
{
	//save all known recipients
	this->saveKnownEmpfaenger();

	//ensure that all file permissions are correct
	this->setFilePermissions();

	qDebug() << Q_FUNC_INFO << "deleted";
}

const QString abt_settings::getRecipientsFilename() const
{
	return this->d->recipientsFilename;
}

const QString abt_settings::getAccountDataFilename() const
{
	return this->d->accountdataFilename;
}

const QString abt_settings::getHistoryFilename() const
{
	return this->d->historyFilename;
}

const QString abt_settings::getDataDir() const
{
	return this->d->dataDir;
}

//private
void abt_settings::loadTextKeyDescriptions()
{
	if (this->d->textKeyDescriptions == NULL) {
		this->d->textKeyDescriptions = new QHash<int, QString>;
	}

	this->d->textKeyDescriptions->clear();

	if (!this->d->settings->childGroups().contains("TextKeyDescriptions")) {
		//TextKexDescriptions doesnt exists, use default values
		this->d->settings->beginGroup("TextKeyDescriptions");
		this->d->settings->setValue("04", "Lastschrift (Abbuchungsauftragsverfahren)");
		this->d->settings->setValue("05", "Lastschrift (Einzugsermächtigungsverfahren)");
		this->d->settings->setValue("51", "Überweisung");
		this->d->settings->setValue("52", "Dauerauftrags-Überweisung");
		this->d->settings->setValue("53", "Lohn-, Gehalts-, Renten-Überweisung");
		this->d->settings->setValue("54", "Vermögenswirksame Leistung (VL)");
		this->d->settings->setValue("56", "Überweisung öffentlicher Kassen");
		this->d->settings->setValue("67", "Überweisung mit prüfziffergesicherten Zuordnungsdaten (BZÜ)");
		this->d->settings->setValue("69", "Spendenüberweisung");
		this->d->settings->endGroup();
	}
	
	this->d->settings->beginGroup("TextKeyDescriptions");
	//go through all keys and store the values
	//in the hash d->textKeyDescriptions
	foreach (QString key, this->d->settings->allKeys()) {
		QString text = this->d->settings->value(key, tr("Unbekannt")).toString();
		this->d->textKeyDescriptions->insert(key.toInt(), text);
	}

	this->d->settings->endGroup();
}

//private
void abt_settings::setFilePermissions()
{
	bool ret;
	QString iniFilename = QDir::homePath() +
			      SETTINGS_STANDARD_FOLDER + "/" +
			      SETTINGS_INIFILENAME;
	//the path must be useable on all systems
	iniFilename = QDir::toNativeSeparators(iniFilename);

	//ensure file permissions are correct
	ret = QFile::setPermissions(iniFilename,
				    QFile::ReadOwner | QFile::ReadUser |
				    QFile::WriteOwner | QFile::WriteUser);
	if (!ret) qWarning() << Q_FUNC_INFO << "setting permissions on"
			     << iniFilename << "failed";

	ret = QFile::setPermissions(this->getAccountDataFilename(),
				    QFile::ReadOwner | QFile::ReadUser |
				    QFile::WriteOwner | QFile::WriteUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions on"
			     << this->getAccountDataFilename() << "failed";

	ret = QFile::setPermissions(this->getHistoryFilename(),
				    QFile::ReadOwner | QFile::ReadUser |
				    QFile::WriteOwner | QFile::WriteUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions on"
			     << this->getHistoryFilename() << "failed";

	ret = QFile::setPermissions(this->getRecipientsFilename(),
				    QFile::ReadOwner | QFile::ReadUser |
				    QFile::WriteOwner | QFile::WriteUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions  on"
			     << this->getRecipientsFilename() << "failed";

	//also the permissions fpr the folder should be correct
	ret = QFile::setPermissions(QDir::toNativeSeparators(QDir::homePath() +
							     SETTINGS_STANDARD_FOLDER),
				    QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
				    QFile::ReadUser | QFile::WriteUser | QFile::ExeUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions on folder"
			     << QDir::toNativeSeparators(QDir::homePath() +
							 SETTINGS_STANDARD_FOLDER)
			     << "failed";

	ret = QFile::setPermissions(this->getDataDir(),
				    QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
				    QFile::ReadUser | QFile::WriteUser | QFile::ExeUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions on folder"
			     << this->getDataDir() << "failed";

}

const QHash<int, QString> *abt_settings::getTextKeyDescriptions() const
{
	return this->d->textKeyDescriptions;
}

const QList<abt_EmpfaengerInfo*>* abt_settings::loadKnownEmpfaenger()
{
	this->d->recipientsList->clear();

	QFile file(this->d->recipientsFilename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return NULL;

	QTextStream in(&file);
	QStringList infoStringList;
	abt_EmpfaengerInfo *recipientInfo;
	while (!in.atEnd()) {
		QString line = in.readLine();
		infoStringList = line.split("\t", QString::KeepEmptyParts);

		recipientInfo = new abt_EmpfaengerInfo();
		recipientInfo->setName(infoStringList.at(0));
		recipientInfo->setKontonummer(infoStringList.at(1));
		recipientInfo->setBLZ(infoStringList.at(2));
		recipientInfo->setIBAN(infoStringList.at(3));
		recipientInfo->setBIC(infoStringList.at(4));
		recipientInfo->setInstitut(infoStringList.at(5));
		recipientInfo->setVerw(infoStringList.at(6));

		this->d->recipientsList->append(recipientInfo);
	}

	file.close();
	return this->d->recipientsList;
}

void abt_settings::saveKnownEmpfaenger()
{
	abt_EmpfaengerInfo *recipientInfo;

	QFile file(this->d->recipientsFilename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);

	for (int i=0; i<this->d->recipientsList->size(); ++i) {
		recipientInfo = this->d->recipientsList->at(i);
		out << recipientInfo->getName() << "\t";
		out << recipientInfo->getKontonummer() << "\t";
		out << recipientInfo->getBLZ() << "\t";
		out << recipientInfo->getIBAN() << "\t";
		out << recipientInfo->getBIC() << "\t";
		out << recipientInfo->getInstitut() << "\t";
		out << recipientInfo->getVerw() << "\n";
	}

	file.close();
}

const QList<abt_EmpfaengerInfo *> *abt_settings::getKnownRecipients() const
{
	 return this->d->recipientsList;
}


/**
 * Checks if the @a recipientInfo is already known or not.
 *
 * If the recipient is already known, the supplied @a recipientInfo is deleted
 * and the pointer is moved to the already known recipient object.
 * Otherwise the new recipient is added to the known recipients and the signal
 * @ref recipientsListChanged() is emitted.
 *
 */
//public slot
void abt_settings::addKnownRecipient(abt_EmpfaengerInfo *recipientInfo)
{
	int pos = this->d->recipientsList->indexOf(recipientInfo);
	if (pos == -1) { //we must add the unknown receiver to our list
		this->d->recipientsList->append(recipientInfo);
		emit this->recipientsListChanged();
	} else { // the "unknown" receiver already, exists
		delete recipientInfo; //delete the current object
		//and let point it to the already known object
		recipientInfo = this->d->recipientsList->at(pos);
	}
}

//public slot
void abt_settings::onReplaceKnownRecipient(int position, abt_EmpfaengerInfo *newRecipient)
{
	abt_EmpfaengerInfo *oldListEntry;
	oldListEntry = this->d->recipientsList->at(position);
	this->d->recipientsList->replace(position, newRecipient);
	delete oldListEntry;
	emit this->recipientsListChanged();
}

//public slot
void abt_settings::deleteKnownRecipient(abt_EmpfaengerInfo* recipientInfo)
{
	abt_EmpfaengerInfo *oldEntry;
	int pos = this->d->recipientsList->indexOf(recipientInfo);
	oldEntry = this->d->recipientsList->takeAt(pos);
	delete oldEntry;
	emit this->recipientsListChanged();
}





void abt_settings::saveWindowStateGeometry(QByteArray state,
					   QByteArray geometry)
{
	this->d->settings->setValue("Main/WindowState", state);
	this->d->settings->setValue("Main/WindowGeometry", geometry);
}

QByteArray abt_settings::loadWindowState() const
{
	return this->d->settings->value("Main/WindowState", QVariant()).toByteArray();
}

QByteArray abt_settings::loadWindowGeometry() const
{
	return this->d->settings->value("Main/WindowGeometry", QVariant()).toByteArray();
}




void abt_settings::saveSelAccountInWidget(const QString &widgetName, const aqb_AccountInfo *acc)
{
	QString groupname("Main/Widget");
	groupname.append(widgetName);
	this->d->settings->setValue(groupname, acc->get_ID());
}

int abt_settings::loadSelAccountInWidget(const QString &widgetName) const
{
	QString groupname("Main/Widget");
	groupname.append(widgetName);
	return this->d->settings->value(groupname, -1).toInt();
}



bool abt_settings::showDialog(const QString &dialogType) const
{
	return this->d->settings->value(QString("Dialogs/Show").append(dialogType),
				     true).toBool();
}

void abt_settings::setShowDialog(const QString &dialogType, bool show)
{
	this->d->settings->setValue(QString("Dialogs/Show").append(dialogType), show);
}




bool abt_settings::appendJobToOutbox(const QString &jobname) const
{
	return this->d->settings->value(QString("LoadAtStart/").append(jobname),
				     false).toBool();
}

void abt_settings::setAppendJobToOutbox(const QString &jobname, bool get)
{
	this->d->settings->setValue(QString("LoadAtStart/").append(jobname), get);
}


//public
bool abt_settings::autoAddNewRecipients() const
{
	return this->d->settings->value("Options/autoAddNewRecipients", true).toBool();
}

//public
void abt_settings::setAutoAddNewRecipients(bool value)
{
	this->d->settings->setValue("Options/autoAddNewRecipients", value);
}

/**
 * @brief reads all entrys for Im-/Exporter favorites from the settings
 *
 * This function goes trough all groups of "ImExportFavorites" (also the
 * childgroups) and returns a QStringList which items could be passed to
 * @ref isProfileFavorit() to determine if the profile should be handled
 * as favorite or not.
 *
 */
QStringList abt_settings::getAllProfileFavorites() const
{
	this->d->settings->beginGroup("ImExportFavorites");
	QStringList retList;

	foreach(const QString group, this->d->settings->childGroups()) {
		this->d->settings->beginGroup(group);

		foreach(const QString key, this->d->settings->childKeys()) {
			retList.append(QString(group).append("/").append(key));
		}

		this->d->settings->endGroup();
	}
	this->d->settings->endGroup();

	return retList;
}

bool abt_settings::isProfileFavorit(const QString &name) const
{
	QString key = QString("ImExportFavorites/").append(name);
	return this->d->settings->value(key, false).toBool();
}

void abt_settings::setProfileFavorit(const QString &name, bool favorit)
{
	QString key = QString("ImExportFavorites/").append(name);
	this->d->settings->setValue(key, favorit);
}

/**
 * @brief removes the entry from the settings file
 * @param name: the key of the profile (form: "plugin/name")
 */
void abt_settings::deleteProfileFavorit(const QString &name)
{
	this->d->settings->beginGroup("ImExportFavorites");
	this->d->settings->remove(name);
	this->d->settings->endGroup();
}


bool abt_settings::isAdvancedOptionSet(const QString &option) const
{
	QString key = QString("Options/Advanced/").append(option);
	return this->d->settings->value(key, false).toBool();
}

void abt_settings::setAdvancedOption(const QString &option, bool value)
{
	QString key = QString("Options/Advanced/").append(option);
	this->d->settings->setValue(key, value);
}

void abt_settings::deleteAdvancedOption(const QString &option)
{
	this->d->settings->beginGroup("Options/Advanced/");
	this->d->settings->remove(option);
	this->d->settings->endGroup();
}


void abt_settings::saveColWidth(const QString &name, int col, int width)
{
	QString key = QString("Main/").append(name);
	QString key2 = QString("/col%1").arg(col);
	key.append(key2);
	this->d->settings->setValue(key, width);
}

int abt_settings::getColWidth(const QString &name, int col, int def)
{
	QString key = QString("Main/").append(name);
	QString key2 = QString("/col%1").arg(col);
	key.append(key2);
	bool convOK;
	int ret = this->d->settings->value(key, def).toInt(&convOK);
	if (convOK) {
		return ret;
	}

	return def;
}


//static
void abt_settings::resizeColToContentsFor(QTreeWidget *w)
{
	for (int i=0; i<w->columnCount(); ++i) {
		w->resizeColumnToContents(i);
	}
}


/**
 * @returns -1: error (not handled \a type passed)
 * @returns  0: not supported
 * @returns  1: supported
 * @returns  2: not yet supported (maybe in future release)
 *
 */
//static public
int abt_settings::supportedByAbtransfers(const AB_JOB_TYPE type)
{
	switch (type) {
	//supported and implemented types
	case AB_Job_TypeGetBalance :

	case AB_Job_TypeTransfer :
	case AB_Job_TypeInternalTransfer :
	case AB_Job_TypeSepaTransfer :

	case AB_Job_TypeCreateDatedTransfer :
	case AB_Job_TypeModifyDatedTransfer :
	case AB_Job_TypeDeleteDatedTransfer :
	case AB_Job_TypeGetDatedTransfers :

	case AB_Job_TypeCreateStandingOrder :
	case AB_Job_TypeModifyStandingOrder :
	case AB_Job_TypeDeleteStandingOrder :
	case AB_Job_TypeGetStandingOrders :
		return 1;
		break;

	//not supported but should be implemented
	case AB_Job_TypeEuTransfer :
	case AB_Job_TypeDebitNote :
	case AB_Job_TypeSepaDebitNote :
	case AB_Job_TypeLoadCellPhone :
		return 2;
		break;

	//not supported (and not planed to be implemented)
	case AB_Job_TypeGetTransactions :
	case AB_Job_TypeUnknown :
		return 0;
		break;
	}

	return -1; //error
}

//public
void abt_settings::setRecipientsFilename(const QString &filename)
{
	if (filename.isEmpty()) {
		qWarning() << Q_FUNC_INFO << "filename is empty, nothing to store";
		return; //Abbruch
	}

	this->d->settings->setValue("Main/RecipientsFilename", filename);

	this->d->recipientsFilename = filename;
}

//public
void abt_settings::setAccountDataFilename(const QString &filename)
{
	if (filename.isEmpty()) {
		qWarning() << Q_FUNC_INFO << "filename is empty, nothing to store";
		return; //Abbruch
	}

	this->d->settings->setValue("Main/AccountDataFilename", filename);

	this->d->accountdataFilename = filename;
}

//public
void abt_settings::setHistoryFilename(const QString &filename)
{
	if (filename.isEmpty()) {
		qWarning() << Q_FUNC_INFO << "filename is empty, nothing to store";
		return; //Abbruch
	}

	this->d->settings->setValue("Main/HistoryFilename", filename);

	this->d->historyFilename = filename;
}

//public
void abt_settings::setDataDir(const QString &dirname)
{
	if (dirname.isEmpty()) {
		qWarning() << Q_FUNC_INFO << "dirname is empty, nothing to store";
		return; //Abbruch
	}

	this->d->settings->setValue("Main/DataDir", dirname);

	this->d->dataDir = dirname;
}

