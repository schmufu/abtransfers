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


abt_job_ctrl::abt_job_ctrl(QObject *parent) :
    QObject(parent)
{
	this->jobqueue = new QList<AB_JOB*>;
	this->log = new QStringList("created: " + QDate::currentDate().toString(Qt::DefaultLocaleLongDate));
}

abt_job_ctrl::~abt_job_ctrl()
{
	//Free all queued jobs
	// AB_Banking_free() aus aqb_banking löscht alle jobs!
	// deswegen laufen wir hier in einen segFault!
//	while (!this->jobqueue->isEmpty()) {
//		AB_Job_free(this->jobqueue->takeFirst());
//	}

	delete this->jobqueue;
	delete this->log;
}

void abt_job_ctrl::addlog(const QString &str)
{
	static QString time;
	time = QTime::currentTime().toString(Qt::DefaultLocaleShortDate);
	time.append(": ");
	time.append(str);
	this->log->append(time);
}

//SLOT
void abt_job_ctrl::addNewSingleTransfer(aqb_AccountInfo *acc, const trans_SingleTransfer *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addNewSingleDebitNote(aqb_AccountInfo *acc, const trans_SingleDebitNote *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addNewEuTransfer(aqb_AccountInfo *acc, const trans_EuTransfer *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addNewInternalTransfer(aqb_AccountInfo *acc, const trans_InternalTransfer *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addNewSepaTransfer(aqb_AccountInfo *acc, const trans_SepaTransfer *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

/******* Dated Transfers ********/

//SLOT
void abt_job_ctrl::addCreateDatedTransfer(aqb_AccountInfo *acc, const trans_DatedTransfer *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addModifyDatedTransfer(aqb_AccountInfo *acc, const trans_DatedTransfer *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addDeleteDatedTransfer(aqb_AccountInfo *acc, const trans_DatedTransfer *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addGetDatedTransfers(aqb_AccountInfo *acc)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}


/******* Standing Orders ********/


//SLOT
void abt_job_ctrl::addCreateStandingOrder(aqb_AccountInfo *acc, const trans_StandingOrder *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addModifyStandingOrder(aqb_AccountInfo *acc, const trans_StandingOrder *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addDeleteStandingOrder(aqb_AccountInfo *acc, const trans_StandingOrder *t)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
}

//SLOT
void abt_job_ctrl::addGetStandingOrders(aqb_AccountInfo *acc)
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

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(job);
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
		AB_Job_List2_PushBack(jl, this->jobqueue->at(i));
	}

	this->addlog(QString::fromUtf8("%1 Aufträge in die Jobliste übernommen").arg(this->jobqueue->count()));

	ctx = AB_ImExporterContext_new();

	this->addlog("Führe Job-Liste aus.");

	rv = AB_Banking_ExecuteJobs(banking->getAqBanking(), jl, ctx);
	if (rv) {
		qWarning() << this << "Error on execQueuedTransactions ("
				<< rv << ")";
		//cleanup
		AB_Job_List2_ClearAll(jl);
		AB_ImExporterContext_Clear(ctx);
		return;
	}

	this->parseImExporterContext(ctx);

	this->addlog("Alle Jobs übertragen und Antworten ausgewertet");

	AB_Job_List2_ClearAll(jl);
	AB_ImExporterContext_Clear(ctx);

	this->jobqueue->clear();
}

bool abt_job_ctrl::parseImExporterContext(AB_IMEXPORTER_CONTEXT *ctx)
{
	AB_IMEXPORTER_ACCOUNTINFO *ai;

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
		logmsg2.append(abt_transaction::GwenTimeToQDate(
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
		logmsg2.append(abt_transaction::GwenTimeToQDate(
						AB_AccountStatus_GetTime(s)).toString(
								Qt::DefaultLocaleLongDate));
		this->addlog(logmsg + logmsg2);

		s = AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
		cnt++;
	}
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
		strList = abt_transaction::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_transaction::GwenStringListToQStringList(l);
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
		strList = abt_transaction::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_transaction::GwenStringListToQStringList(l);
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
	QStringList strList;
	const AB_VALUE *v;
	const GWEN_STRINGLIST *l;
	int cnt = 0;

	cnt = AB_ImExporterAccountInfo_GetStandingOrderCount(ai);
	logmsg2 = QString("Count: %1").arg(cnt);
	this->addlog(logmsg + logmsg2);

	t = AB_ImExporterAccountInfo_GetFirstStandingOrder(ai);
	while (t) {
		logmsg2 = QString("Purpose:\t");
		l = AB_Transaction_GetPurpose(t);
		strList = abt_transaction::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_transaction::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetNextStandingOrder(ai);
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
		strList = abt_transaction::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_transaction::GwenStringListToQStringList(l);
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
		strList = abt_transaction::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("Value:\t");
		v = AB_Transaction_GetValue(t);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("RemoteName:\t");
		l = AB_Transaction_GetRemoteName(t);
		strList = abt_transaction::GwenStringListToQStringList(l);
		logmsg2.append(strList.join(" - "));
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetNextTransaction(ai);
	}
	return cnt;

}
















