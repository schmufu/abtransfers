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


/** \brief Verwaltung von Aufträgen für die Bank
  *
  * abt_job_ctrl dient der Verwaltung und der Abarbeitung von Aufträgen für
  * die Bank.
  *
  * Dieser Klasse können über die add... Funktionen neue Aufträge zugeteilt
  * werden. Über execQueuedTransactions() können diese Aufträge dann zur Bank
  * gesendet werden und die Anworten werden ausgewertet (friend class \ref
  * abt_parser)
  *
  */

class abt_job_ctrl : public QObject
{
Q_OBJECT

private:
	QList<abt_jobInfo*> *jobqueue;
	aqb_Accounts *m_allAccounts;
	abt_history *m_history;

	void addlog(const QString &str);

	QStringList getParsedJobLogs(const AB_JOB *j) const;

	bool parseImExporterContext(AB_IMEXPORTER_CONTEXT *ctx);

	bool isJobTypeInQueue(const AB_JOB_TYPE type, const abt_jobInfo *ji) const;

	//! Prüft die Übergebene Ausgeführte JobListe auf Fehler und parst deren Context
	bool parseExecutedJobListAndContext(AB_JOB_LIST2 *jobList, AB_IMEXPORTER_CONTEXT *ctx);
	//! Prüft die übergebene ausgeführte JobListe \a jl auf Fehler
	bool parseExecutedJobs(AB_JOB_LIST2 *jl);

	//! adds the recipient from the \a jobInfo to the known recipients
	void addNewRecipient(const abt_jobInfo *jobInfo);

public:
	explicit abt_job_ctrl(aqb_Accounts *allAccounts, abt_history *history,
			      QObject *parent = 0);
	~abt_job_ctrl();

	const QList<abt_jobInfo*> *jobqueueList() const { return this->jobqueue; }

	static void createAvailableHashFor(AB_ACCOUNT *a, QHash<AB_JOB_TYPE, bool> *hash);
	//! Static function for creating every TransactionLimit for an Account
	static void createTransactionLimitsFor(AB_ACCOUNT *a,
					       QHash<AB_JOB_TYPE, abt_transactionLimits*> *ah);

	//! zum Prüfen ob ein bestimmter SO oder DT schon in der jobListe vorhanden ist
	bool isTransactionInQueue(const abt_transaction *t) const;

signals:
	void jobNotAvailable(AB_JOB_TYPE type);
	void jobQueueListChanged();
	void jobAdded(const abt_jobInfo *jobInfo);
	void log(const QString &str);

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
	/** \brief löscht den AB_JOB an der Listenposition \a JobListPos */
	void deleteJob(abt_jobInfo *jobinfo, bool free=true);

};

#endif // ABT_JOB_CTRL_H
