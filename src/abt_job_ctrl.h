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

#ifndef ABT_JOB_CTRL_H
#define ABT_JOB_CTRL_H

#include <QObject>
#include <QtCore/QHash>

#include "abt_transactions.h"
#include "abt_transactionlimits.h"
#include "aqb_accountinfo.h"
#include "aqb_accounts.h"

#include "abt_jobinfo.h"
#include "abt_history.h"


/** @brief Administration of the jobs that are send to the bank
  *
  * This could be descriped as the "heart" of AB-Transfers, because every
  * action that AB-Transfers is executing or retrieving from the bank is done
  * by this class.
  *
  * The abt_job_ctrl is responsible for the @ref jobqueue and the execution
  * of the jobs in the jobqueue.
  *
  * New jobs to execute can be added by the following public slots:
  * @li @ref addNewSingleTransfer()
  * @li @ref addNewSingleDebitNote()
  * @li @ref addNewEuTransfer()
  * @li @ref addNewInternalTransfer()
  * @li @ref addNewSepaTransfer()
  * @li @ref addCreateDatedTransfer()
  * @li @ref addModifyDatedTransfer()
  * @li @ref addDeleteDatedTransfer()
  * @li @ref addGetDatedTransfers()
  * @li @ref addCreateStandingOrder()
  * @li @ref addModifyStandingOrder()
  * @li @ref addDeleteStandingOrder()
  * @li @ref addGetStandingOrders()
  * @li @ref addGetBalance()
  *
  * Over the public slot @ref execQueuedTransactions() the previously added
  * transactions can be executed and parsed (this is done by the class
  * @ref abt_parser).
  *
  *
  * @todo more detailed description should be added.
  *       Especially the static functions and the sorting of the jobqueue
  *	  should be described.
  *
  * @todo maybe the abt_job_ctrl could be splitted into more classes.
  *       In particular the @ref jobqueue could be an own class wich handles
  *	  all positions by its own. This would reduce the overhead in the
  *	  abt_job_ctrl.
  *
  */

class abt_job_ctrl : public QObject
{
Q_OBJECT

private:
	/** this is the main list where all jobs are queued for execution */
	QList<abt_jobInfo*> *jobqueue;
	aqb_Accounts *m_allAccounts;
	abt_history *m_history;

	void addlog(const QString &str);

	QStringList getParsedJobLogs(const AB_JOB *j) const;

	bool parseImExporterContext(AB_IMEXPORTER_CONTEXT *ctx);

	bool isJobTypeInQueue(const AB_JOB_TYPE type, const AB_ACCOUNT *acc) const;

	bool parseExecutedJobs(AB_JOB_LIST2 *jl);

	void addNewRecipient(const abt_jobInfo *jobInfo);

	int getSupposedJobqueue_FirstPos(const AB_ACCOUNT *acc) const;
	int getSupposedJobqueue_LastPos(int firstPos) const;
	int getSupposedJobqueue_NextTransferPos(int firstTransferPos,
						int lastPos) const;
	int getSupposedJobqueue_NextStandingPos(int firstStandingPos,
						int lastPos) const;
	int getSupposedJobqueue_NextDatedPos(int firstDatedPos,
					     int lastPos) const;
	int getSupposedJobqueuePos(const abt_jobInfo *job) const;

	bool exportImExporterContext(AB_IMEXPORTER_CONTEXT *ctx) const;

public:
	explicit abt_job_ctrl(aqb_Accounts *allAccounts, abt_history *history,
			      QObject *parent = 0);
	~abt_job_ctrl();

	const QList<abt_jobInfo*> *jobqueueList() const { return this->jobqueue; }

	static void createAvailableHashFor(AB_ACCOUNT *a, QHash<AB_JOB_TYPE, bool> *hash);
	static void createTransactionLimitsFor(AB_ACCOUNT *a,
					       QHash<AB_JOB_TYPE, abt_transactionLimits*> *ah);

	bool isTransactionInQueue(const abt_transaction *t) const;

signals:
	void jobNotAvailable(AB_JOB_TYPE type);
	void jobQueueListChanged();
	void jobAdded(const abt_jobInfo *jobInfo);
	void log(const QString &str);
	/** \brief is emitted when all jobs are executed.
	 *
	 * \a successfull is true when everything went fine, otherwise false.
	 */
	void executionFinished(bool successfull);

public slots:
	void addNewSingleTransfer(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addNewSingleDebitNote(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addNewEuTransfer(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addNewInternalTransfer(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addNewSepaTransfer(const aqb_AccountInfo *acc, const abt_transaction *t);

	void addCreateDatedTransfer(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addModifyDatedTransfer(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addDeleteDatedTransfer(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addGetDatedTransfers(const aqb_AccountInfo *acc, bool withoutInfo = false);

	void addCreateStandingOrder(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addModifyStandingOrder(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addDeleteStandingOrder(const aqb_AccountInfo *acc, const abt_transaction *t);
	void addGetStandingOrders(const aqb_AccountInfo *acc, bool withoutInfo = false);

	void addGetBalance(const aqb_AccountInfo *acc, bool withoutInfo = false);

	void execQueuedTransactions();

	void moveJob(int JobListPos, int updown);
	void deleteJob(abt_jobInfo *jobinfo, bool free=true);

};

#endif // ABT_JOB_CTRL_H
