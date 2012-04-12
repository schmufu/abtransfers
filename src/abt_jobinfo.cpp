/******************************************************************************
 * Copyright (C) 2012 Patrick Wacker
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
 *	Abstraction of an AB_JOB, used at "Ausgang" and "Historie"
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#include "abt_jobinfo.h"

#include <aqbanking/jobsingletransfer.h>
#include <aqbanking/jobsingledebitnote.h>
#include <aqbanking/jobinternaltransfer.h>
#include <aqbanking/jobeutransfer.h>
#include <aqbanking/jobsepatransfer.h>
#include <aqbanking/jobsepadebitnote.h>
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>
#include <aqbanking/jobloadcellphone.h>

#include <aqbanking/jobcreatedatedtransfer.h>
#include <aqbanking/jobmodifydatedtransfer.h>
#include <aqbanking/jobdeletedatedtransfer.h>
#include <aqbanking/jobgetdatedtransfers.h>

#include <aqbanking/jobcreatesto.h>
#include <aqbanking/jobmodifysto.h>
#include <aqbanking/jobdeletesto.h>
#include <aqbanking/jobgetstandingorders.h>

#include "abt_conv.h"
#include "aqb_accountinfo.h"
#include "abt_transaction_base.h"



abt_jobInfo::abt_jobInfo(AB_JOB *j)
{
	//wir sind ein container für den job, geben diesen wieder zurück,
	//löschen ihn aber nicht!
	this->m_job = j;

	//Alle weiteren Elemente so setzen als währen wir ohne job aufgerufen
	//worden, bzw. die benötigten Parameter aus dem Job kopieren
	this->m_jobInfo = new QStringList();
	this->m_jobType = AB_Job_GetType(this->m_job);
	this->m_jobStatus = AB_Job_GetStatus(this->m_job);
	this->m_ABAccount = AB_Job_GetAccount(this->m_job);

	//je nachdem welcher Job übergeben wurde hat dieser eine andere
	//oder keine AB_Transaction. Dies durchgehen und den privaten
	//Pointer (this->m_trans) entsprechend setzen
	this->setMyTransactionFromJob();

	//create the info stringlist that is displayed at the "Ausgang"/"Historie" page
	this->createJobInfoStringList(this->m_jobInfo);
}

abt_jobInfo::abt_jobInfo(AB_JOB_TYPE type, AB_JOB_STATUS status,
			 const AB_TRANSACTION *t, const AB_ACCOUNT *acc,
			 QDateTime date)
	: m_job(NULL),
	  m_ABAccount(acc),
	  m_jobType(type),
	  m_jobStatus(status),
	  m_date(date)
{
	this->m_jobInfo = new QStringList();
	//wir speichern eine kopie der AB_TRANSACTION als neue abt_transaction
	this->m_trans = new abt_transaction(AB_Transaction_dup(t), true);

	//create the info stringlist that is displayed at the "Ausgang"/"Historie" page
	this->createJobInfoStringList(this->m_jobInfo);
}


abt_jobInfo::~abt_jobInfo()
{
	//The job is owned by aqBanking, so we dont free it!

	//we created a copy of the jobs transaction
	delete this->m_trans; //could be NULL, but it is safe to delete a NULL-Pointer

	//we created the StringList, so we delete it.
	delete this->m_jobInfo;
}

/** \brief setzt den privaten pointer \a this->trans auf die Transaction des jobs */
void abt_jobInfo::setMyTransactionFromJob()
{
	const AB_TRANSACTION *AB_Trans = NULL;

	switch (this->getAbJobType()) {
	case AB_Job_TypeCreateDatedTransfer:
		AB_Trans = AB_JobCreateDatedTransfer_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeCreateStandingOrder:
		AB_Trans = AB_JobCreateStandingOrder_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeDebitNote:
		AB_Trans = AB_JobSingleDebitNote_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeDeleteDatedTransfer:
		AB_Trans = AB_JobDeleteDatedTransfer_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeDeleteStandingOrder:
		AB_Trans = AB_JobDeleteStandingOrder_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeEuTransfer:
		AB_Trans = AB_JobEuTransfer_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeGetBalance:
		break;
	case AB_Job_TypeGetDatedTransfers:
		break;
	case AB_Job_TypeGetStandingOrders:
		break;
	case AB_Job_TypeGetTransactions:
		break;
	case AB_Job_TypeInternalTransfer:
		AB_Trans = AB_JobInternalTransfer_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeLoadCellPhone:
		break;
	case AB_Job_TypeModifyDatedTransfer:
		AB_Trans = AB_JobModifyDatedTransfer_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeModifyStandingOrder:
		AB_Trans = AB_JobModifyStandingOrder_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeSepaDebitNote:
		AB_Trans = AB_JobSepaDebitNote_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeSepaTransfer:
		AB_Trans = AB_JobSepaTransfer_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeTransfer:
		AB_Trans = AB_JobSingleTransfer_GetTransaction(this->m_job);
		break;
	case AB_Job_TypeUnknown:
		AB_Trans = NULL;
		break;
	}

	if (AB_Trans == NULL) {
		//the job does not have a transaction, so do i
		this->m_trans = NULL;
		return;
	}

	//wir kopieren die transaction des jobs
	AB_TRANSACTION *tcopy = AB_Transaction_dup(AB_Trans);
	//und erstellen uns für diese Kopie eine abt_transaction
	abt_transaction *trans = new abt_transaction(tcopy, true);
	//diese Transaction merken wir uns und arbeiten dann später mit dieser
	this->m_trans = trans;
}


AB_JOB_STATUS abt_jobInfo::getAbJobStatus() const
{
	return this->m_jobStatus;
}

const QString abt_jobInfo::getStatus() const
{
	return abt_conv::JobStatusToQString(this->m_jobStatus);
}

AB_JOB_TYPE abt_jobInfo::getAbJobType() const
{
	return this->m_jobType;
}

const QString abt_jobInfo::getType() const
{
	return abt_conv::JobTypeToQString(this->m_jobType);
}

const QStringList *abt_jobInfo::getInfo() const
{
	return this->m_jobInfo;
}

AB_JOB *abt_jobInfo::getJob() const
{
	return this->m_job;
}

const AB_ACCOUNT* abt_jobInfo::getAbAccount() const
{
	return this->m_ABAccount;
}

const QString abt_jobInfo::getKontoNr() const
{
	return QString::fromUtf8(AB_Account_GetAccountNumber(this->m_ABAccount));
}

const QString abt_jobInfo::getBLZ() const
{
	return QString::fromUtf8(AB_Account_GetBankCode(this->m_ABAccount));
}

int abt_jobInfo::getAccountID() const
{
	return AB_Account_GetUniqueId(this->m_ABAccount);
}

void abt_jobInfo::createJobInfoStringList(QStringList *strList) const
{

	switch (this->m_jobType) {
	case AB_Job_TypeCreateDatedTransfer:
		this->createJobInfoStringList_CreateDatedTransfer(strList);
		break;
	case AB_Job_TypeCreateStandingOrder:
		this->createJobInfoStringList_CreateStandingOrder(strList);
		break;
	case AB_Job_TypeDebitNote:
		this->createJobInfoStringList_DebitNote(strList);
		break;
	case AB_Job_TypeDeleteDatedTransfer:
		this->createJobInfoStringList_DeleteDatedTransfer(strList);
		break;
	case AB_Job_TypeDeleteStandingOrder:
		this->createJobInfoStringList_DeleteStandingOrder(strList);
		break;
	case AB_Job_TypeEuTransfer:
		this->createJobInfoStringList_EuTransfer(strList);
		break;
	case AB_Job_TypeGetBalance:
		this->createJobInfoStringList_GetBalance(strList);
		break;
	case AB_Job_TypeGetDatedTransfers:
		this->createJobInfoStringList_GetDatedTransfers(strList);
		break;
	case AB_Job_TypeGetStandingOrders:
		this->createJobInfoStringList_GetStandingOrders(strList);
		break;
	case AB_Job_TypeGetTransactions:
		this->createJobInfoStringList_GetTransactions(strList);
		break;
	case AB_Job_TypeInternalTransfer:
		this->createJobInfoStringList_InternalTransfer(strList);
		break;
	case AB_Job_TypeLoadCellPhone:
		this->createJobInfoStringList_LoadCellPhone(strList);
		break;
	case AB_Job_TypeModifyDatedTransfer:
		this->createJobInfoStringList_ModifyDatedTransfer(strList);
		break;
	case AB_Job_TypeModifyStandingOrder:
		this->createJobInfoStringList_ModifyStandingOrder(strList);
		break;
	case AB_Job_TypeSepaDebitNote:
		this->createJobInfoStringList_SepaDebitNote(strList);
		break;
	case AB_Job_TypeSepaTransfer:
		this->createJobInfoStringList_SepaTransfer(strList);
		break;
	case AB_Job_TypeTransfer:
		this->createJobInfoStringList_Transfer(strList);
		break;
	case AB_Job_TypeUnknown:
		this->createJobInfoStringList_Unknown(strList);
		break;
	}
}


/** \brief Fügt den Absender der Stringliste \a strList hinzu */
void abt_jobInfo::createJobInfoStringList_Append_From(QStringList *strList) const
{
	if (!this->m_trans) return; //only if transaction available
	QString info;

	info = QObject::tr("Von: %1 (%2 - %3)").arg(this->m_trans->getLocalName(),
						    this->m_trans->getLocalAccountNumber(),
						    this->m_trans->getLocalBankCode());
	strList->append(info);
}

/** \brief Fügt den Empfänger der Stringliste \a strList hinzu */
void abt_jobInfo::createJobInfoStringList_Append_To(QStringList *strList) const
{
	if (!this->m_trans) return; //only if transaction available
	QString info;

	info = QObject::tr("Zu:  %1 (%2 - %3)").arg(this->m_trans->getRemoteName().at(0),
						    this->m_trans->getRemoteAccountNumber(),
						    this->m_trans->getRemoteBankCode());
	strList->append(info);
}

/** \brief Fügt den Verwendungszweck der Stringliste \a strList hinzu */
void abt_jobInfo::createJobInfoStringList_Append_Purpose(QStringList *strList) const
{
	if (!this->m_trans) return; //only if transaction available

	strList->append(QObject::tr("Verwendungszweck:"));
	for (int i=0; i<this->m_trans->getPurpose().size(); ++i) {
		strList->append("    " + this->m_trans->getPurpose().at(i));
	}
}

/** \brief Fügt den Betrag und die Währung der Stringliste \a strList hinzu */
void abt_jobInfo::createJobInfoStringList_Append_Value(QStringList *strList) const
{
	if (!this->m_trans) return; //only if transaction available
	QString info;
	const AB_VALUE *v = this->m_trans->getValue();

	info = QObject::tr("Betrag: %1 %2").arg(abt_conv::ABValueToString(v, true),
						AB_Value_GetCurrency(v));
	strList->append(info);
}


/** \brief Erstellt den Standart Informations Text für einen Job
  *
  * Folgende Informationen werden der übergebenen StringListe \a strList angefügt:
  *
  * \pre
  *     'Von: NAME (KTONR - BLZ)'
  *	'Zu:  NAME (KTONR - BLZ)'
  *	'Verwendungszweck:'
  *	     Alle Zeilen des Verwendungszwecks
  *	'Betrag: WERT WÄHRUNG'
  *
  */
void abt_jobInfo::createJobInfoStringList_Standard_Text(QStringList *strList) const
{
	this->createJobInfoStringList_Append_From(strList);
	this->createJobInfoStringList_Append_To(strList);
	this->createJobInfoStringList_Append_Purpose(strList);
	this->createJobInfoStringList_Append_Value(strList);
}

/** \brief Erstellt den Standart Text für Daueraufträge */
void abt_jobInfo::createJobInfoStringList_ForStandingOrders(QStringList *strList) const
{
	if (!this->m_trans) return; //only if transaction available

	const abt_transaction *t = this->m_trans;

	this->createJobInfoStringList_Standard_Text(strList);
	strList->append(""); //leere Zeile

	int cycle = t->getCycle();
	AB_TRANSACTION_PERIOD period = t->getPeriod();
	int day = t->getExecutionDay();

	QString strPeriod;
	if (period == AB_Transaction_PeriodMonthly) {
		switch(cycle) {
		case 1: strPeriod = QObject::tr("Monat"); break;
		default: strPeriod = QObject::tr("Monate"); break;
		}
	} else {
		switch(cycle) {
		case 1: strPeriod = QObject::tr("Woche"); break;
		default: strPeriod = QObject::tr("Wochen"); break;
		}
	}

	QString strCycle;
	if (cycle <= 1) {
		if (period == AB_Transaction_PeriodMonthly) {
			strCycle = QObject::tr("jeden");
		} else {
			strCycle = QObject::tr("jeden");
		}
	} else {
		strCycle = QObject::tr("alle %1").arg(cycle);
	}

	strList->append(QObject::tr("Ausführung: %1 %2 am %3").arg(
			strCycle, strPeriod).arg(day));

	strList->append(QObject::tr("Erste Ausführung:   %1").arg(
			t->getFirstExecutionDate().toString()));
	strList->append(QObject::tr("Letzte Ausführung:  %1").arg(
			t->getLastExecutionDate().toString()));
	strList->append(QObject::tr("Nächste Ausführung: %1").arg(
			t->getNextExecutionDate().toString()));

}

/** \brief Erstellt den Standart Text für terminierte Überweisungen */
void abt_jobInfo::createJobInfoStringList_ForDatedTransfers(QStringList *strList) const
{
	const abt_transaction *t = this->m_trans;
	this->createJobInfoStringList_Standard_Text(strList);
	strList->append(QObject::tr("Tag der Ausführung: %1").arg(
			t->getDate().toString(Qt::DefaultLocaleLongDate)));
}


void abt_jobInfo::createJobInfoStringList_CreateDatedTransfer(QStringList *strList) const
{
	this->createJobInfoStringList_ForDatedTransfers(strList);
}

void abt_jobInfo::createJobInfoStringList_CreateStandingOrder(QStringList *strList) const
{
	this->createJobInfoStringList_ForStandingOrders(strList);
}

void abt_jobInfo::createJobInfoStringList_DebitNote(QStringList *strList) const
{
	this->createJobInfoStringList_Standard_Text(strList);
}

void abt_jobInfo::createJobInfoStringList_DeleteDatedTransfer(QStringList *strList) const
{
	this->createJobInfoStringList_ForDatedTransfers(strList);}

void abt_jobInfo::createJobInfoStringList_DeleteStandingOrder(QStringList *strList) const
{
	this->createJobInfoStringList_ForStandingOrders(strList);
}

void abt_jobInfo::createJobInfoStringList_EuTransfer(QStringList *strList) const
{
	this->createJobInfoStringList_Append_From(strList);
	strList->append(QObject::tr("Zu:  %1 (%2 - %3) [%4]").arg(
			this->m_trans->getRemoteName().at(0),
			this->m_trans->getRemoteAccountNumber(),
			this->m_trans->getRemoteBankCode(),
			this->m_trans->getRemoteBankLocation()));
	this->createJobInfoStringList_Append_Purpose(strList);
	this->createJobInfoStringList_Append_Value(strList);
}

void abt_jobInfo::createJobInfoStringList_GetBalance(QStringList *strList) const
{
	strList->append(QObject::tr("Aktualisiert den aktuellen Saldo für das"));
	strList->append(QObject::tr("Konto %1 (%2)").arg(
			AB_Account_GetAccountNumber(this->m_ABAccount),
			AB_Account_GetAccountName(this->m_ABAccount)));
}

void abt_jobInfo::createJobInfoStringList_GetDatedTransfers(QStringList *strList) const
{
	strList->append(QObject::tr("Holt alle noch nicht ausgeführten terminierten Überweisungen"));
	strList->append(QObject::tr("für das Konto %1 (%2)").arg(
			AB_Account_GetAccountNumber(this->m_ABAccount),
			AB_Account_GetAccountName(this->m_ABAccount)));
}

void abt_jobInfo::createJobInfoStringList_GetStandingOrders(QStringList *strList) const
{
	strList->append(QObject::tr("Holt alle bei der Bank hinterlegten Daueraufträge"));
	strList->append(QObject::tr("für das Konto %1 (%2)").arg(
			AB_Account_GetAccountNumber(this->m_ABAccount),
			AB_Account_GetAccountName(this->m_ABAccount)));

}

void abt_jobInfo::createJobInfoStringList_GetTransactions(QStringList *strList) const
{
	strList->append(QObject::tr("Hm, sollte nicht vorkommen! Dieser Auftrag wird"));
	strList->append(QObject::tr("momentan von AB-Transfers nicht unterstützt!"));
	strList->append(QObject::tr("Bitte Löschen Sie diesen Auftrag, da nicht sicher"));
	strList->append(QObject::tr("ist welche Fehler eventuell auftreten könnten!"));
}

void abt_jobInfo::createJobInfoStringList_InternalTransfer(QStringList *strList) const
{
	this->createJobInfoStringList_Standard_Text(strList);
}

void abt_jobInfo::createJobInfoStringList_LoadCellPhone(QStringList *strList) const
{
	strList->append(QObject::tr("Hm, sollte nicht vorkommen! Dieser Auftrag wird"));
	strList->append(QObject::tr("momentan von AB-Transfers nicht unterstützt!"));
	strList->append(QObject::tr("Bitte Löschen Sie diesen Auftrag, da nicht sicher"));
	strList->append(QObject::tr("ist welche Fehler eventuell auftreten könnten!"));
}

void abt_jobInfo::createJobInfoStringList_ModifyDatedTransfer(QStringList *strList) const
{
	this->createJobInfoStringList_ForDatedTransfers(strList);
}

void abt_jobInfo::createJobInfoStringList_ModifyStandingOrder(QStringList *strList) const
{
	this->createJobInfoStringList_ForStandingOrders(strList);
}

void abt_jobInfo::createJobInfoStringList_SepaDebitNote(QStringList *strList) const
{
	strList->append(QObject::tr("Hm, sollte nicht vorkommen! Dieser Auftrag wird"));
	strList->append(QObject::tr("momentan von AB-Transfers nicht unterstützt!"));
	strList->append(QObject::tr("Bitte Löschen Sie diesen Auftrag, da nicht sicher"));
	strList->append(QObject::tr("ist welche Fehler eventuell auftreten könnten!"));
}

void abt_jobInfo::createJobInfoStringList_SepaTransfer(QStringList *strList) const
{
	this->createJobInfoStringList_Standard_Text(strList);
}

void abt_jobInfo::createJobInfoStringList_Transfer(QStringList *strList) const
{
	this->createJobInfoStringList_Standard_Text(strList);
}

void abt_jobInfo::createJobInfoStringList_Unknown(QStringList *strList) const
{
	strList->append(QObject::tr("Hm, sollte nicht vorkommen! Dieser Auftrag wird"));
	strList->append(QObject::tr("momentan von AB-Transfers nicht unterstützt!"));
	strList->append(QObject::tr("Bitte Löschen Sie diesen Auftrag, da nicht sicher"));
	strList->append(QObject::tr("ist welche Fehler eventuell auftreten könnten!"));
}

