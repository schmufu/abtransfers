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

#include "abt_settings.h"

#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QTextCodec>
#include "globalvars.h"

abt_settings::abt_settings(QObject *parent) :
	QObject(parent)
{
	qDebug() << Q_FUNC_INFO << "created";

	//the standard folder is .abtransfers in the home directory
	//there is always the settings.ini

	QString homePath = QDir::homePath();
	QString iniFilename = homePath + "/.abtransfers/settings.ini";
	//the path must be useable on all systems
	iniFilename = QDir::toNativeSeparators(iniFilename);
	//if the folder does not exist, we create it
	QDir dataStorage(QDir::toNativeSeparators(homePath + "/.abtransfers"));
	if (!dataStorage.exists()) {
		bool ret = dataStorage.mkpath(dataStorage.absolutePath());
		if (!ret) {
			qWarning() << Q_FUNC_INFO << "could not create default"
				   << "folder:" << dataStorage.absolutePath();
			//we continue, the default values are useable
		}
	}


	this->settings = new QSettings(iniFilename, QSettings::IniFormat, this);
	this->settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

	this->m_recipientsList = new QList<abt_EmpfaengerInfo*>;

	//depending on the user-selection the data-files can be stored in other
	//places. We load this values or the default ones if they not exists.
	QString defValue;

	//Standard folder to store data
	defValue = homePath + "/.abtransfers/";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_dataDir = this->settings->value("Main/DataDir", defValue).toString();

	//file for known recipients
	defValue = homePath + "/.abtransfers/recipients.txt";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_recipientsFilename =
		this->settings->value("Main/RecipientsFilename", defValue).toString();

	//file for account data (balance, standing-/dated-transfers)
	defValue = homePath + "/.abtransfers/accountdata.ctx";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_accountdataFilename =
		this->settings->value("Main/AccountDataFilename", defValue).toString();

	//file for the history
	defValue = homePath + "/.abtransfers/history.ctx";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_historyFilename =
		this->settings->value("Main/HistoryFilename", defValue).toString();

	//filename for the automatic export
	defValue = homePath + "/.abtransfers/automatic_export.csv";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_autoExportFilename =
		this->settings->value("Main/AutoExportFilename", defValue).toString();

	this->m_textKeyDescr = nullptr;
	this->loadTextKeyDescriptions();

	//ensure that all file permissions are correct
	this->setFilePermissions();
}

abt_settings::~abt_settings()
{
	//save all known recipients
	this->saveKnownEmpfaenger();
	//and then free the object
	while (this->m_recipientsList->size()) {
		delete this->m_recipientsList->takeFirst();
	}
	//as well as the list itself
	delete this->m_recipientsList;

	delete this->m_textKeyDescr;

	//delete the QSettings object
	delete this->settings;

	//ensure that all file permissions are correct
	this->setFilePermissions();

	qDebug() << Q_FUNC_INFO << "deleted";
}

//private
void abt_settings::loadTextKeyDescriptions()
{
	if (this->m_textKeyDescr == nullptr) {
		this->m_textKeyDescr = new QHash<int, QString>;
	}

	this->m_textKeyDescr->clear();

	if (!this->settings->childGroups().contains("TextKeyDescriptions")) {
		//TextKexDescriptions doesnt exists, use default values
		this->settings->beginGroup("TextKeyDescriptions");
		this->settings->setValue("04", "Lastschrift (Abbuchungsauftragsverfahren)");
		this->settings->setValue("05", "Lastschrift (Einzugsermächtigungsverfahren)");
		this->settings->setValue("51", "Überweisung");
		this->settings->setValue("52", "Dauerauftrags-Überweisung");
		this->settings->setValue("53", "Lohn-, Gehalts-, Renten-Überweisung");
		this->settings->setValue("54", "Vermögenswirksame Leistung (VL)");
		this->settings->setValue("56", "Überweisung öffentlicher Kassen");
		this->settings->setValue("67", "Überweisung mit prüfziffergesicherten Zuordnungsdaten (BZÜ)");
		this->settings->setValue("69", "Spendenüberweisung");
		this->settings->endGroup();
	}
	
	this->settings->beginGroup("TextKeyDescriptions");
	//go through all keys and store the values in a QHash
	foreach (QString key, this->settings->allKeys()) {
		QString text = this->settings->value(key, tr("Unbekannt")).toString();
		this->m_textKeyDescr->insert(key.toInt(), text);
	}

	this->settings->endGroup();
}

//private
void abt_settings::setFilePermissions()
{
	QString homePath = QDir::homePath();
	QString iniFilename = homePath + "/.abtransfers/settings.ini";
	//the path must be useable on all systems
	iniFilename = QDir::toNativeSeparators(iniFilename);

	//ensure file permissions are correct
	bool ret = QFile::setPermissions(iniFilename,
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
	ret = QFile::setPermissions(QDir::toNativeSeparators(homePath + "/.abtransfers"),
				    QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
				    QFile::ReadUser | QFile::WriteUser | QFile::ExeUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions on folder"
			     << QDir::toNativeSeparators(homePath + "/.abtransfers") << "failed";

	ret = QFile::setPermissions(this->getDataDir(),
				    QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
				    QFile::ReadUser | QFile::WriteUser | QFile::ExeUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions on folder"
			     << this->getDataDir() << "failed";

}

const QHash<int, QString> *abt_settings::getTextKeyDescriptions() const
{
	return this->m_textKeyDescr;
}

const QList<abt_EmpfaengerInfo*>* abt_settings::loadKnownEmpfaenger()
{
	this->m_recipientsList->clear();

	QFile file(this->m_recipientsFilename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return nullptr;

	QTextStream in(&file);
	QStringList InfoStringList;
	abt_EmpfaengerInfo *recipientInfo;
	while (!in.atEnd()) {
		QString line = in.readLine();
		InfoStringList = line.split("\t", QString::KeepEmptyParts);

		recipientInfo = new abt_EmpfaengerInfo();
		recipientInfo->setName(InfoStringList.at(0));
		recipientInfo->setKontonummer(InfoStringList.at(1));
		recipientInfo->setBLZ(InfoStringList.at(2));
		recipientInfo->setIBAN(InfoStringList.at(3));
		recipientInfo->setBIC(InfoStringList.at(4));
		recipientInfo->setInstitut(InfoStringList.at(5));
		recipientInfo->setVerw(InfoStringList.at(6));

		this->m_recipientsList->append(recipientInfo);
	}

	file.close();
	return this->m_recipientsList;
}

void abt_settings::saveKnownEmpfaenger()
{
	abt_EmpfaengerInfo *recipientInfo;

	QFile file(this->m_recipientsFilename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);

	for (int i=0; i<this->m_recipientsList->size(); ++i) {
		recipientInfo = this->m_recipientsList->at(i);
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
	int pos = this->m_recipientsList->indexOf(recipientInfo);
	if (pos == -1) { //we must add the unknown receiver to our list
		this->m_recipientsList->append(recipientInfo);
		emit this->recipientsListChanged();
	} else { // the "unknown" receiver already, exists
		delete recipientInfo; //delete the current object
		//and let point it to the already known object
		recipientInfo = this->m_recipientsList->at(pos);
	}
}

//public slot
void abt_settings::onReplaceKnownRecipient(int position, abt_EmpfaengerInfo *newRecipient)
{
	abt_EmpfaengerInfo *oldListEntry;
	oldListEntry = this->m_recipientsList->at(position);
	this->m_recipientsList->replace(position, newRecipient);
	delete oldListEntry;
	emit this->recipientsListChanged();
}

//public slot
void abt_settings::deleteKnownRecipient(abt_EmpfaengerInfo* recipientInfo)
{
	abt_EmpfaengerInfo *oldEntry;
	int pos = this->m_recipientsList->indexOf(recipientInfo);
	oldEntry = this->m_recipientsList->takeAt(pos);
	delete oldEntry;
	emit this->recipientsListChanged();
}

//public slot
/** \brief stores the language wanted by the user at the settings
 *
 * Therefore the selected language can be restored at next start.
 */
void abt_settings::setLanguage(const QString &language)
{
	this->settings->setValue("Options/Language", language);
}




void abt_settings::saveWindowStateGeometry(QByteArray state,
					   QByteArray geometry)
{
	this->settings->setValue("Main/WindowState", state);
	this->settings->setValue("Main/WindowGeometry", geometry);
}

QByteArray abt_settings::loadWindowState() const
{
	return this->settings->value("Main/WindowState", QVariant()).toByteArray();
}

QByteArray abt_settings::loadWindowGeometry() const
{
	return this->settings->value("Main/WindowGeometry", QVariant()).toByteArray();
}




void abt_settings::saveSelAccountInWidget(const QString &widgetName, const aqb_AccountInfo *acc)
{
	QString groupname("Main/Widget");
	groupname.append(widgetName);
	this->settings->setValue(groupname, acc->get_ID());
}

int abt_settings::loadSelAccountInWidget(const QString &widgetName) const
{
	QString groupname("Main/Widget");
	groupname.append(widgetName);
	return this->settings->value(groupname, -1).toInt();
}



bool abt_settings::showDialog(const QString &dialogType) const
{
	return this->settings->value(QString("Dialogs/Show").append(dialogType),
				     true).toBool();
}

void abt_settings::setShowDialog(const QString &dialogType, bool show)
{
	this->settings->setValue(QString("Dialogs/Show").append(dialogType), show);
}




bool abt_settings::appendJobToOutbox(const QString &jobname) const
{
	return this->settings->value(QString("LoadAtStart/").append(jobname),
				     false).toBool();
}

void abt_settings::setAppendJobToOutbox(const QString &jobname, bool get)
{
	this->settings->setValue(QString("LoadAtStart/").append(jobname), get);
}


//public
bool abt_settings::autoAddNewRecipients() const
{
	return this->settings->value("Options/autoAddNewRecipients", true).toBool();
}

//public
void abt_settings::setAutoAddNewRecipients(bool value)
{
	this->settings->setValue("Options/autoAddNewRecipients", value);
}

//public
bool abt_settings::autoExportEnabled() const
{
	return this->settings->value("Options/autoExportEnabled", false).toBool();
}

//public
void abt_settings::setAutoExportEnabled(bool value)
{
	this->settings->setValue("Options/autoExportEnabled", value);
}

//public
const QString abt_settings::autoExportProfileName() const
{
	return this->settings->value("Options/autoExportProfileName", "csv").toString();
}

//public
void abt_settings::setAutoExportProfileName(const QString name) const
{
	this->settings->setValue("Options/autoExportProfileName", name);
}

//public
const QString abt_settings::autoExportPluginName() const
{
	return this->settings->value("Options/autoExportPluginName", "default").toString();
}

//public
void abt_settings::setAutoExportPluginName(const QString name) const
{
	this->settings->setValue("Options/autoExportPluginName", name);
}

//public
bool abt_settings::autoExportAsTransaction() const
{
	return this->settings->value("Main/AutoExportAsTransaction", true).toBool();
}

//public
void abt_settings::setAutoExportAsTransaction(bool value)
{
	this->settings->setValue("Main/AutoExportAsTransaction", value);
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
	this->settings->beginGroup("ImExportFavorites");
	QStringList retList;

	foreach(const QString group, this->settings->childGroups()) {
		this->settings->beginGroup(group);

		foreach(const QString key, this->settings->childKeys()) {
			retList.append(QString(group).append("/").append(key));
		}

		this->settings->endGroup();
	}
	this->settings->endGroup();

	return retList;
}

bool abt_settings::isProfileFavorit(const QString &name) const
{
	QString key = QString("ImExportFavorites/").append(name);
	return this->settings->value(key, false).toBool();
}

void abt_settings::setProfileFavorit(const QString &name, bool favorit)
{
	QString key = QString("ImExportFavorites/").append(name);
	this->settings->setValue(key, favorit);
}

/**
 * @brief removes the entry from the settings file
 * @param name: the key of the profile (form: "plugin/name")
 */
void abt_settings::deleteProfileFavorit(const QString &name)
{
	this->settings->beginGroup("ImExportFavorites");
	this->settings->remove(name);
	this->settings->endGroup();
}

//public
/** \brief sets the advanced options enabled flag.
 *
 * This function must be used to alter the enable flag for the advanced options!
 *
 * The setAdvancedOption() function would not save anything when the
 * advanced options are disabled!
 */
void abt_settings::setAdvancedOptionEnabled(bool enable)
{
	this->settings->setValue("Options/Advanced/enabled", enable);
}

//public
/** \brief checks if the advanced options are enabled or not
 *
 * \returns true if enabled
 * \returns false if disabled
 */
bool abt_settings::isAdvancedEnabled() const
{
	QString key = "Options/Advanced/enabled";
	return this->settings->value(key, false).toBool();
}

//public
/** \brief checks if the advanced options are enabled and returns the value
 *        of the wanted \a option.
 *
 * If the advanced options are not enabled, false is returned always!
 * Otherwise the value of the supplied \a option.
 */
bool abt_settings::isAdvancedOptionSet(const QString &option) const
{
	bool ret = false;

	if (this->isAdvancedEnabled()) {
		QString key = QString("Options/Advanced/").append(option);
		ret = this->settings->value(key, false).toBool();
	}
	return ret;
}

//public
/** \brief stores the \a value for the \a \option
 *
 * The value is only stored when the advanced options are enabled!
 */
void abt_settings::setAdvancedOption(const QString &option, bool value)
{
	if (this->isAdvancedEnabled()) {
		QString key = QString("Options/Advanced/").append(option);
		this->settings->setValue(key, value);
	}
}

//public
/** \overload */
void abt_settings::setAdvancedOption(const QString &option, QString value)
{
	if (this->isAdvancedEnabled()) {
		QString key = QString("Options/Advanced/").append(option);
		this->settings->setValue(key, value);
	}
}

//public
/** \brief gets the value for the \a option from the advanced options
 *
 * If the advanced options are not enabled or if the \a option is not stored
 * at the settings file, the \a defValue is returned.
 *
 * Otherwise the read string from the settings file is returned.
 */
QString abt_settings::getAdvancedOption(const QString &option, const QString defValue) const
{
	if (this->isAdvancedEnabled()) {
		QString key = QString("Options/Advanced/").append(option);
		return this->settings->value(key, defValue).toString();
	}
	return defValue;
}

//public
/** \brief deletes an advanced option from the settings file */
void abt_settings::deleteAdvancedOption(const QString &option)
{
	this->settings->beginGroup("Options/Advanced/");
	this->settings->remove(option);
	this->settings->endGroup();
}

//public
/** \brief returns the regular expression for the purpose field.
 *
 * Additional checks are performed to be sure the returned value could be
 * used as a regular expression.
 *
 * If the stored regex is empty or an invalid regex, the default value is
 * returned.
 */
QString abt_settings::allowedCharsPurposeRegex() const
{
	QString regex = this->getAdvancedOption("RegexPurpose",
						DEFAULT_REGEX_PURPOSE);

	if (regex.isEmpty() || !QRegExp(regex).isValid())
		regex = DEFAULT_REGEX_PURPOSE;

	return regex;
}

//public
/** \brief returns the regular expression for the recipient name / bankname field.
 *
 * Additional checks are performed to be sure the returned value could be
 * used as a regular expression.
 *
 * If the stored regex is empty or an invalid regex, the default value is
 * returned.
 */
QString abt_settings::allowedCharsRecipientRegex() const
{
	QString regex = this->getAdvancedOption("RegexRecipient",
						DEFAULT_REGEX_RECIPIENT);

	if (regex.isEmpty() || !QRegExp(regex).isValid())
		regex = DEFAULT_REGEX_RECIPIENT;

	return regex;
}

//public
/** \brief returns the language set by the user (or an empty string if nothing set)
 */
QString abt_settings::language() const
{
	return this->settings->value("Options/Language", QString("")).toString();
}


void abt_settings::saveColWidth(const QString &name, int col, int width)
{
	QString key = QString("Main/").append(name);
	QString key2 = QString("/col%1").arg(col);
	key.append(key2);
	this->settings->setValue(key, width);
}

int abt_settings::getColWidth(const QString &name, int col, int def)
{
	QString key = QString("Main/").append(name);
	QString key2 = QString("/col%1").arg(col);
	key.append(key2);
	bool convOK;
	int ret = this->settings->value(key, def).toInt(&convOK);
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
	int ret = -1; //default error

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
		ret = 1; //supported
		break;

		//not supported but should be implemented
	case AB_Job_TypeEuTransfer :
	case AB_Job_TypeDebitNote :
	case AB_Job_TypeSepaDebitNote :
	case AB_Job_TypeLoadCellPhone :
#if AQBANKING_VERSION_MAJOR >= 5 && AQBANKING_VERSION_MINOR >= 5
	case AB_Job_TypeSepaFlashDebitNote :
	case AB_Job_TypeSepaCreateStandingOrder :
	case AB_Job_TypeSepaDeleteStandingOrder :
	case AB_Job_TypeSepaModifyStandingOrder :
	case AB_Job_TypeSepaGetStandingOrders :
#endif
		ret = 2; //not supported yet
		break;

		//not supported (and not planed to be implemented)
	case AB_Job_TypeGetTransactions :
	case AB_Job_TypeUnknown :
		ret = 0; //not supported yet
		break;
	default:
		ret = -1; //error (not handled type)
	}

	return ret;
}

//public
void abt_settings::setRecipientsFilename(const QString &filename)
{
	if (filename.isEmpty()) {
		qWarning() << Q_FUNC_INFO << "filename is empty, nothing to store";
		return; //Abbruch
	}

	this->settings->setValue("Main/RecipientsFilename", filename);

	this->m_recipientsFilename = filename;
}

//public
void abt_settings::setAccountDataFilename(const QString &filename)
{
	if (filename.isEmpty()) {
		qWarning() << Q_FUNC_INFO << "filename is empty, nothing to store";
		return; //Abbruch
	}

	this->settings->setValue("Main/AccountDataFilename", filename);

	this->m_accountdataFilename = filename;
}

//public
void abt_settings::setHistoryFilename(const QString &filename)
{
	if (filename.isEmpty()) {
		qWarning() << Q_FUNC_INFO << "filename is empty, nothing to store";
		return; //Abbruch
	}

	this->settings->setValue("Main/HistoryFilename", filename);

	this->m_historyFilename = filename;
}

//public
void abt_settings::setDataDir(const QString &dirname)
{
	if (dirname.isEmpty()) {
		qWarning() << Q_FUNC_INFO << "dirname is empty, nothing to store";
		return; //Abbruch
	}

	this->settings->setValue("Main/DataDir", dirname);

	this->m_dataDir = dirname;
}

//public
void abt_settings::setAutoExportFilename(const QString &filename)
{
	if (filename.isEmpty()) {
		qWarning() << Q_FUNC_INFO << "filename is empty, nothing to store";
		return; // cancel
	}

	this->settings->setValue("Main/AutoExportFilename", filename);

	this->m_autoExportFilename = filename;
}

