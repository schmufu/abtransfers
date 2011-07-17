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

#include "abt_transactions.h"
#include "aqb_accountinfo.h"

class abt_job_info
{
private:
	AB_JOB *job;
	QStringList *jobInfo;

public:
	abt_job_info(AB_JOB *j, const QString &Info);
	~abt_job_info();

	const QString getStatus() const;
	const QString getType() const;
	const QStringList* getInfo() const;
	AB_JOB *getJob() const;

};

class abt_job_ctrl : public QObject
{
Q_OBJECT

private:
	QList<abt_job_info*> *jobqueue;

	void addlog(const QString &str);

	bool parseImExporterContext(AB_IMEXPORTER_CONTEXT *ctx);

	int parseImExporterContext_Messages(AB_IMEXPORTER_CONTEXT *ctx);
	int parseImExporterContext_Securitys(AB_IMEXPORTER_CONTEXT *ctx);
	int parseImExporterAccountInfo_Status(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_DatedTransfers(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_NotedTransactions(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_StandingOrders(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_Transfers(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_Transactions(AB_IMEXPORTER_ACCOUNTINFO *ai);

	bool checkJobStatus(AB_JOB_LIST2 *jl);

public:
	explicit abt_job_ctrl(QObject *parent = 0);
	~abt_job_ctrl();

	const QList<abt_job_info*> *jobqueueList() const { return this->jobqueue; }


signals:
	void jobNotAvailable(AB_JOB_TYPE type);
	void jobQueueListChanged();
	void log(const QString &str);

public slots:
	void addNewSingleTransfer(aqb_AccountInfo *acc, const trans_SingleTransfer *t);
	void addNewSingleDebitNote(aqb_AccountInfo *acc, const trans_SingleDebitNote *t);
	void addNewEuTransfer(aqb_AccountInfo *acc, const trans_EuTransfer *t);
	void addNewInternalTransfer(aqb_AccountInfo *acc, const trans_InternalTransfer *t);
	void addNewSepaTransfer(aqb_AccountInfo *acc, const trans_SepaTransfer *t);

	void addCreateDatedTransfer(aqb_AccountInfo *acc, const trans_DatedTransfer *t);
	void addModifyDatedTransfer(aqb_AccountInfo *acc, const trans_DatedTransfer *t);
	void addDeleteDatedTransfer(aqb_AccountInfo *acc, const trans_DatedTransfer *t);
	void addGetDatedTransfers(aqb_AccountInfo *acc);

	void addCreateStandingOrder(aqb_AccountInfo *acc, const trans_StandingOrder *t);
	void addModifyStandingOrder(aqb_AccountInfo *acc, const trans_StandingOrder *t);
	void addDeleteStandingOrder(aqb_AccountInfo *acc, const trans_StandingOrder *t);
	void addGetStandingOrders(aqb_AccountInfo *acc);

	void execQueuedTransactions();
};

#endif // ABT_JOB_CTRL_H
