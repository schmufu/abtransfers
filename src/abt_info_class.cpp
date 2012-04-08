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
 *	classes that store information about dated transfers, messages,
 *	standing orders etc.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#include "abt_info_class.h"

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


abt_info_class::abt_info_class()
{
}






















/*********** abt_job_info class *******************/
abt_job_info::abt_job_info(AB_JOB *j)
	: job(j)
{
	this->jobInfo = new QStringList();

	//create the info stringlist that is displayed at the "Ausgang" page
	this->createJobInfoStringList(this->jobInfo);
}

abt_job_info::~abt_job_info()
{
	//The job is owned by aqBanking, so we dont free it!

	//we created the StringList, so we delete it.
	delete this->jobInfo;
}

AB_JOB_STATUS abt_job_info::getAbJobStatus() const
{
	return AB_Job_GetStatus(this->job);
}

const QString abt_job_info::getStatus() const
{
	return abt_conv::JobStatusToQString(this->job);
}

AB_JOB_TYPE abt_job_info::getAbJobType() const
{
	return AB_Job_GetType(this->job);
}

const QString abt_job_info::getType() const
{
	return abt_conv::JobTypeToQString(this->job);
}

const QStringList *abt_job_info::getInfo() const
{
	return this->jobInfo;
}

AB_JOB *abt_job_info::getJob() const
{
	return this->job;
}

const abt_transaction* abt_job_info::getAbtTransaction() const
{
	const AB_TRANSACTION *t = NULL;

	switch (this->getAbJobType()) {
	case AB_Job_TypeCreateDatedTransfer:
		t = AB_JobCreateDatedTransfer_GetTransaction(this->job);
		break;
	case AB_Job_TypeCreateStandingOrder:
		t = AB_JobCreateStandingOrder_GetTransaction(this->job);
		break;
	case AB_Job_TypeDebitNote:
		t = AB_JobSingleDebitNote_GetTransaction(this->job);
		break;
	case AB_Job_TypeDeleteDatedTransfer:
		t = AB_JobDeleteDatedTransfer_GetTransaction(this->job);
		break;
	case AB_Job_TypeDeleteStandingOrder:
		t = AB_JobDeleteStandingOrder_GetTransaction(this->job);
		break;
	case AB_Job_TypeEuTransfer:
		t = AB_JobEuTransfer_GetTransaction(this->job);
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
		t = AB_JobInternalTransfer_GetTransaction(this->job);
		break;
	case AB_Job_TypeLoadCellPhone:
		break;
	case AB_Job_TypeModifyDatedTransfer:
		t = AB_JobModifyDatedTransfer_GetTransaction(this->job);
		break;
	case AB_Job_TypeModifyStandingOrder:
		t = AB_JobModifyStandingOrder_GetTransaction(this->job);
		break;
	case AB_Job_TypeSepaDebitNote:
		break;
	case AB_Job_TypeSepaTransfer:
		t = AB_JobSepaTransfer_GetTransaction(this->job);
		break;
	case AB_Job_TypeTransfer:
		t = AB_JobSingleTransfer_GetTransaction(this->job);
		break;
	case AB_Job_TypeUnknown:
		t = NULL;
		break;
	}

	if (t == NULL) {
		return NULL;
	}

	//wir liefern eine const copy unserer Transaction zurück!
	const abt_transaction *trans = new abt_transaction(t);
	return trans;
}

const AB_ACCOUNT* abt_job_info::getAbAccount() const
{
	return AB_Job_GetAccount(this->job);
}

const QString abt_job_info::getKontoNr() const
{
	return QString::fromUtf8(
		AB_Account_GetAccountNumber(AB_Job_GetAccount(this->job)));
}

const QString abt_job_info::getBLZ() const
{
	return QString::fromUtf8(
		AB_Account_GetBankCode(AB_Job_GetAccount(this->job)));
}

void abt_job_info::createJobInfoStringList(QStringList *strList) const
{

	switch (this->getAbJobType()) {
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
void abt_job_info::createJobInfoStringList_Append_From(QStringList *strList) const
{
	QString info;
	const abt_transaction *t = this->getAbtTransaction();

	info = QObject::tr("Von: %1 (%2 - %3)").arg(t->getLocalName(),
						    t->getLocalAccountNumber(),
						    t->getLocalBankCode());
	strList->append(info);
}

/** \brief Fügt den Empfänger der Stringliste \a strList hinzu */
void abt_job_info::createJobInfoStringList_Append_To(QStringList *strList) const
{
	QString info;
	const abt_transaction *t = this->getAbtTransaction();

	info = QObject::tr("Zu:  %1 (%2 - %3)").arg(t->getRemoteName().at(0),
						    t->getRemoteAccountNumber(),
						    t->getRemoteBankCode());
	strList->append(info);
}

/** \brief Fügt den Verwendungszweck der Stringliste \a strList hinzu */
void abt_job_info::createJobInfoStringList_Append_Purpose(QStringList *strList) const
{
	const abt_transaction *t = this->getAbtTransaction();

	strList->append(QObject::tr("Verwendungszweck:"));
	for (int i=0; i<t->getPurpose().size(); ++i) {
		strList->append("   " + t->getPurpose().at(i));
	}
}

/** \brief Fügt den Betrag und die Währung der Stringliste \a strList hinzu */
void abt_job_info::createJobInfoStringList_Append_Value(QStringList *strList) const
{
	QString info;
	const abt_transaction *t = this->getAbtTransaction();
	const AB_VALUE *v = t->getValue();

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
  *	'Zu:  NAME (KTONR - BLT)'
  *	'Verwendungszweck:'
  *	     Alle Zeilen des Verwendungszwecks
  *	'Betrag: WERT WÄHRUNG'
  *
  */
void abt_job_info::createJobInfoStringList_Standard_Text(QStringList *strList) const
{
	this->createJobInfoStringList_Append_From(strList);
	this->createJobInfoStringList_Append_To(strList);
	this->createJobInfoStringList_Append_Purpose(strList);
	this->createJobInfoStringList_Append_Value(strList);
}

/** \brief Erstellt den standart Text für Daueraufträge */
void abt_job_info::createJobInfoStringList_ForStandingOrders(QStringList *strList) const
{
	//! \todo ist bei allen createJobInfoStringList_*StandingOrder identisch!
	const abt_transaction *t = this->getAbtTransaction();
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

void abt_job_info::createJobInfoStringList_CreateDatedTransfer(QStringList *strList) const
{
	//! \todo ist bei allen createJobInfoStringList_*DatedTransfer identisch!
	const abt_transaction *t = this->getAbtTransaction();
	this->createJobInfoStringList_Standard_Text(strList);
	strList->append(QObject::tr("Tag der Ausführung: %1").arg(
			t->getDate().toString(Qt::DefaultLocaleLongDate)));
}

void abt_job_info::createJobInfoStringList_CreateStandingOrder(QStringList *strList) const
{
	this->createJobInfoStringList_ForStandingOrders(strList);
}

void abt_job_info::createJobInfoStringList_DebitNote(QStringList *strList) const
{
	this->createJobInfoStringList_Standard_Text(strList);
}

void abt_job_info::createJobInfoStringList_DeleteDatedTransfer(QStringList *strList) const
{
	//! \todo ist bei allen createJobInfoStringList_*DatedTransfer identisch!
	const abt_transaction *t = this->getAbtTransaction();
	this->createJobInfoStringList_Standard_Text(strList);
	strList->append(QObject::tr("Tag der Ausführung: %1").arg(
			t->getDate().toString(Qt::DefaultLocaleLongDate)));

}

void abt_job_info::createJobInfoStringList_DeleteStandingOrder(QStringList *strList) const
{
	this->createJobInfoStringList_ForStandingOrders(strList);
}

void abt_job_info::createJobInfoStringList_EuTransfer(QStringList *strList) const
{
	const abt_transaction *t = this->getAbtTransaction();
	this->createJobInfoStringList_Append_From(strList);
	strList->append(QObject::tr("Zu:  %1 (%2 - %3) [%4]").arg(
			t->getRemoteName().at(0),
			t->getRemoteAccountNumber(),
			t->getRemoteBankCode(),
			t->getRemoteBankLocation()));
	this->createJobInfoStringList_Append_Purpose(strList);
	this->createJobInfoStringList_Append_Value(strList);
}

void abt_job_info::createJobInfoStringList_GetBalance(QStringList *strList) const
{
	const AB_ACCOUNT *acc = this->getAbAccount();
	strList->append(QObject::tr("Aktualisiert den aktuellen Saldo für das"));
	strList->append(QObject::tr("Konto %1 (%2)").arg(
			AB_Account_GetAccountNumber(acc),
			AB_Account_GetAccountName(acc)));
}

void abt_job_info::createJobInfoStringList_GetDatedTransfers(QStringList *strList) const
{
	const AB_ACCOUNT *acc = this->getAbAccount();
	strList->append(QObject::tr("Holt alle noch nicht ausgeführten terminierten Überweisungen"));
	strList->append(QObject::tr("für das Konto %1 (%2)").arg(
			AB_Account_GetAccountNumber(acc),
			AB_Account_GetAccountName(acc)));
}

void abt_job_info::createJobInfoStringList_GetStandingOrders(QStringList *strList) const
{
	const AB_ACCOUNT *acc = this->getAbAccount();
	strList->append(QObject::tr("Holt alle bei der Bank hinterlegten Daueraufträge"));
	strList->append(QObject::tr("für das Konto %1 (%2)").arg(
			AB_Account_GetAccountNumber(acc),
			AB_Account_GetAccountName(acc)));

}

void abt_job_info::createJobInfoStringList_GetTransactions(QStringList *strList) const
{
	strList->append(QObject::tr("Hm, sollte nicht vorkommen! Dieser Auftrag wird"));
	strList->append(QObject::tr("momentan von AB-Transfers nicht unterstützt!"));
	strList->append(QObject::tr("Bitte Löschen Sie diesen Auftrag, da nicht sicher"));
	strList->append(QObject::tr("ist welche Fehler eventuell auftreten könnten!"));
}

void abt_job_info::createJobInfoStringList_InternalTransfer(QStringList *strList) const
{
	this->createJobInfoStringList_Standard_Text(strList);
}

void abt_job_info::createJobInfoStringList_LoadCellPhone(QStringList *strList) const
{
	strList->append(QObject::tr("Hm, sollte nicht vorkommen! Dieser Auftrag wird"));
	strList->append(QObject::tr("momentan von AB-Transfers nicht unterstützt!"));
	strList->append(QObject::tr("Bitte Löschen Sie diesen Auftrag, da nicht sicher"));
	strList->append(QObject::tr("ist welche Fehler eventuell auftreten könnten!"));
}

void abt_job_info::createJobInfoStringList_ModifyDatedTransfer(QStringList *strList) const
{
	//! \todo ist bei allen createJobInfoStringList_*DatedTransfer identisch!
	const abt_transaction *t = this->getAbtTransaction();
	this->createJobInfoStringList_Standard_Text(strList);
	strList->append(QObject::tr("Tag der Ausführung: %1").arg(
			t->getDate().toString(Qt::DefaultLocaleLongDate)));

}

void abt_job_info::createJobInfoStringList_ModifyStandingOrder(QStringList *strList) const
{
	this->createJobInfoStringList_ForStandingOrders(strList);
}

void abt_job_info::createJobInfoStringList_SepaDebitNote(QStringList *strList) const
{
	strList->append(QObject::tr("Hm, sollte nicht vorkommen! Dieser Auftrag wird"));
	strList->append(QObject::tr("momentan von AB-Transfers nicht unterstützt!"));
	strList->append(QObject::tr("Bitte Löschen Sie diesen Auftrag, da nicht sicher"));
	strList->append(QObject::tr("ist welche Fehler eventuell auftreten könnten!"));
}

void abt_job_info::createJobInfoStringList_SepaTransfer(QStringList *strList) const
{
	this->createJobInfoStringList_Standard_Text(strList);
}

void abt_job_info::createJobInfoStringList_Transfer(QStringList *strList) const
{
	this->createJobInfoStringList_Standard_Text(strList);
}

void abt_job_info::createJobInfoStringList_Unknown(QStringList *strList) const
{
	strList->append(QObject::tr("Hm, sollte nicht vorkommen! Dieser Auftrag wird"));
	strList->append(QObject::tr("momentan von AB-Transfers nicht unterstützt!"));
	strList->append(QObject::tr("Bitte Löschen Sie diesen Auftrag, da nicht sicher"));
	strList->append(QObject::tr("ist welche Fehler eventuell auftreten könnten!"));
}


/*********** abt_job_ctrl class *******************/















