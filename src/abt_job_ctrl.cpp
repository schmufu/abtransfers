/******************************************************************************
 * Copyright (C) 2011-2012 Patrick Wacker
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
#include <QMessageBox>

#include <aqbanking/job.h>
#include <aqbanking/transaction.h>

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

#include "abt_parser.h"

#include "globalvars.h"
#include "abt_conv.h"



abt_job_ctrl::abt_job_ctrl(aqb_Accounts *allAccounts, abt_history *history,
			   QObject *parent) :
    QObject(parent)
{
	this->jobqueue = new QList<abt_jobInfo*>;
	this->m_allAccounts = allAccounts;
	this->m_history = history;

	emit this->log(tr("Job-Controller erstellt (%1)").arg(
			QDate::currentDate().toString(Qt::SystemLocaleLongDate)));
}

abt_job_ctrl::~abt_job_ctrl()
{
	//Free all queued abt_jobInfo's
	while (!this->jobqueue->isEmpty()) {
		abt_jobInfo *j = this->jobqueue->takeFirst();
		//The job inside the jobInfo must not exist and is owned by
		//AqBanking, so we dont free it!
		delete j;
	}

	delete this->jobqueue;
        this->jobqueue = NULL;

	qDebug() << Q_FUNC_INFO << "deleted";
}

//static public
/**
 * @brief stores the availability of every AB_JOB_TYPE in the QHash
 *
 * This static function retrieves the availability of each AB_JOB_TYPE and
 * stores the result in the supplied QHash.
 *
 * Example usage of the information in the QHash:
 * @code
 *	if (hash->value(AB_Job_TypeTranfer)) {
 *		//transfers are supported
 *	} else {
 *		//transfers are not supported
 *	}
 * @endcode
 *
 * @param a: account from AqBanking for which the hash is filled
 * @param hash: pointer to the QHash that should be used
 */
void abt_job_ctrl::createAvailableHashFor(AB_ACCOUNT *a,
					  QHash<AB_JOB_TYPE, bool> *hash)
{
	Q_ASSERT(hash != NULL);
	AB_JOB *j = NULL;

	j = AB_JobCreateDatedTransfer_new(a);
	hash->insert(AB_Job_TypeCreateDatedTransfer, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobCreateStandingOrder_new(a);
	hash->insert(AB_Job_TypeCreateStandingOrder, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobSingleDebitNote_new(a);
	hash->insert(AB_Job_TypeDebitNote, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobDeleteDatedTransfer_new(a);
	hash->insert(AB_Job_TypeDeleteDatedTransfer, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobDeleteStandingOrder_new(a);
	hash->insert(AB_Job_TypeDeleteStandingOrder, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobEuTransfer_new(a);
	hash->insert(AB_Job_TypeEuTransfer, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobGetBalance_new(a);
	hash->insert(AB_Job_TypeGetBalance, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobGetDatedTransfers_new(a);
	hash->insert(AB_Job_TypeGetDatedTransfers, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobGetStandingOrders_new(a);
	hash->insert(AB_Job_TypeGetStandingOrders, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobGetBalance_new(a);
	hash->insert(AB_Job_TypeGetTransactions, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobInternalTransfer_new(a);
	hash->insert(AB_Job_TypeInternalTransfer, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobLoadCellPhone_new(a);
	hash->insert(AB_Job_TypeLoadCellPhone, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobModifyDatedTransfer_new(a);
	hash->insert(AB_Job_TypeModifyDatedTransfer, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobModifyStandingOrder_new(a);
	hash->insert(AB_Job_TypeModifyStandingOrder, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobSepaDebitNote_new(a);
	hash->insert(AB_Job_TypeSepaDebitNote, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobSepaTransfer_new(a);
	hash->insert(AB_Job_TypeSepaTransfer, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

	j = AB_JobSingleTransfer_new(a);
	hash->insert(AB_Job_TypeTransfer, AB_Job_CheckAvailability(j) == 0);
	AB_Job_free(j);

}

//static public
/**
 * @brief retrieves all Limits from AqBanking and stores it in the QHash
 *
 * This static function retrieves all TransactionLimits that are supplied by
 * the bank as BPD (Bank Parameter Daten) and UPD (User Parameter Daten) and
 * stores this as a @ref abt_transactionLimits in the QHash @a ah (AccountHash)
 *
 * @param a: account from AqBanking for which the hash is filled
 * @param ah: pointer to the QHash that should be used
 */
void abt_job_ctrl::createTransactionLimitsFor(AB_ACCOUNT *a,
					      QHash<AB_JOB_TYPE, abt_transactionLimits*> *ah)
{
	//ah: AccountHash
	//a: Account
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


	/** @todo AB_JobEuTransder_GetFielsLimits does not exists!
	 *	  Where can we get the FieldLimits for an EU-Transfer?
	 */
//	j = AB_JobEuTransfer_new(a);
//	if (AB_Job_CheckAvailability(j)) {
//		qDebug("Job EuTransfer not available");
//	} else {
//		AB_JobEuTransfer_SetTransaction(j, t);
//		tl = AB_JobSingleTransfer_GetFieldLimits(j);
//		if (tl) {
//			abt_transactionLimits *limits = new abt_transactionLimits(tl);
//			ah->insert(AB_Job_TypeEuTransfer, limits);
//		} else {
//			qDebug("tl not set!");
//		}
//	}
//	AB_Job_free(j);

	j = AB_JobSepaDebitNote_new(a);
	if (AB_Job_CheckAvailability(j)) {
		qDebug("Job SepaDebitNote not available");
	} else {
		AB_JobSepaDebitNote_SetTransaction(j, t);
		tl = AB_JobSepaDebitNote_GetFieldLimits(j);
		if (tl) {
			abt_transactionLimits *limits = new abt_transactionLimits(tl);
			ah->insert(AB_Job_TypeSepaDebitNote, limits);
		} else {
			qDebug("tl not set!");
		}
	}
	AB_Job_free(j);

	AB_Transaction_free(t);
}



//private
/**
 * @brief prepares the supplied QString for a log-message
 *
 * The current time is prepended before the @a str and then the signal
 * @ref log() is emitted with the modified @a str
 *
 * @param str: the message that should be logged
 */
void abt_job_ctrl::addlog(const QString &str)
{
	static QString time;
	time = QTime::currentTime().toString("HH:mm:ss.zzz");
	time.append(": ");
	time.append(str);

	emit this->log(time);
}

//private
/**
 * @brief gets the logs from a AB_JOB and returns them as a QStringList
 *
 * With AB_Job_GetLogs(AB_JOB) the logs from the AB_JOB @a j are retrieved
 * and stored in a QStringList.
 *
 * The logs from the job are preprocessed and every "unknown" UTF8 character
 * is replaced by the corresponding ASCII character.
 *
 * @param j: the AB_JOB which logs should be processed
 * @returns a QStringList with the preprocessed logs
 */
QStringList abt_job_ctrl::getParsedJobLogs(const AB_JOB *j) const
{
	QStringList strList;
	GWEN_STRINGLIST *gwenStrList;

	gwenStrList = AB_Job_GetLogs(j);
	if (gwenStrList) { //only if logs are existent
		strList = abt_conv::GwenStringListToQStringList(gwenStrList);
		//the GWEN_StringList is no longer needed
		GWEN_StringList_free(gwenStrList);
	}

	//preprocess the logs from AqBanking (UTF8 to ASCII)
	//replace %22 by "
	strList.replaceInStrings("%22", "\"", Qt::CaseSensitive);
	//replace %28 by (
	strList.replaceInStrings("%28", "(", Qt::CaseSensitive);
	//replace %29 by )
	strList.replaceInStrings("%29", ")", Qt::CaseSensitive);
	//replace %3A by :
	strList.replaceInStrings("%3A", ":", Qt::CaseSensitive);
	//replace %C3%A4 by ä
	strList.replaceInStrings("%C3%A4", "ä", Qt::CaseSensitive);
	//replace %C3%84 by Ä
	strList.replaceInStrings("%C3%84", "Ä", Qt::CaseSensitive);
	//replace %C3%BC by ü
	strList.replaceInStrings("%C3%BC", "ü", Qt::CaseSensitive);
	//replace %C3%9C by Ü
	strList.replaceInStrings("%C3%9C", "Ü", Qt::CaseSensitive);
	//replace %C3%B6 by ö
	strList.replaceInStrings("%C3%B6", "ö", Qt::CaseSensitive);
	//replace %C3%96 by Ö
	strList.replaceInStrings("%C3%96", "Ö", Qt::CaseSensitive);
	//replace %3D by =
	strList.replaceInStrings("%3D", "=", Qt::CaseSensitive);

	return strList;
}

//private
/**
 * @brief returns the position of the first job for the account
 *
 * This function iterates over the @ref jobqueue and returns the position
 * of the first job in the jobqueue which belongs to the account @a acc.
 *
 * If no job for the account @a acc is in the queue the size auf the jobqueue
 * is returned (could be used for inserting at the end of the list).
 *
 * @param acc: the AB_ACCOUNT for which the first job should be found.
 * @returns the position in the jobqueue of the first job for the account
 */
int abt_job_ctrl::getSupposedJobqueue_FirstPos(const AB_ACCOUNT *acc) const
{
	int firstPos = this->jobqueue->size(); //default: last position

	for(int pos=0; pos<this->jobqueue->size(); ++pos) {
		if (this->jobqueue->at(pos)->getAbAccount() == acc) {
			firstPos = pos;
			break; //position found, cancel loop
		}
	}

	return firstPos;
}

//private
/**
 * @brief returns the position of the last job for the account
 *
 * Iterates over the @ref jobqueue from @a firstPos on and stops if the job
 * isn't for the account at the @a firstpos.
 *
 * @param start: the position of the first job in queue for the wanted account
 * @returns the position in the jobqueue of the last job for the account
 */
int abt_job_ctrl::getSupposedJobqueue_LastPos(int firstPos) const
{
	if (this->jobqueue->size()-1 <= firstPos) {
		return firstPos;
	}

	const AB_ACCOUNT *acc = this->jobqueue->at(firstPos)->getAbAccount();

	//default (that we can check for errors)
	int lastPos = -1;

	//omit the first one, we know its identical
	for (int pos=firstPos+1; pos<this->jobqueue->size(); ++pos) {
		if (this->jobqueue->at(pos)->getAbAccount() != acc) {
			lastPos = pos - 1; //the previous position was the last
			break; //position found, cancel loop
		}
	}

	//maybe all jobs in the queue are for the same account, then
	//lastPos is not set correctly. Check it.
	if (lastPos == -1) {
		lastPos = this->jobqueue->size()-1;
	}

	//the position of the last job in queue for this account
	return lastPos;
}

//private
/**
 * @brief determines the position in the jobqueue where the next transfer job
 *	  should be inserted
 *
 * @param firstTransferPos: the position in the queue where the first transfer
 *                          job is
 * @param lastPos: the position of the last job in queue that belongs to the
 *                 same account
 * @returns the position where the next transfer should be inserteds
 */
int abt_job_ctrl::getSupposedJobqueue_NextTransferPos(int firstTransferPos,
						      int lastPos) const
{
	int wantedPos = -1;

	//the transfer job types should be at "top"
	for (int pos=firstTransferPos; pos<=lastPos; ++pos) {
		if (this->jobqueue->size() <= pos) {
			break; //nothing can match
		}
		AB_JOB_TYPE type = this->jobqueue->at(pos)->getAbJobType();
		switch (type) {
		case AB_Job_TypeTransfer:
		case AB_Job_TypeInternalTransfer:
		case AB_Job_TypeEuTransfer:
		case AB_Job_TypeSepaTransfer:
		case AB_Job_TypeSepaDebitNote:
		case AB_Job_TypeDebitNote:
		case AB_Job_TypeLoadCellPhone:
			continue; //next position in queue
			break;
		default:
			//pos is the position we are searching for
			wantedPos = pos;
			break;
		}
		if (wantedPos != -1) {
			break; //position found, cancel loop
		}
	}

	//if the for-loop does not set wantedPos (this could be if only transfers
	//for the account are in the queue) we want to be after the last position
	if (wantedPos == -1) {
		wantedPos = lastPos + 1;
	}

	return wantedPos;
}

//private
/**
 * @brief determines the position in the jobqueue where the next standing order
 *	  job should be inserted
 *
 * @param firstStandingPos: the position of the first standingOrder job
 *                          (edit, del, modify)
 * @param lastPos: the position of the last job in queue that belongs to the
 *                 same account
 * @returns the position where the next standingOrder job should be inserted
 */
int abt_job_ctrl::getSupposedJobqueue_NextStandingPos(int firstStandingPos,
						      int lastPos) const
{
	int wantedPos = -1;

	//find the position of the last standingOrder job
	for (int pos=firstStandingPos; pos<=lastPos; ++pos) {
		AB_JOB_TYPE type = this->jobqueue->at(pos)->getAbJobType();
		if ((type == AB_Job_TypeCreateStandingOrder) ||
		    (type == AB_Job_TypeModifyStandingOrder) ||
		    (type == AB_Job_TypeDeleteStandingOrder)) {
			continue; //next position in queue
		}
		//pos is now the position of the job just after the above types
		wantedPos = pos;
		break;
	}

	//if the for-loop does not set wantedPos, which happen when the
	//following jobs are all standingOrder change jobs OR no jobs are
	//present after the firstStandingPos. Then we want to be after
	//the last position.
	if (wantedPos == -1) {
		wantedPos = lastPos + 1;
	}

	return wantedPos;
}

//private
/**
 * @brief determines the position in the jobqueue where the next dated transfer
 *	  job should be inserted
 *
 * @param firstDatedPos: the position of the first datedTransfer job
 *                       (edit, del, modify)
 * @param lastPos: the position of the last job in queue that belongs to the
 *                 same account
 * @returns the position where the next datedTransfer job should be inserted
 */
int abt_job_ctrl::getSupposedJobqueue_NextDatedPos(int firstDatedPos,
						   int lastPos) const
{
	int wantedPos = -1;

	//find the position of the last datedTransfer job
	for (int pos=firstDatedPos; pos<=lastPos; ++pos) {
		AB_JOB_TYPE qtype = this->jobqueue->at(pos)->getAbJobType();
		if ((qtype == AB_Job_TypeCreateDatedTransfer) ||
		    (qtype == AB_Job_TypeModifyDatedTransfer) ||
		    (qtype == AB_Job_TypeDeleteDatedTransfer)) {
			continue; //next position in queue
		}
		//pos is now the job just after the above types
		wantedPos = pos;
		break;
	}
	//if the for-loop does not set wantedPos we want to be after the
	//last position
	if (wantedPos == -1) {
		wantedPos = lastPos + 1;
	}

	return wantedPos;
}

//private
/**
 * @brief determines the position in the jobqueue for the supplied job
 *
 * This function should be used to determine the position in the jobqueue
 * where the supplied @a job should be inserted.
 *
 * The jobqueue is structured as follows for every account:
 *  @li transfers (internal, sepa, etc)
 *  @li standingOrders changes (edit, del, modify)
 *  @li getStandingOrders
 *  @li datedTransfer changes (edit, del, modify)
 *  @li getDatedTransfers
 *  @li getBalance
 *
 *
 * If more than one transfer, standing order change or dated transfer change
 * job is in the list, the supposed position is just after the existing ones.
 *
 * @returns the supposed position in the jobqueue for the supplied @a job
 */
int abt_job_ctrl::getSupposedJobqueuePos(const abt_jobInfo *job) const
{
	//the first occurrence of a job for the account in the jobqueue
	int firstPos = this->getSupposedJobqueue_FirstPos(job->getAbAccount());
	//the last occurrence of a job for the account in the jobqueue
	int lastPos = this->getSupposedJobqueue_LastPos(firstPos);

	int wantedPos = lastPos + 1; //default, after the last one

	//we have the positions where our jobs for the account are in the queue.
	//Where the current job goes, depends on the type of the job
	switch (job->getAbJobType()) {
	case AB_Job_TypeTransfer:
	case AB_Job_TypeInternalTransfer:
	case AB_Job_TypeEuTransfer:
	case AB_Job_TypeSepaTransfer:
	case AB_Job_TypeSepaDebitNote:
	case AB_Job_TypeDebitNote:
	case AB_Job_TypeLoadCellPhone: {
		//these job types should be at "top"
		wantedPos = this->getSupposedJobqueue_NextTransferPos(firstPos,
								      lastPos);
		break;
	}
	case AB_Job_TypeCreateStandingOrder:
	case AB_Job_TypeModifyStandingOrder:
	case AB_Job_TypeDeleteStandingOrder: {
		//these job types should be after the transfer jobs
		int firstStandingPos = this->getSupposedJobqueue_NextTransferPos(firstPos, lastPos);
		wantedPos = this->getSupposedJobqueue_NextStandingPos(firstStandingPos, lastPos);
		break;
	}
	case AB_Job_TypeGetStandingOrders: {
		//the update of standingOrders should be after the standingOrder changes
		int firstStandingPos = this->getSupposedJobqueue_NextTransferPos(firstPos, lastPos);
		//the update of standingOrders should be direct after the last
		//job that changes a standingOrder, so the position is equal to
		//the position of the next standingOrder
		wantedPos = this->getSupposedJobqueue_NextStandingPos(firstStandingPos, lastPos);
		break;
	}
	case AB_Job_TypeCreateDatedTransfer:
	case AB_Job_TypeModifyDatedTransfer:
	case AB_Job_TypeDeleteDatedTransfer: {
		//these job types should be after the standingOrder jobs
		int firstStandingPos = this->getSupposedJobqueue_NextTransferPos(firstPos, lastPos);
		int firstDatedPos = this->getSupposedJobqueue_NextStandingPos(firstStandingPos, lastPos);
		if (this->isJobTypeInQueue(AB_Job_TypeGetStandingOrders, job->getAbAccount())) {
			//we have a update job for standingOrders, skip it
			firstDatedPos++;
		}
		wantedPos = this->getSupposedJobqueue_NextDatedPos(firstDatedPos, lastPos);
		break;
	}
	case AB_Job_TypeGetDatedTransfers: {
		//the update of datedTransfers should be after the datedTransfer changes
		int firstStandingPos = this->getSupposedJobqueue_NextTransferPos(firstPos, lastPos);
		int firstDatedPos = this->getSupposedJobqueue_NextStandingPos(firstStandingPos, lastPos);
		if (this->isJobTypeInQueue(AB_Job_TypeGetStandingOrders, job->getAbAccount())) {
			//we have a update job for standingOrders, skip it
			firstDatedPos++;
		}
		//the update of datedTransfers should be direct after the last
		//job that changes a datedTransfer, so the position is equal to
		//the position of the next datedTransfer
		wantedPos = this->getSupposedJobqueue_NextDatedPos(firstDatedPos, lastPos);
		break;
	}
	case AB_Job_TypeGetBalance: {
		//the update of the balance should always be the last job
		wantedPos = lastPos + 1;
	}
	default:
		wantedPos = lastPos + 1; //default, after the last one
		break;
	}

	//security checks
	if (wantedPos > this->jobqueue->size()) {
		wantedPos = this->jobqueue->size(); //last possible position
	}

	if (wantedPos < 0) {
		wantedPos = 0; //first possible position
	}

	return wantedPos;

}


//public slot
/**
 * @brief adds a new single transfer to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addNewSingleTransfer(const aqb_AccountInfo *acc,
					const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobSingleTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeTransfer);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobSingleTransfer_SetTransaction(job, t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//public slot
/**
 * @brief adds a new single debit note to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addNewSingleDebitNote(const aqb_AccountInfo *acc,
					 const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobSingleDebitNote_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeDebitNote);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobSingleDebitNote_SetTransaction(job, t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//public slot
/**
 * @brief adds a new eu transfer to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addNewEuTransfer(const aqb_AccountInfo *acc,
				    const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobEuTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeEuTransfer);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobEuTransfer_SetTransaction(job, t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//public slot
/**
 * @brief adds a new internal transfer to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addNewInternalTransfer(const aqb_AccountInfo *acc,
					  const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobInternalTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeInternalTransfer);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobInternalTransfer_SetTransaction(job, t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//public slot
/**
 * @brief adds a new sepa transfer to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addNewSepaTransfer(const aqb_AccountInfo *acc,
				      const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobSepaTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeSepaTransfer);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobSepaTransfer_SetTransaction(job, t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}


/******* Dated Transfers ********/


//public slot
/**
 * @brief adds a create dated transfer to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addCreateDatedTransfer(const aqb_AccountInfo *acc,
					  const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobCreateDatedTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeCreateDatedTransfer);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobCreateDatedTransfer_SetTransaction(job,
						      t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();

	/** \todo Muss dies stattfinden? Siehe auch ToDo-Eintrag bei
		  Daueraufträgen.
		  Wobei die transaction aus der aktualisierung unter umständen
		  Andere Daten enthält als die transaction aus dem job!
		  Es wurd ein Dauerauftrag mit Letztmalig zur Bank geschickt,
		  bei der Aktualisierung wurde dieser allerdings ohne Enddatum
		  zurück gemeldet!
	*/
//	if (!this->isJobTypeInQueue(AB_Job_TypeGetDatedTransfers, ji)) {
//		//nach dem erstellen muss eine Aktualisierung stattfinden.
//		this->addGetDatedTransfers(acc, true);
//	}


}

//public slot
/**
 * @brief adds a modify dated tranfer to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addModifyDatedTransfer(const aqb_AccountInfo *acc,
					  const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobModifyDatedTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeModifyDatedTransfer);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobModifyDatedTransfer_SetTransaction(job,
						      t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();

	//after a change, the dated transfers must be retrieved from the bank
	if (!this->isJobTypeInQueue(AB_Job_TypeGetDatedTransfers,
				    ji->getAbAccount())) {
		this->addGetDatedTransfers(acc, true);
	}

}

//public slot
/**
 * @brief adds a delete dated tranfer to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addDeleteDatedTransfer(const aqb_AccountInfo *acc,
					  const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobDeleteDatedTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeDeleteDatedTransfer);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobDeleteDatedTransfer_SetTransaction(job,
						      t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//public slot
/**
 * @brief adds a get dated transfers to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addGetDatedTransfers(const aqb_AccountInfo *acc,
					bool withoutInfo /*=false*/)
{
	int rv;

	if (acc == NULL) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job AB_Job_TypeGetDatedTransfers is not"
			   << "available (no valid account [NULL])";
		emit jobNotAvailable(AB_Job_TypeGetDatedTransfers);
		return; //cancel adding
	}

	//this jobtype should only be send once
	if (this->isJobTypeInQueue(AB_Job_TypeGetDatedTransfers,
				   acc->get_AB_ACCOUNT())) {
		return; //already in queue, nothing to do
	}

	AB_JOB *job = AB_JobGetDatedTransfers_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeGetDatedTransfers);
		return; //cancel adding
	}

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	if (!withoutInfo) { //only emit a jobAdded() signal if wanted
		emit this->jobAdded(ji);
	}

	emit this->jobQueueListChanged();
}


/******* Standing Orders ********/


//public slot
/**
 * @brief adds a create standing order to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addCreateStandingOrder(const aqb_AccountInfo *acc,
					  const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobCreateStandingOrder_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeCreateStandingOrder);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobCreateStandingOrder_SetTransaction(job,
						      t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();

	/** \todo Muss dies stattfinden? Wenn es durchgeführt wird erscheint
		  der Dauerauftrag doppelt!
		  Wobei die transaction aus der aktualisierung unter umständen
		  Andere Daten enthält als die transaction aus dem job!
		  Es wurd ein DA mit Letztmalig zur Bank geschickt, bei der
		  Aktualisierung wurde dieser allerdings ohne Enddatum zurück
		  gemeldet!
	*/
//	if (!this->isJobTypeInQueue(AB_Job_TypeGetStandingOrders, ji)) {
//		//nach dem erstellen muss eine Aktualisierung stattfinden.
//		this->addGetStandingOrders(acc, true);
//	}

}

//public slot
/**
 * @brief adds a modify standing order to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addModifyStandingOrder(const aqb_AccountInfo *acc,
					  const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobModifyStandingOrder_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeModifyStandingOrder);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobModifyStandingOrder_SetTransaction(job,
						      t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();

	//after a change, the standing orders must be retrieved from the bank
	if (!this->isJobTypeInQueue(AB_Job_TypeGetStandingOrders,
				    ji->getAbAccount())) {
		this->addGetStandingOrders(acc, true);
	}

}

//public slot
/**
 * @brief adds a delete standing order to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addDeleteStandingOrder(const aqb_AccountInfo *acc,
					  const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobDeleteStandingOrder_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeDeleteStandingOrder);
		return; //cancel adding
	}

	//add transaction to the job
	rv = AB_JobDeleteStandingOrder_SetTransaction(job,
						      t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//public slot
/**
 * @brief adds a get standing orders to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addGetStandingOrders(const aqb_AccountInfo *acc,
					bool withoutInfo /*=false*/)
{
	int rv;

	if (acc == NULL) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job AB_Job_TypeGetStandingOrders is not"
			   << "available (no valid account [NULL])";
		emit jobNotAvailable(AB_Job_TypeGetStandingOrders);
		return; //abort
	}

	//this jobtype should only be send once
	if (this->isJobTypeInQueue(AB_Job_TypeGetStandingOrders,
				   acc->get_AB_ACCOUNT())) {
		return; //already in queue, nothing to do
	}


	AB_JOB *job = AB_JobGetStandingOrders_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeGetStandingOrders);
		return; //cancel adding
	}

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	if (!withoutInfo) { //only emit a jobAdded() signal if wanted
		emit this->jobAdded(ji);
	}

	emit this->jobQueueListChanged();

	//Das zuständige AccountInfo Object darüber informieren wenn wir
	//mit dem parsen fertig sind (damit dies die SOs neu laden kann)
	//--> wird jetzt direkt beim parsen erledigt!
//	connect(this, SIGNAL(standingOrdersParsed()),
//		acc, SLOT(loadKnownStandingOrders()));

}



//public slot
/**
 * @brief adds a get balance to the jobqueue
 *
 * Creates a new AB_JOB and checks if it is supported.
 *
 * Depending on the type the job is added at the right position in the jobqueue.
 *
 * When everything went fine the signal @ref jobAdded() and
 * @ref jobQueueListChanged() are emitted, otherwise the signal
 * @ref jobNotAvailable() is emitted.
 *
 * @param acc: the account
 * @param t: the transaction data
 */
void abt_job_ctrl::addGetBalance(const aqb_AccountInfo *acc,
				 bool withoutInfo /*=false*/)
{
	int rv;

	if (acc == NULL) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job AB_Job_TypeGetBalance is not available"
			   << "(no valid account [NULL])";
		emit jobNotAvailable(AB_Job_TypeGetBalance);
		return; //cancel
	}

	//this jobtype should only be send once
	if (this->isJobTypeInQueue(AB_Job_TypeGetBalance,
				   acc->get_AB_ACCOUNT())) {
		return; //already in queue, nothing to do
	}


	AB_JOB *job = AB_JobGetBalance_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) { //Job is not available!
		qWarning() << Q_FUNC_INFO
			   << "Job is not available -"
			   << "AB_Job_CheckAvailability returned:" << rv;
		emit jobNotAvailable(AB_Job_TypeGetBalance);
		return; //cancel adding
	}

	abt_jobInfo *ji = new abt_jobInfo(job);

	//get the right position and insert the job in the jobqueue (at
	//execution time the AB_JOB_LIST is created from the jobs in the queue)
	int pos = this->getSupposedJobqueuePos(ji);
	this->jobqueue->insert(pos, ji);

	if (!withoutInfo) { //only emit a jobAdded() signal if wanted
		emit this->jobAdded(ji);
	}

	emit this->jobQueueListChanged();
}



/******** execution *******/


//public slot
/**
 * @brief executes all queued jobs and calls the functions for parsing the
 *	  retrieved information
 *
 */
void abt_job_ctrl::execQueuedTransactions()
{
	int rv;
	AB_JOB_LIST2 *jl;
	AB_IMEXPORTER_CONTEXT *ctx;

	this->addlog(tr("Erstelle Job-Liste."));

	jl = AB_Job_List2_new();

	//append all jobs from the jobqueue to the AB_JOB_LIST2
	for (int i=0; i<this->jobqueue->size(); ++i) {
		AB_Job_List2_PushBack(jl, this->jobqueue->at(i)->getJob());
	}

	this->addlog(tr("%1 Aufträge in die Jobliste übernommen")
		     .arg(this->jobqueue->count()));


	ctx = AB_ImExporterContext_new();

	this->addlog(tr("Führe Job-Liste aus."));

	rv = AB_Banking_ExecuteJobs(banking->getAqBanking(), jl, ctx);
	if (rv) {
		qWarning() << Q_FUNC_INFO
			   << "AB_Banking_ExecuteJobs() returned" << rv;

		this->addlog(tr("*********************************************"
				"** E R R O R                               **"
				"** Fehler bei AB_Banking_ExecuteJobs().    **"
				"** return value = %1                       **"
				"**                                         **"
				"** Es wird abgebrochen und keine weitere   **"
				"** Bearbeitung/Auswertung durchgeführt!    **"
				"*********************************************")
			     .arg(rv));

		/** @todo cleanup is required, but not tested.
		 * This case doesnt occur yet and therefore i dont know if
		 * the following is working.
		 * We must remove all abt_jobInfo objects from the jobqueue too!
		 * Depending on what is wanted, all jobs must be recreated and
		 * attached to the jobqueue?
		 */

		AB_Job_List2_FreeAll(jl); //this also deletes all AB_JOB's
		AB_ImExporterContext_Clear(ctx);
		AB_ImExporterContext_free(ctx);

		while (!this->jobqueue->isEmpty()) {
			abt_jobInfo *j = this->jobqueue->takeFirst();
			//The job inside the jobInfo must not exist and is
			//owned by AqBanking, so we dont free it!
			delete j;
		}

		//the queuelist changed, tell everyone who wants to know
		emit this->jobQueueListChanged();

		return;
	}


	bool successfull = this->parseExecutedJobs(jl);

	//analyze the retrieved information and set it in the corresponding
	//accounts
	abt_parser::parse_ctx(ctx, this->m_allAccounts);

	this->addlog(tr("Alle Jobs übertragen und Antworten ausgewertet"));

	//we free the AB_JOB_LIST2. This also frees all AB_JOB objects in the
	//list and all data that is stored there!
	//All objects that took a AB_JOB as argument, must not work with this
	//reference (normaly the make a copy of the data they need)
	AB_Job_List2_FreeAll(jl); //this also deletes all AB_JOB's
	AB_ImExporterContext_Clear(ctx);
	AB_ImExporterContext_free(ctx);

	//the queuelist definitive changed, we tell everyone who wants to know
	emit this->jobQueueListChanged();

	//Tell the user if the execution was erroneous
	if (!successfull) {
		/** @todo the abt_job_ctrl should not use GUI elements.
		 * This could be send as signal and handled by the mainwindow
		 */
		QMessageBox::critical(qobject_cast<QWidget*>(this->parent()),
				      tr("Fehlerhafte Ausführung"),
				      tr("<b>Die Aufträge wurden nicht erfolgreich "
					 "ausgeführt!</b><br /><br />"
					 "Alle Aufträge die nicht erfolgreich "
					 "ausgeführt werden konnten befinden sich "
					 "weiterhin im Ausgang. Dort können diese "
					 "Aufträge über das Kontext-Menü (rechte "
					 "Maustaste) bearbeitet und korrigiert "
					 "oder gelöscht werden.<br />"
					 "Bitte beachten Sie auch die Ausgaben "
					 "im \"Log\"-Fenster um Hinweise für die "
					 "fehlerhafte Ausführung zu erhalten."),
				      QMessageBox::Ok, QMessageBox::Ok);
	}

}

//private
/**
 * @brief checks the executed AB_JOB_LIST2 @a jl for errors
 *
 * checks the status of the executed jobs and moves them, if faultless, to
 * the history-list.
 *
 * If an error occured the job is removed from the AB_JOB_LIST2 @a jl and is
 * therefore still useable.
 *
 * @param jl: the AB_JOB_LIST2 to check
 * @return true: if all jobs executed without fault
 * @return false: if one ore more jobs are faulty
 */
bool abt_job_ctrl::parseExecutedJobs(AB_JOB_LIST2 *jl)
{
	AB_JOB *j = NULL;
	AB_JOB_STATUS jobState;
	AB_JOB_TYPE jobType;
	QString strType;
	QStringList strList;
	qDebug() << Q_FUNC_INFO << "started";
	bool ret = true; //default - everything ok

	//The jobs were send to the bank and maybe changed by the backend.
	//We go trough the jobs and parse them according to there state.

	AB_JOB_LIST2_ITERATOR *jli;
	jli = AB_Job_List2_First(jl);
	if (jli) {
		j = AB_Job_List2Iterator_Data(jli);
	}
	while (j) {
		jobType = AB_Job_GetType(j);
		strType = AB_Job_Type2Char(jobType);
		jobState = AB_Job_GetStatus(j);

		if (jobState == AB_Job_StatusFinished ||
		    jobState == AB_Job_StatusPending) {
			//the job was executed without error or it is pending

			/* @todo must we create a copy of the AB_JOB?
			 *
			 * -->	no longer, the abt_jobInfo only administered
			 *	the job and copys all data thats needed
			 */

			/* @todo save the state in the transaction.
			 *
			 * The state should be save within the transaction so
			 * that the history can display the state of the
			 * executed job.
			 *
			 * -->	The state is now saved by abt_history in the
			 *	transaction and recoverd by the parser for
			 *	the abt_jobInfo.
			 *	For details see abt_history::getContext()
			 */
			//AB_Transaction_SetType(t, AB_Transaction_TypeTransaction);
			//AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
			//AB_Transaction_SetStatus(t, AB_Transaction_StatusRevoked);

			abt_jobInfo *jobInfo = new abt_jobInfo(j);
			this->m_history->add(jobInfo);

			this->addlog(tr("<b>Ausführung von '%1' erfolgreich.</b> "
					"Der Auftrag wurde zur Historie hinzugefügt")
				     .arg(abt_conv::JobTypeToQString(jobType)));

			//if wanted, we add a new known recipient
			if (settings->autoAddNewRecipients()) {
				switch(jobType) {
				//we dont add internalTransfer recipients
				//because they are already known by the account
				case AB_Job_TypeCreateDatedTransfer:
				case AB_Job_TypeCreateStandingOrder:
				case AB_Job_TypeDebitNote:
				case AB_Job_TypeEuTransfer:
				case AB_Job_TypeModifyDatedTransfer:
				case AB_Job_TypeModifyStandingOrder:
				case AB_Job_TypeSepaDebitNote:
				case AB_Job_TypeSepaTransfer:
				case AB_Job_TypeTransfer:
					this->addNewRecipient(jobInfo);
					break;
				default:
					//don't try to save a recipient were
					//no one exists.
					break;
				}
			}


			//depending on what jobs are executed we must update
			//the account objects
			AB_ACCOUNT *a = AB_Job_GetAccount(j);
			aqb_AccountInfo *acc = this->m_allAccounts->getAccount(a);
			const AB_TRANSACTION *t;
			abt_datedTransferInfo *dt;
			abt_standingOrderInfo *so;

			switch(jobType) {
			case AB_Job_TypeCreateDatedTransfer:
				//a new dated transfer was created, the data
				//is in the AB_IMEXPORTER_CONTEXT
				break;

			case AB_Job_TypeDeleteDatedTransfer:
				t = AB_JobDeleteDatedTransfer_GetTransaction(j);
				dt = new abt_datedTransferInfo(t);
				acc->removeDatedTransfer(dt);
				delete dt;
                                dt = NULL;
				break;

			case AB_Job_TypeModifyDatedTransfer:
				t = AB_JobModifyDatedTransfer_GetTransaction(j);
				dt = new abt_datedTransferInfo(t);
				acc->removeDatedTransfer(dt);
				delete dt;
                                dt = NULL;
				break;

			case AB_Job_TypeCreateStandingOrder:
				//a new standing order was created, the data
				//is in the AB_IMEXPORTER_CONTEXT
				break;

			case AB_Job_TypeDeleteStandingOrder:
				t = AB_JobDeleteStandingOrder_GetTransaction(j);
				so = new abt_standingOrderInfo(t);
				acc->removeStandingOrder(so);
				delete so;
                                so = NULL;
				break;

			case AB_Job_TypeModifyStandingOrder:
				t = AB_JobModifyStandingOrder_GetTransaction(j);
				so = new abt_standingOrderInfo(t);
				acc->removeStandingOrder(so);
				delete so;
                                so = NULL;
				break;

			case AB_Job_TypeGetDatedTransfers:
				//all dated transfers are updated, the data
				//is in the AB_IMEXPORTER_CONTEXT and parsed
				//by the abt_parser
				acc->clearDatedTransfers();
				break;

			case AB_Job_TypeGetStandingOrders:
				//all standing orders are updated, the data
				//is in the AB_IMEXPORTER_CONTEXT and parsed
				//by the abt_parser
				acc->clearStandingOrders();
				break;

			default:
				break; //nothing to do
			}

			//remove the job from the jobqueue

			// C A U T I O N !
			//If deleteJob(job, free) is called with free = true
			//[default], this will delete the AB_JOB and therefore
			//it would not be accessable anymore!

			abt_jobInfo *ji;
			for (int i=0; i<this->jobqueue->size(); ++i) {
				if (this->jobqueue->at(i)->getJob() == j) {
					//found the job, delete it
					ji = this->jobqueue->at(i);
					this->deleteJob(ji, false);
					break; //no more jobs possible
				}
			}
		} else {
			//there was an error at the execution of the job
			ret = false; //we will return false (error)

			this->addlog(tr("<b><font color=red>"
					"Ausführung von '%1' fehlerhaft."
					"</font></b> "
					"Der Auftrag bleibt im Ausgang erhalten")
				     .arg(abt_conv::JobTypeToQString(jobType)));

			//we remove the job from the AB_JOB_LIST2, therefore
			//it wont be delete by the deletion of the AB_JOB_LIST2.
			AB_Job_List2_Remove(jl, j);

			//we remove it also from the jobqueue
			abt_jobInfo *infojob;
			for (int i=0; i<this->jobqueue->size(); ++i) {
				if (this->jobqueue->at(i)->getJob() == j) {
					//found the abt_jobInfo, delete it
					infojob = this->jobqueue->at(i);
					this->deleteJob(infojob, false);
					break; //no more jobs possible
				}
			}

			//then we create a new abt_jobInfo and add the new
			//abt_jobInfo to the jobqueue
			abt_jobInfo *ji = new abt_jobInfo(j);
			int pos = this->getSupposedJobqueuePos(ji);
			this->jobqueue->insert(pos, ji);
		}


		strList = this->getParsedJobLogs(j); //parse the job logs
		//and add all lines to our log
		foreach(QString line, strList) {
			this->addlog(tr("JobLog: %1").arg(line));
		}

		j = AB_Job_List2Iterator_Next(jli); //next Job in list
	} /* while (j) */

	AB_Job_List2Iterator_free(jli); //free the Joblist iterator

	return ret;
}

//private
/**
 * @brief adds the recipient from the \a jobInfo to the known recipients
 *
 * If the recipient from the \a jobInfo is already known, nothing happen.
 * Otherwise the recipient is added to the list of known recipients.
 *
 * @param jobInfo: the job that maybe contain a new recipient
 */
void abt_job_ctrl::addNewRecipient(const abt_jobInfo *jobInfo)
{
	QString rcp_name = jobInfo->getTransaction()->getRemoteName().at(0);
	QString rcp_kto = jobInfo->getTransaction()->getRemoteAccountNumber();
	QString rcp_blz = jobInfo->getTransaction()->getRemoteBankCode();
	QString rcp_iban = jobInfo->getTransaction()->getRemoteIban();
	QString rcp_bic = jobInfo->getTransaction()->getRemoteBic();
	QString rcp_inst = jobInfo->getTransaction()->getRemoteBankName();

	abt_EmpfaengerInfo *ei = new abt_EmpfaengerInfo(rcp_name, rcp_kto, rcp_blz,
							rcp_iban, rcp_bic, rcp_inst);

	bool recipientAlreadyKnown = false;
	const QList<abt_EmpfaengerInfo*> *rcpList = settings->getKnownRecipients();
	for(int i=0; i<rcpList->size(); ++i) {
		//we need to dereference the pointer so that the == operator
		//of abt_EmpfaengerInfo checks for equal data
		if (*rcpList->at(i) == *ei) {
			recipientAlreadyKnown = true;
			break; //known recipient found, abort search
		}
	}

	//only add the new recipient if we do not already know it
	if (!recipientAlreadyKnown) {
		settings->addKnownRecipient(ei);
	}
}

//private
/**
 * @brief checks if an AB_JOB_TYPE is already in the queue or not
 *
 * Checks if the @a type is already in the @ref jobqueue and also the supplied
 * account @a acc must match.
 *
 * This could be used to check if an update job is already in the queue and
 * must not added twice.
 *
 * @param type: the AB_JOB_TYPE to check for
 * @param acc: the AB_ACCOUNT to be checked
 * @return true: if the AB_JOB_TYPE in already in queue for the AB_ACCOUNT
 * @return false: if the AB_JOB_TYPE is not in queue for the AB_ACCOUNT
 */
bool abt_job_ctrl::isJobTypeInQueue(const AB_JOB_TYPE type,
				    const AB_ACCOUNT *acc) const
{
	for(int i=0; i<this->jobqueue->size(); i++) {
		//jiiq = JobInfoInQueue
		const abt_jobInfo *jiiq = this->jobqueue->at(i);

		if ((jiiq->getAbJobType() == type) &&
		    (jiiq->getAbAccount() == acc)) {
			//jobtype exists and is for the same account
			return true;
		}
	}

	return false;
}

//public
/**
 * @brief checks if the supplied transaction @a t is in the queue or not
 *
 * This function can be used to check if a modification or deletion of a
 * standing order or dated transfer is alreade in the queue for exucution
 * or not.
 *
 * If a transaction for a standing order order dated transfer already exists
 * in the queue a further editing should be prohibited.
 *
 * @param t: abt_transaction to check
 * @return true: if the transaction @a t is already in the jobqueue
 * @return false: if the transcation @a t is not in the jobqueue
 */
bool abt_job_ctrl::isTransactionInQueue(const abt_transaction *t) const
{
	//check all jobs in the jobqueue
	for(int i=0; i<this->jobqueue->size(); i++) {
		//jiiq = JobInfoInQueue
		const abt_jobInfo *jiiq = this->jobqueue->at(i);

		if (jiiq->getTransaction()) { //the job has a transaction
			if (jiiq->getTransaction()->getFiId() == t->getFiId()) {
				//the FiId matches! --> the transaction are equal
				return true;
			}
		}
	}

	return false;
}

//public slot
/**
 * @brief moves the job from @a jobListPos @a updown steps up or down
 *
 * This function should be used with caution!
 *
 * The postions in the @ref jobqueue are there for faultfree execution and
 * parsing of the jobs. If you change the position of a job it is not secured
 * that the execution and parsing is as expected!
 *
 * @param JobListPos: the position of the job to move
 * @param updown: positive for upwards, negative for downwards
 */
void abt_job_ctrl::moveJob(int JobListPos, int updown)
{
	if (JobListPos >= this->jobqueue->size()) {
		qWarning().nospace() << Q_FUNC_INFO
				     << " - JobListPos [" << JobListPos << "]"
				     << " is greater than the jobqueue->size() "
				     << "[" << this->jobqueue->size() << "]";
		return; //abort
	}

	int newPos = JobListPos + updown;
	if ((newPos < 0) || (newPos >= this->jobqueue->size())) {
		qWarning().nospace() << Q_FUNC_INFO
				     << " - new position [" << newPos << "]"
				     << " is not reachable! size() "
				     << "[" << this->jobqueue->size() << "]";
		return; //abort
	}

	this->jobqueue->move(JobListPos, newPos);

	//inform all who wants to know about the change
	emit this->jobQueueListChanged();
}

//public slot
/**
 * @brief removes the job @a jobinfo from the jobqueue
 *
 * @attention If the underlying AB_JOB is already used in a AB_JOB_LIST2 this
 * AB_JOB must not be freed! (@a free must be false! true is the default!)
 *
 * This removes the abt_jobInfo @a jobinfo from the jobqueue and depending of
 * the @a free parameter also frees the underlying AB_JOB.
 *
 * @param jobinfo: the abt_jobInfo to be removed from the jobqueue
 * @param free: defines if the underlying AB_JOB should also be freed or not
 */
void abt_job_ctrl::deleteJob(abt_jobInfo *jobinfo, bool free /*=true*/)
{
	this->jobqueue->removeAll(jobinfo); //remove the jobinfo from the list

	if (free) {
		AB_Job_free(jobinfo->getJob()); //delete the AqBanking AB_JOB
	}

	delete jobinfo; //delete the abt_jobInfo
        jobinfo = NULL;

	//inform all who wants to know about the change
	emit this->jobQueueListChanged();
}

