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
#include "globalvars.h"

abt_settings::abt_settings(QObject *parent) :
	QObject(parent)
{
	this->Settings = new QSettings(QDir::homePath() + "/.ab_transfers/settings.ini",
				       QSettings::IniFormat, this);

	this->EmpfaengerList = new QList<abt_EmpfaengerInfo*>;

	this->knownEmpfaengerFilename =
		this->Settings->value("Main/EmpfaengerFileName", QDir::homePath() +
				      "/.ab_transfers/knownEmpfaenger.txt").toString();

	this->m_dataDir = this->Settings->value("Main/DataDir", QDir::homePath() +
						"/.ab_transfers/").toString();

}

abt_settings::~abt_settings()
{
	//Alle Empfaenger aus der EmpängerListe speichern
	this->saveKnownEmpfaenger(this->EmpfaengerList);
	//und dann die Objekte löschen
	while (this->EmpfaengerList->size()) {
		delete this->EmpfaengerList->takeFirst();
	}
	//sowie die Liste an sich
	delete this->EmpfaengerList;

	//Einstellungen in der ini-Datei speichern
	this->Settings->setValue("Main/EmpfaengerFileName",
				 this->knownEmpfaengerFilename);
	this->Settings->setValue("Main/DataDir",
				 this->m_dataDir);
	//und danach das Object wieder löschen
	delete this->Settings;
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

void abt_settings::saveKnownEmpfaenger(const QList<abt_EmpfaengerInfo *> *list)
{
	abt_EmpfaengerInfo *EmpfaengerInfo;

	QFile file(this->knownEmpfaengerFilename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);

	for (int i=0; i<list->size(); ++i) {
		EmpfaengerInfo = list->at(i);
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


const QString *abt_settings::getDataDir() const
{
	return &this->m_dataDir;
}










QList<abt_DAInfo*> *abt_settings::getDAsForAccount(const QString &KtoNr,
						   const QString &BLZ)
{
	QStringList DA_IDs;
	this->Settings->beginGroup("Dauerauftraege");
	QString key = KtoNr + "_" + BLZ;
	DA_IDs = this->Settings->value(key).toStringList();
	this->Settings->endGroup();

	abt_DAInfo *DAInfo;
	QList<abt_DAInfo*> *List = new QList<abt_DAInfo*>;

	for (int i=0; i<DA_IDs.size(); ++i) {
		AB_TRANSACTION *t = abt_transaction::loadTransaction(DA_IDs.at(i));
		abt_transaction *trans = new abt_transaction(t, true);
		DAInfo = new abt_DAInfo(trans);
		List->append(DAInfo);
	}

	return List;
}

void abt_settings::saveDAsForAccount(const QStringList &DAIDs,
				     const QString &KtoNr, const QString &BLZ)
{
	this->Settings->beginGroup("Dauerauftraege");
	QString key = KtoNr + "_" + BLZ;
	this->Settings->setValue(key, DAIDs);
	this->Settings->endGroup();
}

//static
void abt_settings::freeDAsList(QList<abt_DAInfo*> *list)
{
	while (list->size()) {
		delete list->takeFirst();
	}
	delete list;
}

//static
void abt_settings::resizeColToContentsFor(QTreeWidget *w)
{
	for (int i=0; i<w->columnCount(); ++i) {
		w->resizeColumnToContents(i);
	}
}
