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

abt_settings::abt_settings(QObject *parent) :
	QObject(parent)
{
	qDebug() << Q_FUNC_INFO << "created";

	//Der Standard-Speicherordner ist .abtransfers im /home/USER Verzeichnis
	//dort liegt auch IMMER die settings.ini

	QString homePath = QDir::homePath();
	QString iniFilename = homePath + "/.abtransfers/settings.ini";
	//damit der Pfad auf allen Systemen genutzt werden kann.
	iniFilename = QDir::toNativeSeparators(iniFilename);
	//wenn der Ordner noch nicht existiert erstellen wir ihn
	QDir dataStorage(QDir::toNativeSeparators(homePath + "/.abtransfers"));
	if (!dataStorage.exists()) {
		bool ret = dataStorage.mkpath(dataStorage.absolutePath());
		if (!ret) {
			qWarning() << Q_FUNC_INFO << "could not create default"
				   << "folder:" << dataStorage.absolutePath();
			//wir machen trotzdem weiter, die Default-Werte sind
			//trotzdem nutzbar.
		}
	}


	this->settings = new QSettings(iniFilename, QSettings::IniFormat, this);
	this->settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

	this->m_recipientsList = new QList<abt_EmpfaengerInfo*>;

	//Die anderen Daten können je nach Einstellung durch den Benutzer
	//auch woanders gespeichert sein. Wir laden diese Daten, bzw. default
	//Werte wenn sie nicht existieren.
	QString defValue;

	//Standart-Speicherordner
	defValue = homePath + "/.abtransfers/";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_dataDir = this->settings->value("Main/DataDir", defValue).toString();

	//Datei für Bekannte Empfänger
	defValue = homePath + "/.abtransfers/recipients.txt";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_recipientsFilename =
		this->settings->value("Main/RecipientsFilename", defValue).toString();

	//Datei für die Account-Daten (Balance, Daueraufträge, Terminüberweisungen)
	defValue = homePath + "/.abtransfers/accountdata.ctx";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_accountdataFilename =
		this->settings->value("Main/AccountDataFilename", defValue).toString();

	//Datei für die History
	defValue = homePath + "/.abtransfers/history.ctx";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_historyFilename =
		this->settings->value("Main/HistoryFilename", defValue).toString();


	this->m_textKeyDescr = NULL;
	this->loadTextKeyDescriptions();

	//Sicherstellen das alle Dateiberechtigungen stimmen
	this->setFilePermissions();
}

abt_settings::~abt_settings()
{
	//Alle Empfaenger aus der EmpängerListe speichern
	this->saveKnownEmpfaenger();
	//und dann die Objekte löschen
	while (this->m_recipientsList->size()) {
		delete this->m_recipientsList->takeFirst();
	}
	//sowie die Liste an sich
	delete this->m_recipientsList;

	delete this->m_textKeyDescr;

	//das Settings Object wieder löschen
	delete this->settings;

	//Sicherstellen das alle Dateiberechtigungen stimmen
	this->setFilePermissions();

	qDebug() << Q_FUNC_INFO << "deleted";
}

//private
void abt_settings::loadTextKeyDescriptions()
{
	if (this->m_textKeyDescr == NULL) {
		this->m_textKeyDescr = new QHash<int, QString>;
	}

	this->m_textKeyDescr->clear();

	if (!this->settings->childGroups().contains("TextKeyDescriptions")) {
		//TextKexDescriptions noch unbekannt, default werte setzen
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
	//Alle Schlüssel durchgehen und deren Werte in einem QHash Speichern
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
	//damit der Pfad auf allen Systemen genutzt werden kann.
	iniFilename = QDir::toNativeSeparators(iniFilename);

	//Sicherstellen das die Dateiberechtigungen stimmen
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

	//Auch die Berechtigungen für die Ornder sollten stimmen
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
		return NULL;

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
		recipientInfo->setVerw1(InfoStringList.at(3));
		recipientInfo->setVerw2(InfoStringList.at(4));
		recipientInfo->setVerw3(InfoStringList.at(5));
		recipientInfo->setVerw4(InfoStringList.at(6));

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
		out << recipientInfo->getVerw1() << "\t";
		out << recipientInfo->getVerw2() << "\t";
		out << recipientInfo->getVerw3() << "\t";
		out << recipientInfo->getVerw4() << "\n";
	}

	file.close();
}

/**
  Es wird überprüft ob der Empfänger bereits bekannt ist und wenn er nicht
  bekannt ist wird er in der EmpfängerListe hinzugefügt.

  Wenn der Empfänger bereits bekannt ist wird die Adresse des breits bekannten
  EmpfaengerInfo-Objects in \a EmpfaengerInfo gespeichert und das übergebene
  Object gelöscht.
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
	return this->settings->value("General/autoAddNewRecipients", true).toBool();
}

//public
void abt_settings::setAutoAddNewRecipients(bool value)
{
	this->settings->setValue("General/autoAddNewRecipients", value);
}



//static
void abt_settings::resizeColToContentsFor(QTreeWidget *w)
{
	for (int i=0; i<w->columnCount(); ++i) {
		w->resizeColumnToContents(i);
	}
}


/**
 * @return -1: error (not handled \a type passed)
 *	   0: not supported
 *	   1: supported
 *	   2: not yet supported (maybe in future release)
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
	case AB_Job_TypeSepaTransfer :
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

