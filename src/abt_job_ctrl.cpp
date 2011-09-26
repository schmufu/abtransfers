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

const QString abt_job_info::getStatus() const
{
	return abt_conv::JobStatusToQString(this->job);
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




void abt_job_ctrl::addlog(const QString &str)
{
	static QString time;
	time = QTime::currentTime().toString(Qt::DefaultLocaleLongDate);
	time.append(": ");
	time.append(str);

	emit this->log(time);
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
void abt_job_ctrl::addGetDatedTransfers(const aqb_AccountInfo *acc)
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
	info.append("INFO: Holt alle noch nicht ausgeführten Terminüberweisungen");

	abt_job_info *ji = new abt_job_info(job, info);


	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
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
	//mit dem parsen fertig sind (damit dies die DAs neu laden kann)
	connect(this, SIGNAL(standingOrdersParsed()),
		acc, SLOT(loadKnownStandingOrders()));
	connect(this, SIGNAL(datedTransfersParsed()),
		acc, SLOT(loadKnownDatedTransfers()));

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
		this->addlog(QString("** ERROR ** Fehler bei AB_Baning_ExecuteJobs(). return value = %1 ** ABBRUCH **").arg(rv));
		AB_Job_List2_ClearAll(jl);
		AB_ImExporterContext_Clear(ctx);
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

	this->parseImExporterContext(ctx);

	this->addlog("Alle Jobs übertragen und Antworten ausgewertet");

	AB_Job_List2_ClearAll(jl);
	AB_ImExporterContext_Clear(ctx);

	//Alle Objecte in der jobqueue liste löschen
	while (!this->jobqueue->isEmpty()) {
		abt_job_info *j = this->jobqueue->takeFirst();
		//AB_Job_free(j->getJob());
		delete j;
	}
	emit this->jobQueueListChanged();
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
		this->parseImExporterAccountInfo_Status(ai);
		this->parseImExporterAccountInfo_DatedTransfers(ai);
		this->parseImExporterAccountInfo_NotedTransactions(ai);
		this->parseImExporterAccountInfo_StandingOrders(ai);
		this->parseImExporterAccountInfo_Transactions(ai);
		this->parseImExporterAccountInfo_Transfers(ai);

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
	QStringList strList;
	const AB_VALUE *v;
	const GWEN_STRINGLIST *l;
	int cnt = 0;

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

		t = AB_ImExporterAccountInfo_GetNextDatedTransfer(ai);
	}
	return cnt;

}

int abt_job_ctrl::parseImExporterAccountInfo_NotedTransactions(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
	AB_TRANSACTION *t;
	QString logmsg = "Recvd-NotedTransactions: ";
	QString logmsg2;
	QStringList strList, DTIDs;
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

		/*! \todo NOCH NICHT FERTIG! Momentan wird in Daueraufträge.ini
		 *	  gespeichert.
		 */
		//Bei der Bank hinterlegte Terminüberweisung auch lokal speichern
		this->addlog(QString(
			"Speichere bei der Bank hinterlegte Terminüberweisung (ID: %1)"
			).arg(AB_Transaction_GetFiId(t)));
		abt_transaction::saveTransaction(t);
		DTIDs.append(QString::fromUtf8(AB_Transaction_GetFiId(t)));

		t = AB_ImExporterAccountInfo_GetNextNotedTransaction(ai);
	}

	if (DAIDs.size() > 0) {
		//Die lokal gespeicherten DatedTransfers auch in den Einstellungen merken.
		QString KtoNr = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountNumber(ai));
		QString BLZ = QString::fromUtf8(AB_ImExporterAccountInfo_GetBankCode(ai));

		settings->saveDatedTransfersForAccount(DTIDs, KtoNr, BLZ);

		emit this->datedTransfersParsed();
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
		abt_transaction::saveTransaction(t);
		DAIDs.append(QString::fromUtf8(AB_Transaction_GetFiId(t)));
		t = AB_ImExporterAccountInfo_GetNextStandingOrder(ai);
	}

	/* DONE   Das Speichern wird auch ausgeführt wenn keine DAs empfangen
		  wurden. Somit werden alle vorhandenen DAs gelöscht!
		  Es muss nur gespeichert werden wenn auch DAs empfangen wurden!
	*/
	if (DAIDs.size() > 0) {
		//Die lokal gespeicherten DAs auch in den Einstellungen merken.
		QString KtoNr = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountNumber(ai));
		QString BLZ = QString::fromUtf8(AB_ImExporterAccountInfo_GetBankCode(ai));

		settings->saveStandingOrdersForAccount(DAIDs, KtoNr, BLZ);

		emit this->standingOrdersParsed();
	}

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
	GWEN_STRINGLIST *strl;
	QString strState;
	QString strType;
	QStringList strList;
	bool res=true;
	static int run = 0;
	run++;
	qDebug() << "in checkJobStatus() - RUN: " << run;

	AB_JOB_LIST2_ITERATOR *jli;
	jli = AB_Job_List2Iterator_new(jl);
	jli = AB_Job_List2_First(jl);
	j = AB_Job_List2Iterator_Data(jli);
	while (j) {
		qDebug() << "in checkJobStatus()-while - RUN: " << run;
		type = AB_Job_GetType(j);
		strType = AB_Job_Type2Char(type);
		this->addlog(QString("JobType: ").append(strType));
		state = AB_Job_GetStatus(j);
		if (state == AB_Job_StatusPending) {
			res = false;
		}
		strState = AB_Job_Status2Char(state);
		this->addlog(QString("JobState: ").append(strState));

		strl = AB_Job_GetLogs(j);
		strList = abt_conv::GwenStringListToQStringList(strl);
		GWEN_StringList_free(strl); // \done macht jetzt abt_conv selbst oder?
					    //NEIN! QStringlistToGwenStringList löscht sich selbst!
					    //GwenToQ macht dies nicht!

		//die logs von aqBanking ein wenig aufbereiten
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
		strList.replaceInStrings("%C3%BC", "ö", Qt::CaseSensitive);
		// %C3%96 durch Ö ersetzen
		strList.replaceInStrings("%C3%96", "Ö", Qt::CaseSensitive);
		// %3D durch = ersetzen
		strList.replaceInStrings("%3D", "=", Qt::CaseSensitive);

		//Alle Strings der StringListe zum Log hinzufügen
		for (int i=0; i<strList.count(); ++i) {
			this->addlog(QString("JobLog: ").append(strList.at(i)));
		}

		j = AB_Job_List2Iterator_Next(jli);
	}

	AB_Job_List2Iterator_free(jli);

	qDebug() << "in checkJobStatus()-end - RUN: " << run;

	return res;
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










