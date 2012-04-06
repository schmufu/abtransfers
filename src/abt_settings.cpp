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
	QString defValue;

	this->Settings = new QSettings(QDir::homePath() + "/.ab_transfers/settings.ini",
				       QSettings::IniFormat, this);
	this->Settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

	this->EmpfaengerList = new QList<abt_EmpfaengerInfo*>;


	defValue = QDir::homePath() + "/.ab_transfers/knownEmpfaenger.txt";
	defValue = QDir::toNativeSeparators(defValue);
	this->knownEmpfaengerFilename =
		this->Settings->value("Main/EmpfaengerFileName", defValue).toString();

	defValue = QDir::homePath() + "/.ab_transfers/";
	defValue = QDir::toNativeSeparators(defValue);
	this->m_dataDir = this->Settings->value("Main/DataDir", defValue).toString();

	this->m_textKeyDescr = NULL;
	this->loadTextKeyDescriptions();

	//Sicherstellen das die Dateiberechtigungen stimmen
	bool ret = QFile::setPermissions(QDir::homePath() + "/.ab_transfers/settings.ini",
					 QFile::ReadOwner | QFile::ReadUser |
					 QFile::WriteOwner | QFile::WriteUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions failed";
	ret = QFile::setPermissions(QDir::homePath() + "/.ab_transfers",
				    QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
				    QFile::ReadUser | QFile::WriteUser | QFile::ExeUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions on folder failed";
}

abt_settings::~abt_settings()
{
	//Alle Empfaenger aus der EmpängerListe speichern
	this->saveKnownEmpfaenger();
	//und dann die Objekte löschen
	while (this->EmpfaengerList->size()) {
		delete this->EmpfaengerList->takeFirst();
	}
	//sowie die Liste an sich
	delete this->EmpfaengerList;

	delete this->m_textKeyDescr;

	//Einstellungen in der ini-Datei speichern
	this->Settings->setValue("Main/EmpfaengerFileName",
				 this->knownEmpfaengerFilename);
	this->Settings->setValue("Main/DataDir", this->m_dataDir);

	//und danach das Object wieder löschen
	delete this->Settings;

	//Sicherstellen das die Dateiberechtigungen stimmen
	bool ret = QFile::setPermissions(QDir::homePath() + "/.ab_transfers/settings.ini",
					 QFile::ReadOwner | QFile::ReadUser |
					 QFile::WriteOwner | QFile::WriteUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions failed";
	ret = QFile::setPermissions(QDir::homePath() + "/.ab_transfers",
				    QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
				    QFile::ReadUser | QFile::WriteUser | QFile::ExeUser);
	if (!ret) qWarning() << Q_FUNC_INFO << " setting permissions on folder failed";

	qDebug() << Q_FUNC_INFO << "deleted";
}

void abt_settings::loadTextKeyDescriptions()
{
	if (this->m_textKeyDescr == NULL) {
		this->m_textKeyDescr = new QHash<int, QString>;
	}

	this->m_textKeyDescr->clear();

	if (!this->Settings->childGroups().contains("TextKeyDescriptions")) {
		//TextKexDescriptions noch unbekannt, default werte setzen
		this->Settings->beginGroup("TextKeyDescriptions");
		this->Settings->setValue("04", "Lastschrift (Abbuchungsauftragsverfahren)");
		this->Settings->setValue("05", "Lastschrift (Einzugsermächtigungsverfahren)");
		this->Settings->setValue("51", "Überweisung");
		this->Settings->setValue("52", "Dauerauftrags-Überweisung");
		this->Settings->setValue("53", "Lohn-, Gehalts-, Renten-Überweisung");
		this->Settings->setValue("54", "Vermögenswirksame Leistung (VL)");
		this->Settings->setValue("56", "Überweisung öffentlicher Kassen");
		this->Settings->setValue("67", "Überweisung mit prüfziffergesicherten Zuordnungsdaten (BZÜ)");
		this->Settings->setValue("69", "Spendenüberweisung");
		this->Settings->endGroup();
	}
	
	this->Settings->beginGroup("TextKeyDescriptions");
	//Alle Schlüssel durchgehen und deren Werte in einem QHash Speichern
	foreach (QString key, this->Settings->allKeys()) {
		QString text = this->Settings->value(key, tr("Unbekannt")).toString();
		this->m_textKeyDescr->insert(key.toInt(), text);
	}

	this->Settings->endGroup();
}

const QHash<int, QString> *abt_settings::getTextKeyDescriptions() const
{
	return this->m_textKeyDescr;
}

const QList<abt_EmpfaengerInfo*>* abt_settings::loadKnownEmpfaenger()
{
	this->EmpfaengerList->clear();

	QFile file(this->knownEmpfaengerFilename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return NULL;

	QTextStream in(&file);
	QStringList InfoStringList;
	abt_EmpfaengerInfo *EmpfaengerInfo;
	while (!in.atEnd()) {
		QString line = in.readLine();
		InfoStringList = line.split("\t", QString::KeepEmptyParts);

		EmpfaengerInfo = new abt_EmpfaengerInfo();
		EmpfaengerInfo->setName(InfoStringList.at(0));
		EmpfaengerInfo->setKontonummer(InfoStringList.at(1));
		EmpfaengerInfo->setBLZ(InfoStringList.at(2));
		EmpfaengerInfo->setVerw1(InfoStringList.at(3));
		EmpfaengerInfo->setVerw2(InfoStringList.at(4));
		EmpfaengerInfo->setVerw3(InfoStringList.at(5));
		EmpfaengerInfo->setVerw4(InfoStringList.at(6));

		this->EmpfaengerList->append(EmpfaengerInfo);
	}

	file.close();
	return this->EmpfaengerList;
}

void abt_settings::saveKnownEmpfaenger()
{
	abt_EmpfaengerInfo *EmpfaengerInfo;

	QFile file(this->knownEmpfaengerFilename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);

	for (int i=0; i<this->EmpfaengerList->size(); ++i) {
		EmpfaengerInfo = this->EmpfaengerList->at(i);
		out << EmpfaengerInfo->getName() << "\t";
		out << EmpfaengerInfo->getKontonummer() << "\t";
		out << EmpfaengerInfo->getBLZ() << "\t";
		out << EmpfaengerInfo->getVerw1() << "\t";
		out << EmpfaengerInfo->getVerw2() << "\t";
		out << EmpfaengerInfo->getVerw3() << "\t";
		out << EmpfaengerInfo->getVerw4() << "\n";
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
void abt_settings::addKnownEmpfaenger(abt_EmpfaengerInfo *EInfo)
{
	int pos = this->EmpfaengerList->indexOf(EInfo);
	if (pos == -1) { //we must add the unknown receiver to our list
		this->EmpfaengerList->append(EInfo);
		emit this->EmpfaengerListChanged();
	} else { // the "unknown" receiver already, exists
		delete EInfo; //delete the current object
		//and let point it to the already known object
		EInfo = this->EmpfaengerList->at(pos);
	}
}

//public slot
void abt_settings::onReplaceKnownEmpfaenger(int position, abt_EmpfaengerInfo *newE)
{
	abt_EmpfaengerInfo *oldListEntry;
	oldListEntry = this->EmpfaengerList->at(position);
	this->EmpfaengerList->replace(position, newE);
	delete oldListEntry;
	emit this->EmpfaengerListChanged();
}

//public slot
void abt_settings::deleteKnownEmpfaenger(abt_EmpfaengerInfo* EmpfaengerInfo)
{
	abt_EmpfaengerInfo *oldEntry;
	int pos = this->EmpfaengerList->indexOf(EmpfaengerInfo);
	oldEntry = this->EmpfaengerList->takeAt(pos);
	delete oldEntry;
	emit this->EmpfaengerListChanged();
}




const QString *abt_settings::getDataDir() const
{
	return &this->m_dataDir;
}












QList<abt_StandingInfo*> *abt_settings::getStandingOrdersForAccount(const aqb_AccountInfo *a)
{
	return this->getStandingOrdersForAccount(a->Number(), a->BankCode());
}

QList<abt_StandingInfo*> *abt_settings::getStandingOrdersForAccount(const QString &KtoNr,
								 const QString &BLZ)
{
	const QString SOMainKey = "StandingOrders/" + BLZ + "/" + KtoNr;
	this->Settings->beginGroup(SOMainKey);

	QStringList SO_IDs = this->Settings->childGroups();
	this->Settings->endGroup();

	//Jetzt stehen in SO_IDs alle IDs von StandingOrders die zu dem
	//angegebenen paar von KtoNr und BLZ gehören.

	qDebug() << "childGroups von " << KtoNr << " = " << SO_IDs;


	abt_StandingInfo *StandingInfo;
	QList<abt_StandingInfo*> *List = new QList<abt_StandingInfo*>;

	foreach(const QString ID, SO_IDs) {
		const QString SOKey = SOMainKey + "/" + ID;
		this->Settings->beginGroup(SOKey);
		abt_transaction *trans = abt_transaction::loadTransaction(this->Settings);
		StandingInfo = new abt_StandingInfo(trans);
		List->append(StandingInfo);
		this->Settings->endGroup();
	}

	return List;
}

void abt_settings::saveStandingOrder(const abt_transaction *t)
{
	const QString BLZ = t->getLocalBankCode();
	const QString KtoNr = t->getLocalAccountNumber();
	const QString ID = t->getFiId();
	const QString SOGroup = "StandingOrders/" + BLZ + "/" + KtoNr + "/" + ID;

	this->Settings->beginGroup(SOGroup);

	abt_transaction::saveTransaction(t, this->Settings);

	this->Settings->endGroup();
}

void abt_settings::saveStandingOrder(const AB_TRANSACTION *t)
{
	const abt_transaction *trans = new abt_transaction(t);
	this->saveStandingOrder(trans);
	delete trans;
}

void abt_settings::deleteStandingOrder(const abt_transaction *t)
{
	const QString BLZ = t->getLocalBankCode();
	const QString KtoNr = t->getLocalAccountNumber();
	const QString ID = t->getFiId();
	const QString SOGroup = "StandingOrders/" + BLZ + "/" + KtoNr + "/" + ID;

	this->Settings->beginGroup(SOGroup);
	this->Settings->remove("");	//Aktuelle Gruppe löschen
	this->Settings->endGroup();
}

void abt_settings::deleteStandingOrder(const AB_TRANSACTION *t)
{
	const abt_transaction *trans = new abt_transaction(t);
	this->deleteStandingOrder(trans);
	delete trans;
}

//static
void abt_settings::freeStandingOrdersList(QList<abt_StandingInfo*> *list)
{
	if (list == NULL) return; //Liste ist schon leer

	while (list->size() != 0) {
		delete list->takeFirst();
	}
	delete list;
	list = NULL;
}








QList<abt_DatedInfo*> *abt_settings::getDatedTransfersForAccount(const aqb_AccountInfo *a)
{
	return this->getDatedTransfersForAccount(a->Number(), a->BankCode());
}

QList<abt_DatedInfo*> *abt_settings::getDatedTransfersForAccount(const QString &KtoNr,
								 const QString &BLZ)
{
	const QString DTMainKey = "DatedTransfers/" + BLZ + "/" + KtoNr;
	this->Settings->beginGroup(DTMainKey);

	QStringList DT_IDs = this->Settings->childGroups();
	this->Settings->endGroup();

	//Jetzt stehen in DTIDs alle IDs von DatedTransfers die zu dem
	//angegebenen paar von KtoNr und BLZ gehören.

	qDebug() << "childGroups von " << KtoNr << " = " << DT_IDs;


	abt_DatedInfo *DatedInfo;
	QList<abt_DatedInfo*> *List = new QList<abt_DatedInfo*>;

	foreach(const QString ID, DT_IDs) {
		const QString DTKey = DTMainKey + "/" + ID;
		this->Settings->beginGroup(DTKey);
		abt_transaction *trans = abt_transaction::loadTransaction(this->Settings);
		DatedInfo = new abt_DatedInfo(trans);
		List->append(DatedInfo);
		this->Settings->endGroup();
	}

	return List;
}

void abt_settings::saveDatedTransfer(const abt_transaction *t)
{
	const QString BLZ = t->getLocalBankCode();
	const QString KtoNr = t->getLocalAccountNumber();
	const QString ID = t->getFiId();
	const QString DTGroup = "DatedTransfers/" + BLZ + "/" + KtoNr + "/" + ID;

	this->Settings->beginGroup(DTGroup);

	abt_transaction::saveTransaction(t, this->Settings);

	this->Settings->endGroup();
}

void abt_settings::saveDatedTransfer(const AB_TRANSACTION *t)
{
	const abt_transaction *trans = new abt_transaction(t);
	this->saveDatedTransfer(trans);
	delete trans;
}

void abt_settings::deleteDatedTransfer(const abt_transaction *t)
{
	const QString BLZ = t->getLocalBankCode();
	const QString KtoNr = t->getLocalAccountNumber();
	const QString ID = t->getFiId();
	const QString DTGroup = "DatedTransfers/" + BLZ + "/" + KtoNr + "/" + ID;

	this->Settings->beginGroup(DTGroup);
	this->Settings->remove("");	//Aktuelle Gruppe löschen
	this->Settings->endGroup();
}

void abt_settings::deleteDatedTransfer(const AB_TRANSACTION *t)
{
	const abt_transaction *trans = new abt_transaction(t);
	this->deleteDatedTransfer(trans);
	delete trans;
}

//static
void abt_settings::freeDatedTransfersList(QList<abt_DatedInfo *> *list)
{
	if (list == NULL) return; //Liste ist schon leer

	while (list->size() != 0) {
		delete list->takeFirst();
	}
	delete list;
	list = NULL;
}






void abt_settings::saveWindowStateGeometry(QByteArray state,
					   QByteArray geometry)
{
	this->Settings->setValue("Main/WindowState", state);
	this->Settings->setValue("Main/WindowGeometry", geometry);
}

QByteArray abt_settings::loadWindowState() const
{
	return this->Settings->value("Main/WindowState", QVariant()).toByteArray();
}

QByteArray abt_settings::loadWindowGeometry() const
{
	return this->Settings->value("Main/WindowGeometry", QVariant()).toByteArray();
}




void abt_settings::saveSelAccountInWidget(const QString &widgetName, const aqb_AccountInfo *acc)
{
	QString groupname("Main/Widget");
	groupname.append(widgetName);
	this->Settings->setValue(groupname, acc->get_ID());
}

int abt_settings::loadSelAccountInWidget(const QString &widgetName) const
{
	QString groupname("Main/Widget");
	groupname.append(widgetName);
	return this->Settings->value(groupname, -1).toInt();
}



bool abt_settings::showDialog(const QString &dialogType) const
{
	return this->Settings->value(QString("Dialogs/Show").append(dialogType),
				     true).toBool();
}

void abt_settings::setShowDialog(const QString &dialogType, bool show)
{
	this->Settings->setValue(QString("Dialogs/Show").append(dialogType), show);
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
	case AB_Job_TypeGetBalance :
	case AB_Job_TypeUnknown :
		return 0;
		break;
	}

	return -1; //error
}
