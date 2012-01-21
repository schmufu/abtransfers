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

#include "abt_job_ctrl.h"

#include <QDebug>

#include <aqbanking/job.h>
#include <aqbanking/transaction.h>

#include <aqbanking/jobsingletransfer.h>
#include <aqbanking/jobsingledebitnote.h>
#include <aqbanking/jobinternaltransfer.h>
#include <aqbanking/jobeutransfer.h>
#include <aqbanking/jobsepatransfer.h>

#include <aqbanking/jobcreatedatedtransfer.h>
#include <aqbanking/jobmodifydatedtransfer.h>
#include <aqbanking/jobdeletedatedtransfer.h>
#include <aqbanking/jobgetdatedtransfers.h>

#include <aqbanking/jobcreatesto.h>
#include <aqbanking/jobmodifysto.h>
#include <aqbanking/jobdeletesto.h>
#include <aqbanking/jobgetstandingorders.h>

#include "globalvars.h"
#include "abt_conv.h"



/*********** abt_job_info class *******************/
abt_job_info::abt_job_info(AB_JOB *j, const QString &Info)
{
	this->job = j;

	//info wird als Semikolon-Getrennte Liste übergeben.
	this->jobInfo = new QStringList(Info.split(";", QString::SkipEmptyParts));
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

const QString abt_job_info::getKontoNr() const
{
	return QString(AB_Account_GetAccountNumber(AB_Job_GetAccount(this->job)));
}
const QString abt_job_info::getBLZ() const
{
	return QString(AB_Account_GetBankCode(AB_Job_GetAccount(this->job)));
}


//static
QString abt_job_info::createJobInfoString(const abt_transaction *t)
{
	QString info;
	info.append("Von: ");
	info.append(t->getLocalName());
	info.append(" (" + t->getLocalAccountNumber());
	info.append(" - " + t->getLocalBankCode() + ")");
	info.append(";");
	info.append("Zu: ");
	info.append(t->getRemoteName().at(0));
	info.append(" (" + t->getRemoteAccountNumber());
	info.append(" - " + t->getRemoteBankCode() + ")");
	info.append(";");
	info.append("Verwendungszweck:;");
	for (int i=0; i<t->getPurpose().size(); ++i) {
		info.append("   " + t->getPurpose().at(i) + ";");
	}
	info.append("Betrag: ");
	const AB_VALUE *v = t->getValue();
	info.append(abt_conv::ABValueToString(v, true));
	info.append(QString(" %1").arg(AB_Value_GetCurrency(v)));
	return info;
}

/*********** abt_job_ctrl class *******************/

abt_job_ctrl::abt_job_ctrl(QObject *parent) :
    QObject(parent)
{
	this->jobqueue = new QList<abt_job_info*>;

	emit this->log("Job-Control created: " + QDate::currentDate().toString(Qt::SystemLocaleLongDate));
}

abt_job_ctrl::~abt_job_ctrl()
{
	//Free all queued jobs
	// AB_Banking_free() aus aqb_banking löscht alle jobs!
	// deswegen laufen wir hier in einen segFault!
	//! \todo hier fehlt ein ->getJob()! evt. damit nochmal testen
//	while (!this->jobqueue->isEmpty()) {
//		AB_Job_free(this->jobqueue->takeFirst());
//	}

	while (!this->jobqueue->isEmpty()) {
		abt_job_info *j = this->jobqueue->takeFirst();
		//The job is owned by aqBanking, so we dont free it!
		//AB_Job_free(j->getJob());
		delete j;
	}

	delete this->jobqueue;

	qDebug() << this << "deleted";
}

/**
 * Erstellt alle TransactionLimits die für einen Account verfügbar sind und
 * speichert diese in dem übergebenen QHash \a ah
 */
//static public
void abt_job_ctrl::createTransactionLimitsFor(AB_ACCOUNT *a,
					      QHash<AB_JOB_TYPE, abt_transactionLimits*> *ah)
{
	//ah = AccountHash
	Q_ASSERT(ah != NULL);
	Q_ASSERT(a != NULL);

	AB_TRANSACTION *t = AB_Transaction_new();
	const AB_TRANSACTION_LIMITS *tl = NULL;
	AB_JOB *j = NULL;


	j = AB_JobSingleTransfer_new(a);
	if (AB_Job_CheckAvailability(j)) {
		qDebug("Job SingleTransfer not available");
	} else {
		AB_JobSingleTransfer_SetTransaction(j, t);
		tl = AB_JobSingleTransfer_GetFieldLimits(j);
		if (tl) {
			abt_transactionLimits *limits = new abt_transactionLimits(tl);
			ah->insert(AB_Job_TypeTransfer, limits);
		} else {
			qDebug("tl not set!");
		}
	}
	AB_Job_free(j);


	j = AB_JobSingleDebitNote_new(a);
	if (AB_Job_CheckAvailability(j)) {
		qDebug("Job SingleDebitNote not available");
	} else {
		AB_JobSingleDebitNote_SetTransaction(j, t);
		tl = AB_JobSingleDebitNote_GetFieldLimits(j);
		if (tl) {
			abt_transactionLimits *limits = new abt_transactionLimits(tl);
			ah->insert(AB_Job_TypeDebitNote, limits);
		} else {
			qDebug("tl not set!");
		}
	}
	AB_Job_free(j);

	//AB_JobEuTransfer_GetFieldLimits existiert nicht!


	j = AB_JobInternalTransfer_new(a);
	if (AB_Job_CheckAvailability(j)) {
		qDebug("Job InternalTransfer not available");
	} else {
		AB_JobInternalTransfer_SetTransaction(j, t);
		tl = AB_JobInternalTransfer_GetFieldLimits(j);
		if (tl) {
			abt_transactionLimits *limits = new abt_transactionLimits(tl);
			ah->insert(AB_Job_TypeInternalTransfer, limits);
		} else {
			qDebug("tl not set!");
		}
	}
	AB_Job_free(j);


	j = AB_JobSepaTransfer_new(a);
	if (AB_Job_CheckAvailability(j)) {
		qDebug("Job SepaTransfer not available");
	} else {
		AB_JobSepaTransfer_SetTransaction(j, t);
		tl = AB_JobSepaTransfer_GetFieldLimits(j);
		if (tl) {
			abt_transactionLimits *limits = new abt_transactionLimits(tl);
			ah->insert(AB_Job_TypeSepaTransfer, limits);
		} else {
			qDebug("tl not set!");
		}
	}
	AB_Job_free(j);


	j = AB_JobCreateDatedTransfer_new(a);
	if (AB_Job_CheckAvailability(j)) {
		qDebug("Job CreateDatedTransfer not available");
	} else {
		AB_JobCreateDatedTransfer_SetTransaction(j, t);
		tl = AB_JobCreateDatedTransfer_GetFieldLimits(j);
		if (tl) {
			abt_transactionLimits *limits = new abt_transactionLimits(tl);
			ah->insert(AB_Job_TypeCreateDatedTransfer, limits);
		} else {
			qDebug("tl not set!");
		}
	}
	AB_Job_free(j);


	j = AB_JobModifyDatedTransfer_new(a);
	if (AB_Job_CheckAvailability(j)) {
		qDebug("Job ModifyDatedTransfer not available");
	} else {
		AB_JobModifyDatedTransfer_SetTransaction(j, t);
		tl = AB_JobModifyDatedTransfer_GetFieldLimits(j);
		if (tl) {
			abt_transactionLimits *limits = new abt_transactionLimits(tl);
			ah->insert(AB_Job_TypeModifyDatedTransfer, limits);
		} else {
			qDebug("tl not set!");
		}
	}
	AB_Job_free(j);


	j = AB_JobCreateStandingOrder_new(a);
	if (AB_Job_CheckAvailability(j)) {
		qDebug("Job CreateStandingOrder not available");
	} else {
		AB_JobCreateStandingOrder_SetTransaction(j, t);
		tl = AB_JobCreateStandingOrder_GetFieldLimits(j);
		if (tl) {
			abt_transactionLimits *limits = new abt_transactionLimits(tl);
			ah->insert(AB_Job_TypeCreateStandingOrder, limits);
		} else {
			qDebug("tl not set!");
		}
	}
	AB_Job_free(j);


	j = AB_JobModifyStandingOrder_new(a);
	if (AB_Job_CheckAvailability(j)) {
		qDebug("Job ModifyStandingOrder not available");
	} else {
		AB_JobModifyStandingOrder_SetTransaction(j, t);
		tl = AB_JobModifyStandingOrder_GetFieldLimits(j);
		if (tl) {
			abt_transactionLimits *limits = new abt_transactionLimits(tl);
			ah->insert(AB_Job_TypeModifyStandingOrder, limits);
		} else {
			qDebug("tl not set!");
		}
	}
	AB_Job_free(j);

	AB_Transaction_free(t);
}



//private
void abt_job_ctrl::addlog(const QString &str)
{
	static QString time;
	time = QTime::currentTime().toString(Qt::DefaultLocaleLongDate);
	time.append(": ");
	time.append(str);

	emit this->log(time);
}

//private
QStringList abt_job_ctrl::getParsedJobLogs(const AB_JOB *j) const
{
	QStringList strList;
	GWEN_STRINGLIST *gwenStrList;

	gwenStrList = AB_Job_GetLogs(j);
	strList = abt_conv::GwenStringListToQStringList(gwenStrList);
	GWEN_StringList_free(gwenStrList); // \done macht jetzt abt_conv selbst oder?
				    //NEIN! QStringlistToGwenStringList löscht sich selbst!
				    //GwenToQ macht dies nicht! (und das aus guten Grund!)

	//die logs von aqBanking ein wenig aufbereiten (UTF8 in ASCII)
	// %22 durch " ersetzen
	strList.replaceInStrings("%22", "\"", Qt::CaseSensitive);
	// %28 durch ( ersetzen
	strList.replaceInStrings("%28", "(", Qt::CaseSensitive);
	// %29 durch ) ersetzen
	strList.replaceInStrings("%29", ")", Qt::CaseSensitive);
	// %3A durch : ersetzen
	strList.replaceInStrings("%3A", ":", Qt::CaseSensitive);
	// %C3%A4 durch ä ersetzen
	strList.replaceInStrings("%C3%A4", "ä", Qt::CaseSensitive);
	// %C3%84 durch Ä ersetzen
	strList.replaceInStrings("%C3%84", "Ä", Qt::CaseSensitive);
	// %C3%BC durch ü ersetzen
	strList.replaceInStrings("%C3%BC", "ü", Qt::CaseSensitive);
	// %C3%9C durch Ü ersetzen
	strList.replaceInStrings("%C3%9C", "Ü", Qt::CaseSensitive);
	// %C3%B6 durch ö ersetzen
	strList.replaceInStrings("%C3%B6", "ö", Qt::CaseSensitive);
	// %C3%96 durch Ö ersetzen
	strList.replaceInStrings("%C3%96", "Ö", Qt::CaseSensitive);
	// %3D durch = ersetzen
	strList.replaceInStrings("%3D", "=", Qt::CaseSensitive);

	//Aufbereitete Logs zurückgeben
	return strList;
}

//SLOT
void abt_job_ctrl::addNewSingleTransfer(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobSingleTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobSingleTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info for SingleTransfer
	QString info = abt_job_info::createJobInfoString(t);

	abt_job_info *ji = new abt_job_info(job, info);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//SLOT
void abt_job_ctrl::addNewSingleDebitNote(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobSingleDebitNote_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeDebitNote);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobSingleDebitNote_SetTransaction(job, t->getAB_Transaction());

	//Create Info for SingleDebitNote
	QString info = abt_job_info::createJobInfoString(t);

	abt_job_info *ji = new abt_job_info(job, info);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//SLOT
void abt_job_ctrl::addNewEuTransfer(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobEuTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeEuTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobEuTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info for EuTransfer
	QString info;
	info.append("Von:;");
	info.append(t->getLocalName());
	info.append(" (" + t->getLocalAccountNumber());
	info.append(" - " + t->getLocalBankCode() + ")");
	info.append(";");
	info.append("Zu:;");
	info.append(t->getRemoteName().at(0));
	info.append(" (" + t->getRemoteAccountNumber());
	info.append(" - " + t->getRemoteBankCode() + " [");
	info.append(t->getRemoteBankLocation() + "])");
	info.append(";");	
	info.append("Verwendungszweck:;");
	for (int i=0; i<t->getPurpose().size(); ++i) {
		info.append(t->getPurpose().at(i) + ";");
	}
	info.append("Betrag: ");
	const AB_VALUE *v = t->getValue();
	info.append(QString("%L1").arg(AB_Value_GetValueAsDouble(v),0,'f',2));
	info.append(QString(" %1").arg(AB_Value_GetCurrency(v)));

	abt_job_info *ji = new abt_job_info(job, info);


	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//SLOT
void abt_job_ctrl::addNewInternalTransfer(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobInternalTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeInternalTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobInternalTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info for Internal Transfer
	QString info = abt_job_info::createJobInfoString(t);

	abt_job_info *ji = new abt_job_info(job, info);


	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//SLOT
void abt_job_ctrl::addNewSepaTransfer(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobSepaTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeSepaTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobSepaTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info for SingleTransfer
	QString info = abt_job_info::createJobInfoString(t);

	abt_job_info *ji = new abt_job_info(job, info);


	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

/******* Dated Transfers ********/

//SLOT
void abt_job_ctrl::addCreateDatedTransfer(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobCreateDatedTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeCreateDatedTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobCreateDatedTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info for NewDatedTransfer
	QString info;
	info.append("Von:;");
	info.append(t->getLocalName());
	info.append(" (" + t->getLocalAccountNumber());
	info.append(" - " + t->getLocalBankCode() + ")");
	info.append(";");
	info.append("Zu:;");
	info.append(t->getRemoteName().at(0));
	info.append(" (" + t->getRemoteAccountNumber());
	info.append(" - " + t->getRemoteBankCode() + ")");
	info.append(";");
	info.append("Verwendungszweck:;");
	for (int i=0; i<t->getPurpose().size(); ++i) {
		info.append(t->getPurpose().at(i) + ";");
	}
	info.append("Tag der Ausführung:;");
	info.append(t->getDate().toString(Qt::DefaultLocaleLongDate) + ";");
	info.append("Betrag: ");
	const AB_VALUE *v = t->getValue();
	info.append(QString("%L1").arg(AB_Value_GetValueAsDouble(v),0,'f',2));
	info.append(QString(" %1").arg(AB_Value_GetCurrency(v)));

	abt_job_info *ji = new abt_job_info(job, info);


	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();

	if (!this->isJobTypeInQueue(AB_Job_TypeGetDatedTransfers, ji)) {
		//nach dem erstellen muss eine Aktualisierung stattfinden.
		this->addGetDatedTransfers(acc, true);
	}


}

//SLOT
void abt_job_ctrl::addModifyDatedTransfer(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobModifyDatedTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeModifyDatedTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobModifyDatedTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	QString info;
	info.append("Von:;");
	info.append(t->getLocalName());
	info.append(" (" + t->getLocalAccountNumber());
	info.append(" - " + t->getLocalBankCode() + ")");
	info.append(";");
	info.append("Zu:;");
	info.append(t->getRemoteName().at(0));
	info.append(" (" + t->getRemoteAccountNumber());
	info.append(" - " + t->getRemoteBankCode() + ")");
	info.append(";");
	info.append("Verwendungszweck:;");
	for (int i=0; i<t->getPurpose().size(); ++i) {
		info.append(t->getPurpose().at(i) + ";");
	}
	info.append("Tag der Ausführung:;");
	info.append(t->getDate().toString(Qt::DefaultLocaleLongDate) + ";");
	info.append("Betrag: ");
	const AB_VALUE *v = t->getValue();
	info.append(QString("%L1").arg(AB_Value_GetValueAsDouble(v),0,'f',2));
	info.append(QString(" %1").arg(AB_Value_GetCurrency(v)));

	abt_job_info *ji = new abt_job_info(job, info);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();

	if (!this->isJobTypeInQueue(AB_Job_TypeGetDatedTransfers, ji)) {
		//nach dem ändern muss eine Aktualisierung stattfinden.
		this->addGetDatedTransfers(acc, true);
	}

}

//SLOT
void abt_job_ctrl::addDeleteDatedTransfer(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobDeleteDatedTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeDeleteDatedTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobDeleteDatedTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	QString info;
	info.append("Von:;");
	info.append(t->getLocalName());
	info.append(" (" + t->getLocalAccountNumber());
	info.append(" - " + t->getLocalBankCode() + ")");
	info.append(";");
	info.append("Zu:;");
	info.append(t->getRemoteName().at(0));
	info.append(" (" + t->getRemoteAccountNumber());
	info.append(" - " + t->getRemoteBankCode() + ")");
	info.append(";");
	info.append("Verwendungszweck:;");
	for (int i=0; i<t->getPurpose().size(); ++i) {
		info.append(t->getPurpose().at(i) + ";");
	}
	info.append("Tag der Ausführung:;");
	info.append(t->getDate().toString(Qt::DefaultLocaleLongDate) + ";");
	info.append("Betrag: ");
	const AB_VALUE *v = t->getValue();
	info.append(QString("%L1").arg(AB_Value_GetValueAsDouble(v),0,'f',2));
	info.append(QString(" %1").arg(AB_Value_GetCurrency(v)));

	abt_job_info *ji = new abt_job_info(job, info);


	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//SLOT
void abt_job_ctrl::addGetDatedTransfers(const aqb_AccountInfo *acc, bool withoutInfo /*=false*/)
{
	int rv;

	AB_JOB *job = AB_JobGetDatedTransfers_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeGetDatedTransfers);
		return; //Abbruch
	}

	//Create Info
	QString info;
	info.append("Holt alle noch nicht ausgeführten Terminüberweisungen;");
	info.append("für das Konto ");
	info.append(acc->Number());
	info.append(" (" + acc->Name() + ")");

	abt_job_info *ji = new abt_job_info(job, info);


	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	if (!withoutInfo) { //Info nur verschicken wenn gewollt
		emit this->jobAdded(ji);
	}
	emit this->jobQueueListChanged();

	//Das zuständige AccountInfo Object darüber informieren wenn wir
	//mit dem parsen fertig sind (damit dies die DTs neu laden kann)
	connect(this, SIGNAL(datedTransfersParsed()),
		acc, SLOT(loadKnownDatedTransfers()));
}


/******* Standing Orders ********/


//SLOT
void abt_job_ctrl::addCreateStandingOrder(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobCreateStandingOrder_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeCreateStandingOrder);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobCreateStandingOrder_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	QString info = abt_job_info::createJobInfoString(t);

	abt_job_info *ji = new abt_job_info(job, info);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//SLOT
void abt_job_ctrl::addModifyStandingOrder(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobModifyStandingOrder_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeModifyStandingOrder);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobModifyStandingOrder_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	QString info = abt_job_info::createJobInfoString(t);

	abt_job_info *ji = new abt_job_info(job, info);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//SLOT
void abt_job_ctrl::addDeleteStandingOrder(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobDeleteStandingOrder_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeDeleteStandingOrder);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobDeleteStandingOrder_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	QString info = abt_job_info::createJobInfoString(t);

	abt_job_info *ji = new abt_job_info(job, info);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//SLOT
void abt_job_ctrl::addGetStandingOrders(const aqb_AccountInfo *acc)
{
	int rv;

	AB_JOB *job = AB_JobGetStandingOrders_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << this << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeGetStandingOrders);
		return; //Abbruch
	}

	//Create Info
	QString info;
	info.append("Holt alle bei der Bank hinterlegten Daueraufträge;");
	info.append("für das Konto ");
	info.append(acc->Number());
	info.append(" (" + acc->Name() + ")");

	abt_job_info *ji = new abt_job_info(job, info);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();

	//Das zuständige AccountInfo Object darüber informieren wenn wir
	//mit dem parsen fertig sind (damit dies die SOs neu laden kann)
	connect(this, SIGNAL(standingOrdersParsed()),
		acc, SLOT(loadKnownStandingOrders()));

}



/******** Ausführung *******/


//SLOT
void abt_job_ctrl::execQueuedTransactions()
{
	int rv;
	AB_JOB_LIST2 *jl;
	AB_IMEXPORTER_CONTEXT *ctx;

	this->addlog("Erstelle Job-Liste.");

	jl = AB_Job_List2_new();

	//Alle jobs in der reihenfolge wie der jobqueue einreihen
	for (int i=0; i<this->jobqueue->size(); ++i) {
		AB_Job_List2_PushBack(jl, this->jobqueue->at(i)->getJob());
	}

	this->addlog(QString::fromUtf8("%1 Aufträge in die Jobliste übernommen").arg(this->jobqueue->count()));

	this->checkJobStatus(jl);

	ctx = AB_ImExporterContext_new();

	this->addlog("Führe Job-Liste aus.");

	rv = AB_Banking_ExecuteJobs(banking->getAqBanking(), jl, ctx);
	if (rv) {
		qWarning() << this << "Error on execQueuedTransactions ("
				<< rv << ")";
		//cleanup
		this->addlog(QString("** ERROR ** Fehler bei AB_Banking_ExecuteJobs(). return value = %1 ** ABBRUCH **").arg(rv));
		AB_Job_List2_ClearAll(jl);
		AB_ImExporterContext_Clear(ctx);
		AB_ImExporterContext_free(ctx);
		return;
	}

	if (!this->checkJobStatus(jl)) {
		this->addlog("***********************************************");
		this->addlog("** A C H T U N G                             **");
		this->addlog("** Fehler bei der Ausführung der Jobs!       **");
		this->addlog("** Momentan werden noch nicht alle Fehler    **");
		this->addlog("** erkannt und abgefangen, Überprüfung der   **");
		this->addlog("** Aufträge von Hand erforderlich!           **");
		this->addlog("***********************************************");
	}

	this->parseExecutedJobListAndContext(jl, ctx);
	this->parseImExporterContext(ctx);

	this->addlog("Alle Jobs übertragen und Antworten ausgewertet");

	AB_Job_List2_ClearAll(jl);
	AB_ImExporterContext_Clear(ctx);
	AB_ImExporterContext_free(ctx);

	//Alle Objecte in der jobqueue liste löschen
	while (!this->jobqueue->isEmpty()) {
		abt_job_info *j = this->jobqueue->takeFirst();
		//AB_Job_free(j->getJob());
		delete j;
	}
	emit this->jobQueueListChanged();
}


bool abt_job_ctrl::parseExecutedJobListAndContext(AB_JOB_LIST2 *jobList, AB_IMEXPORTER_CONTEXT *ctx)
{
	AB_JOB *j;
	AB_JOB_STATUS jobState;
	AB_JOB_TYPE jobType;
	QString strState;
	QString strType;
	QStringList strList;
	int run=0;
	bool res=true;
	qDebug() << Q_FUNC_INFO << "started";

	//Die Jobs wurden zur Bank übertragen und evt. durch das Backend geändert
	//jetzt alle Jobs durchgehen und entsprechend des Status parsen
	AB_JOB_LIST2_ITERATOR *jli;
	jli = AB_Job_List2Iterator_new(jobList);
	jli = AB_Job_List2_First(jobList);
	j = AB_Job_List2Iterator_Data(jli);
	while (j) {
		qDebug() << Q_FUNC_INFO << "- while - RUN: " << ++run;
		jobType = AB_Job_GetType(j);
		strType = AB_Job_Type2Char(jobType);
		this->addlog(QString("JobType: ").append(strType));

		jobState = AB_Job_GetStatus(j);
		strState = AB_Job_Status2Char(jobState);
		this->addlog(QString("JobState: ").append(strState));

		//Die Logs des Backends parsen
		strList = this->getParsedJobLogs(j);

		//Alle Strings der StringListe zum Log hinzufügen
		foreach(QString line, strList) { // (int i=0; i<strList.count(); ++i) {
			this->addlog(QString("JobLog: ").append(line));
		}


		switch (jobType) {
		case AB_Job_TypeCreateDatedTransfer:
			//wenn Status Successfull Transaction des Jobs als Dated Speichern
			this->parseJobTypeCreateDatedTransfer(j);
			break;
		case AB_Job_TypeDeleteDatedTransfer:
			//wenn Successfull Ctx parsen (dort ist der gelöschte DT drin)
			this->parseJobTypeDeleteDatedTransfer(j);
			break;
		case AB_Job_TypeModifyDatedTransfer:
			//wenn Successfull Transaction des Jobs löschen
			//wurde die FiId der Transaction im Job geändert? wenn ja, wie kommen wir an die alte?
			//die neue Transaction muss gespeichert werden
			this->parseJobTypeModifyDatedTransfer(j);
			break;
		case AB_Job_TypeGetDatedTransfers:
			//wenn Successfull Ctx parsen (dort sind die DTs drin)
			this->parseJobTypeGetDatedTransfers(j);
			break;

		//für StandingOrders siehe Kommentare bei DatedTransfers
		case AB_Job_TypeCreateStandingOrder:
			this->parseJobTypeCreateStandingOrder(j);
			break;
		case AB_Job_TypeDeleteStandingOrder:
			this->parseJobTypeDeleteStandingOrder(j);
			break;
		case AB_Job_TypeModifyStandingOrder:
			this->parseJobTypeModifyStandingOrder(j);
			break;
		case AB_Job_TypeGetStandingOrders:
			this->parseJobTypeGetStandingOrders(j);
			break;

		//hier einfach den Ctx parsen? Prüfung auf Erfolg sollte auch gemacht werden!
		//was machen wir bei Fehler? Job sollte in der jobQueue bleiben!
		case AB_Job_TypeTransfer:
			this->parseJobTypeTransfer(j);

		case AB_Job_TypeEuTransfer:
			this->parseJobTypeEuTransfer(j);
			break;
		case AB_Job_TypeSepaTransfer:
			this->parseJobTypeSepaTransfer(j);
			break;
		case AB_Job_TypeInternalTransfer:
			this->parseJobTypeInternalTransfer(j);
			break;
		case AB_Job_TypeDebitNote:
			this->parseJobTypeDebitNote(j);
			break;
		case AB_Job_TypeSepaDebitNote:
			this->parseJobTypeSepaDebitNote(j);
			break;
		case AB_Job_TypeUnknown:
			this->parseJobTypeUnknown(j);
			break;

		case AB_Job_TypeGetBalance:
			this->parseJobTypeGetBalance(j);
			break;
		case AB_Job_TypeGetTransactions:
			this->parseJobTypeGetTransactions(j);
			break;

		case AB_Job_TypeLoadCellPhone:
			this->parseJobTypeLoadCellPhone(j);
			break;

		default:
			break;
		} /* switch (type) */


		j = AB_Job_List2Iterator_Next(jli); //next Job in list
	} /* while (j) */

	AB_Job_List2Iterator_free(jli); //Joblist iterator wieder freigeben

	qDebug() << Q_FUNC_INFO << "finished";

	return res;
}

bool abt_job_ctrl::parseImExporterContext(AB_IMEXPORTER_CONTEXT *ctx)
{
	AB_IMEXPORTER_ACCOUNTINFO *ai;

	QString log = AB_ImExporterContext_GetLog(ctx);
	this->addlog(QString("CTX-LOG: ").append(log));

	this->parseImExporterContext_Messages(ctx);
	this->parseImExporterContext_Securitys(ctx);

	ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
	while(ai) {
		//Beim Anlegen einer Terminüberweisung wird hierher nicht verzweigt!
		this->parseImExporterAccountInfo_Status(ai);
		this->parseImExporterAccountInfo_DatedTransfers(ai);	//Terminüberweisungen
		this->parseImExporterAccountInfo_NotedTransactions(ai);	//geplante Buchungen
		this->parseImExporterAccountInfo_StandingOrders(ai);	//Daueraufträge
		this->parseImExporterAccountInfo_Transactions(ai);	//Buchungen
		this->parseImExporterAccountInfo_Transfers(ai);		//Überweisungen

		ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
	} /* while ai */

	return true;
}


int abt_job_ctrl::parseImExporterContext_Messages(AB_IMEXPORTER_CONTEXT *ctx)
{
	AB_MESSAGE *msg;
	QString logmsg = "Recvd-Message: ";
	QString logmsg2;
	int msgcnt = 0;

	msg = AB_ImExporterContext_GetFirstMessage(ctx);
	while (msg) {
		logmsg2 = QString("Empfangsdatum:\t");
		logmsg2.append(abt_conv::GwenTimeToQDate(
				AB_Message_GetDateReceived(msg)).toString(
						Qt::DefaultLocaleLongDate));
		this->addlog(logmsg + logmsg2);
		logmsg2 = QString("Betreff:\t");
		logmsg2.append(AB_Message_GetSubject(msg));
		this->addlog(logmsg + logmsg2);
		logmsg2 = QString("Text:\t");
		logmsg2.append(AB_Message_GetText(msg));
		this->addlog(logmsg + logmsg2);
		msg = AB_ImExporterContext_GetNextMessage(ctx);
		msgcnt++;
	}

	logmsg2 = QString("Count: %1").arg(msgcnt);
	this->addlog(logmsg + logmsg2);

	return msgcnt;
}

int abt_job_ctrl::parseImExporterContext_Securitys(AB_IMEXPORTER_CONTEXT *ctx)
{
	AB_SECURITY *s;
	QString logmsg = "Recvd-Security: ";
	QString logmsg2;
	const AB_VALUE *v;
	int seccnt = 0;

	s = AB_ImExporterContext_GetFirstSecurity(ctx);
	while (s) {
		logmsg2 = QString("Name:\t");
		logmsg2.append(AB_Security_GetName(s));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("UnitPriceValue:\t");
		v = AB_Security_GetUnitPriceValue(s);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		s = AB_ImExporterContext_GetNextSecurity(ctx);
		seccnt++;
	}

	logmsg2 = QString("Count: %1").arg(seccnt);
	this->addlog(logmsg + logmsg2);

	return seccnt;
}

int abt_job_ctrl::parseImExporterAccountInfo_Status(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
	AB_ACCOUNT_STATUS *s;
	QString logmsg = "Recvd-AccountStatus: ";
	QString logmsg2;
	const AB_VALUE *v;
	const AB_BALANCE *b;
	int cnt = 0;

	s = AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
	while (s) {
		logmsg2 = QString("BankLine:\t");

		v = AB_AccountStatus_GetBankLine(s);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("NotedBalance:\t");
		b = AB_AccountStatus_GetNotedBalance(s);
		v = AB_Balance_GetValue(b);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("BookedBalance:\t");
		b = AB_AccountStatus_GetBookedBalance(s);
		v = AB_Balance_GetValue(b);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Disposable:\t");
		v = AB_AccountStatus_GetDisposable(s);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Disposed:\t");
		v = AB_AccountStatus_GetDisposed(s);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);


		logmsg2 = QString("Time:\t");
		logmsg2.append(abt_conv::GwenTimeToQDate(
						AB_AccountStatus_GetTime(s)).toString(
								Qt::DefaultLocaleLongDate));
		this->addlog(logmsg + logmsg2);

		s = AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
		cnt++;
	}

	logmsg2 = QString("Count: %1").arg(cnt);
	this->addlog(logmsg + logmsg2);

	return cnt;

}

int abt_job_ctrl::parseImExporterAccountInfo_DatedTransfers(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
	AB_TRANSACTION *t;
	QString logmsg = "Recvd-DatedTransfers: ";
	QString logmsg2;
	QStringList strList, DTIDsNew, DTIDsModified, DTIDsDeleted;
	const AB_VALUE *v;
	const GWEN_STRINGLIST *l;
	int cnt = 0;
	bool datedChanged = false;

	cnt = AB_ImExporterAccountInfo_GetDatedTransferCount(ai);
	logmsg2 = QString("Count: %1").arg(cnt);
	this->addlog(logmsg + logmsg2);

	t = AB_ImExporterAccountInfo_GetFirstDatedTransfer(ai);
	while (t) {

		logmsg2 = QString("Purpose:\t");
		l = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		switch (AB_Transaction_GetStatus(t)) {
		case AB_Transaction_StatusRevoked:
			//Bei der Bank hinterlegte Terminüberweisung wurde gelöscht
			this->addlog(QString(
				"Lösche bei der Bank gelöschte Terminüberweisung (ID: %1)"
				).arg(AB_Transaction_GetFiId(t)));
			//löscht die Transaction in der settings.ini
			settings->deleteDatedTransfer(t);
			datedChanged = true;
			break;
		case AB_Transaction_StatusManuallyReconciled:
		case AB_Transaction_StatusAutoReconciled:
			//Bei der Bank hinterlegte Terminüberweisung wurde geändert
			this->addlog(QString(
				"Speichere bei der Bank geänderte Terminüberweisung (ID: %1)"
				).arg(AB_Transaction_GetFiId(t)));
			//einfach löschen und dann neu speichern
			//die neue Transaction hat eine neue ID bekommen, wir
			//müssen die alte löschen und dann die neue Speichern!

			//Hier muss die alte gelöscht werden! wo bekommen wir die her?
			//-->die alte wird durch parseJobTypeModifyDatedTransfer() gelöscht
			//settings->deleteDatedTransfer(t);
			//neue Transaction speichern
			settings->saveDatedTransfer(t);
			datedChanged = true;
			break;
		default:
			//Bei der Bank hinterlegte Terminüberweisung auch lokal speichern
			this->addlog(QString(
				"Speichere bei der Bank hinterlegte Terminüberweisung (ID: %1)"
				).arg(AB_Transaction_GetFiId(t)));
			settings->saveDatedTransfer(t);
			datedChanged = true;
			break;
		}

		t = AB_ImExporterAccountInfo_GetNextDatedTransfer(ai);
	}

	if (datedChanged) {
		emit this->datedTransfersParsed();
	}

//	QString KtoNr = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountNumber(ai));
//	QString BLZ = QString::fromUtf8(AB_ImExporterAccountInfo_GetBankCode(ai));
//
//	if (DTIDsNew.size() > 0) {
//		//Die lokal gespeicherten DatedTransfers auch in den Einstellungen merken.
//		settings->saveDatedTransfersForAccount(DTIDsNew, KtoNr, BLZ);
//
//		emit this->datedTransfersParsed();
//	}

	return cnt;
}

int abt_job_ctrl::parseImExporterAccountInfo_NotedTransactions(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
	AB_TRANSACTION *t;
	QString logmsg = "Recvd-NotedTransactions: ";
	QString logmsg2;
	QStringList strList;
	const AB_VALUE *v;
	const GWEN_STRINGLIST *l;
	int cnt = 0;

	cnt = AB_ImExporterAccountInfo_GetNotedTransactionCount(ai);
	logmsg2 = QString("Count: %1").arg(cnt);
	this->addlog(logmsg + logmsg2);

	t = AB_ImExporterAccountInfo_GetFirstNotedTransaction(ai);
	while (t) {
		logmsg2 = QString("Purpose:\t");
		l = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetNextNotedTransaction(ai);
	}
	return cnt;

}

int abt_job_ctrl::parseImExporterAccountInfo_StandingOrders(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
	AB_TRANSACTION *t;
	QString logmsg = "Recvd-StandingOrders: ";
	QString logmsg2;
	QStringList strList, DAIDs;
	const AB_VALUE *v;
	const GWEN_STRINGLIST *l;
	int cnt = 0;

	cnt = AB_ImExporterAccountInfo_GetStandingOrderCount(ai);
	logmsg2 = QString("Count: %1").arg(cnt);
	this->addlog(logmsg + logmsg2);
	DAIDs.clear();

	t = AB_ImExporterAccountInfo_GetFirstStandingOrder(ai);
	while (t) {
		logmsg2 = QString("Purpose:\t");
		l = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		//Bei der Bank hinterlegten Dauerauftrag auch lokal speichern
		this->addlog(QString(
			"Speichere bei der Bank hinterlegten Dauerauftrag (ID: %1)"
			).arg(AB_Transaction_GetFiId(t)));
		settings->saveStandingOrder(t);
		t = AB_ImExporterAccountInfo_GetNextStandingOrder(ai);
	}

	/* DONE   Das Speichern wird auch ausgeführt wenn keine DAs empfangen
		  wurden. Somit werden alle vorhandenen DAs gelöscht!
		  Es muss nur gespeichert werden wenn auch DAs empfangen wurden!
	*/
//	if (DAIDs.size() > 0) {
//		//Die lokal gespeicherten DAs auch in den Einstellungen merken.
//		QString KtoNr = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountNumber(ai));
//		QString BLZ = QString::fromUtf8(AB_ImExporterAccountInfo_GetBankCode(ai));
//
//		settings->saveStandingOrdersForAccount(DAIDs, KtoNr, BLZ);
//
//
//	}

	emit this->standingOrdersParsed();
	return cnt;

}

int abt_job_ctrl::parseImExporterAccountInfo_Transfers(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
	AB_TRANSACTION *t;
	QString logmsg = "Recvd-Transfers: ";
	QString logmsg2;
	QStringList strList;
	const AB_VALUE *v;
	const GWEN_STRINGLIST *l;
	int cnt = 0;

	cnt = AB_ImExporterAccountInfo_GetTransferCount(ai);
	logmsg2 = QString("Count: %1").arg(cnt);
	this->addlog(logmsg + logmsg2);

	t = AB_ImExporterAccountInfo_GetFirstTransfer(ai);
	while (t) {
		logmsg2 = QString("Purpose:\t");
		l = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetNextTransfer(ai);
	}
	return cnt;

}

int abt_job_ctrl::parseImExporterAccountInfo_Transactions(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
	AB_TRANSACTION *t;
	QString logmsg = "Recvd-Transactions: ";
	QString logmsg2;
	QStringList strList;
	const AB_VALUE *v;
	const GWEN_STRINGLIST *l;
	int cnt = 0;

	cnt = AB_ImExporterAccountInfo_GetTransactionCount(ai);
	logmsg2 = QString("Count: %1").arg(cnt);
	this->addlog(logmsg + logmsg2);

	t = AB_ImExporterAccountInfo_GetFirstTransaction(ai);
	while (t) {
		logmsg2 = QString("Purpose:\t");
		l = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetNextTransaction(ai);
	}
	return cnt;

}

bool abt_job_ctrl::checkJobStatus(AB_JOB_LIST2 *jl)
{
	AB_JOB *j;
	AB_JOB_STATUS state;
	AB_JOB_TYPE type;

	QString strState;
	QString strType;
	QStringList strList;
	bool res=true;
	static int run = 0;
	run++;
	qDebug() << Q_FUNC_INFO << " - RUN: " << run;

	AB_JOB_LIST2_ITERATOR *jli;
	jli = AB_Job_List2Iterator_new(jl);
	jli = AB_Job_List2_First(jl);
	j = AB_Job_List2Iterator_Data(jli);
	while (j) {
		qDebug() << Q_FUNC_INFO << " - while - RUN: " << run;
		type = AB_Job_GetType(j);
		strType = AB_Job_Type2Char(type);
		this->addlog(QString("JobType: ").append(strType));
		state = AB_Job_GetStatus(j);
		if (state == AB_Job_StatusPending) {
			res = false;
		}
		strState = AB_Job_Status2Char(state);
		this->addlog(QString("JobState: ").append(strState));


		strList = this->getParsedJobLogs(j);

		//Alle Strings der StringListe zum Log hinzufügen
		for (int i=0; i<strList.count(); ++i) {
			this->addlog(QString("JobLog: ").append(strList.at(i)));
		}

		j = AB_Job_List2Iterator_Next(jli);
	}

	AB_Job_List2Iterator_free(jli);

	qDebug() << Q_FUNC_INFO << " - end - RUN: " << run;

	return res;
}

/** prüft ob ein job vom typ \a type in der queuelist vorhanden ist und ob
  dieser Job auch für dasselbe Konto wie der Job \a ji ist.

  Kann genutzt werden um zu überprüfen ob bereits ein Aktualisierungs-Job
  in der queuelist vorhanden ist oder nicht. */
bool abt_job_ctrl::isJobTypeInQueue(const AB_JOB_TYPE type, const abt_job_info *ji) const
{
	//Kontrollieren ob ein AktualisierungsAuftrag bereits vorhanden ist

	for(int i=0; i<this->jobqueue->size(); i++) {
		//jiiq = JobInfoInQueue
		const abt_job_info *jiiq = this->jobqueue->at(i);

		if (jiiq->getAbJobType() == type) {
			//job ist vorhanden, ist er auch für dasselbe Konto?
			if ((jiiq->getKontoNr() == ji->getKontoNr()) &&
			    (jiiq->getBLZ() == ji->getBLZ())) {
				return true;
			}
		}
	}

	return false;
}

/** verschiebt den Job von \a jobListPos um \a updown nach oben oder unten */
//public slot
void abt_job_ctrl::moveJob(int JobListPos, int updown)
{
	if (JobListPos >= this->jobqueue->size()) {
		qWarning().nospace() << "abt_job_ctrl::moveJob - JobListPos ["
				<< JobListPos << "] is greater than the jobqueue->size() ["
				<< this->jobqueue->size() << "]";
		return; //Abbruch
	}

	int newPos = JobListPos + updown;
	if ((newPos < 0) || (newPos >= this->jobqueue->size())) {
		qWarning().nospace() << "abt_job_ctrl::moveJob - new position ["
				<< newPos << "] is not reachable! size() ["
				<< this->jobqueue->size() << "]";
		return; //Abbruch
	}

	this->jobqueue->move(JobListPos, newPos);

	//Alle die es wollen darüber Informieren das sich die Liste geändert hat
	emit this->jobQueueListChanged();
}

/** löscht den Job von \a jobListPos */
//public slot
void abt_job_ctrl::deleteJob(int JobListPos)
{
	if ((JobListPos >= this->jobqueue->size()) ||
	    (JobListPos < 0)) {
		qWarning().nospace() << "abt_job_ctrl::deleteJob - JobListPos ["
				<< JobListPos << "] is greater than the jobqueue->size() ["
				<< this->jobqueue->size() << "] (or less than zero)";
	}

	abt_job_info *jobinfo;
	jobinfo = this->jobqueue->takeAt(JobListPos); //aus der Liste enfernen
	AB_Job_free(jobinfo->getJob()); //aq_banking Job löschen
	delete jobinfo; // und jobinfo löschen

	//Alle die es wollen darüber Informieren das sich die Liste geändert hat
	emit this->jobQueueListChanged();
}










/******************************************************************************
 * Parsen der einzelnen Jobs nach Ausführung
 ******************************************************************************/

//wenn Status Successfull Transaction des Jobs als Dated Speichern
int abt_job_ctrl::parseJobTypeCreateDatedTransfer(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobCreateDatedTransfer_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - saving DatedTransfer" << AB_Transaction_GetFiId(t);

		//Die hier vorhandene Transaktion hat keine FiId!!!!!!
		//settings->saveDatedTransfer(t);

		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;

}

//wenn Successfull Ctx parsen (dort ist der gelöschte DT drin)
int abt_job_ctrl::parseJobTypeDeleteDatedTransfer(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobDeleteDatedTransfer_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - deleting DatedTransfer" << AB_Transaction_GetFiId(t);
		settings->deleteDatedTransfer(t);
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

//wenn Successfull Transaction des Jobs löschen
//wurde die FiId der Transaction im Job geändert? wenn ja, wie kommen wir an die alte?
//die neue Transaction muss gespeichert werden
int abt_job_ctrl::parseJobTypeModifyDatedTransfer(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobModifyDatedTransfer_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - deleting DatedTransfer" << AB_Transaction_GetFiId(t);
		//Wir löschen einfach den übertragenen Transfer, der neue steckt
		//im ctx und wird später durch parseCtx gespeichert.
		settings->deleteDatedTransfer(t);
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

//wenn Successfull Ctx parsen (dort sind die DTs drin)
int abt_job_ctrl::parseJobTypeGetDatedTransfers(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	//t = AB_JobGetDatedTransfers_GetDatedTransfers(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}


//für StandingOrders siehe Kommentare bei DatedTransfers
int abt_job_ctrl::parseJobTypeCreateStandingOrder(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobCreateStandingOrder_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeDeleteStandingOrder(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobDeleteStandingOrder_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeModifyStandingOrder(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobModifyStandingOrder_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeGetStandingOrders(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	//t = AB_JobGetStandingOrders_GetStandingOrders(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}


//hier einfach den Ctx parsen? Prüfung auf Erfolg sollte auch gemacht werden!
//was machen wir bei Fehler? Job sollte in der jobQueue bleiben!
int abt_job_ctrl::parseJobTypeTransfer(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobSingleTransfer_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeEuTransfer(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobEuTransfer_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeSepaTransfer(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobSepaTransfer_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeInternalTransfer(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobInternalTransfer_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeDebitNote(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	t = AB_JobSingleDebitNote_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeSepaDebitNote(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	//t = AB_Job(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeUnknown(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	//t = AB_JobCreateDatedTransfer_GetTransaction(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}


int abt_job_ctrl::parseJobTypeGetBalance(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	//t = AB_Job(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}

int abt_job_ctrl::parseJobTypeGetTransactions(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	//t = AB_Job(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}


int abt_job_ctrl::parseJobTypeLoadCellPhone(const AB_JOB *j) {
	AB_JOB_STATUS jobStatus;
	const AB_TRANSACTION *t;

	jobStatus = AB_Job_GetStatus(j);
	//t = AB_Job(j);

	switch(jobStatus) {
	case AB_Job_StatusNew:
		//Job is new and not yet enqueued.
		qDebug() << Q_FUNC_INFO << " - Status: New" << " - Nothing done yet";
		break;
	case AB_Job_StatusUpdated:
		//Job has been updated by the backend and is still not yet enqueued
		qDebug() << Q_FUNC_INFO << " - Status: Updated" << " - Nothing done yet";
		break;
	case AB_Job_StatusEnqueued:
		//Job has been enqueued, i.e. it has not yet been sent, but will
		//be sent at the next AB_BANKING_ExecuteQueue().
		//These jobs are stored in the "todo" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Enqueued" << " - Nothing done yet";
		break;
	case AB_Job_StatusSent:
		//Job has been sent, but there is not yet any response.
		qDebug() << Q_FUNC_INFO << " - Status: Sent" << " - Nothing done yet";
		break;
	case AB_Job_StatusPending:
		//Job has been sent, and an answer has been received, so the Job
		//has been successfully sent to the bank. However, the answer to
		//this job said that the job is still pending at the bank server.
		//This status is most likely used with transfer orders which are
		//accepted by the bank server but checked (and possibly rejected)
		//later. These jobs are stored in the "pending" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Pending" << " - Nothing done yet";
		break;
	case AB_Job_StatusFinished:
		//Job has been sent, a response has been received, and everything
		//has been sucessfully executed. These jobs are stored in the
		//"finished" directory.
		qDebug() << Q_FUNC_INFO << " - Status: Finished" << " - Nothing done yet";
		break;
	case AB_Job_StatusError:
		//There was an error in jobs' execution. These jobs are stored
		//in the "finished" directory. Jobs are never enqueued twice for
		//execution, so if it has this status it will never be sent again.
		qDebug() << Q_FUNC_INFO << " - Status: Error" << " - Nothing done yet";
		break;
	case AB_Job_StatusUnknown:	//default ausführen
		//Unknown status
		qDebug() << Q_FUNC_INFO << " - Status: Unknown" << " - Nothing done yet";
	default:
		qWarning() << "WARNING: parseJobTypeCreateDatedTransfer() - Unknown Job State!";
		break;
	} /* switch(jobStatus) */

	return 1;
}




