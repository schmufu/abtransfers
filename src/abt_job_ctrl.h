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

class abt_job_info
{
private:
	AB_JOB *job;
	QStringList *jobInfo;

public:
	abt_job_info(AB_JOB *j, const QString &Info);
	~abt_job_info();

	AB_JOB_STATUS getAbJobStatus() const;
	const QString getStatus() const;
	AB_JOB_TYPE getAbJobType() const;
	const QString getType() const;
	const QStringList* getInfo() const;
	AB_JOB *getJob() const;

	const QString getKontoNr() const;
	const QString getBLZ() const;

	static QString createJobInfoString(const abt_transaction *t);
};

class abt_job_ctrl : public QObject
{
Q_OBJECT

private:
	QList<abt_job_info*> *jobqueue;

	void addlog(const QString &str);

	QStringList getParsedJobLogs(const AB_JOB *j) const;

	bool parseImExporterContext(AB_IMEXPORTER_CONTEXT *ctx);

	int parseImExporterContext_Messages(AB_IMEXPORTER_CONTEXT *ctx);
	int parseImExporterContext_Securitys(AB_IMEXPORTER_CONTEXT *ctx);
	int parseImExporterAccountInfo_Status(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_DatedTransfers(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_NotedTransactions(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_StandingOrders(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_Transfers(AB_IMEXPORTER_ACCOUNTINFO *ai);
	int parseImExporterAccountInfo_Transactions(AB_IMEXPORTER_ACCOUNTINFO *ai);


	//wenn Status Successfull Transaction des Jobs als Dated Speichern
	int parseJobTypeCreateDatedTransfer(const AB_JOB *j);
	//wenn Successfull Ctx parsen (dort ist der gelöschte DT drin)
	int parseJobTypeDeleteDatedTransfer(const AB_JOB *j);
	//wenn Successfull Transaction des Jobs löschen
	//wurde die FiId der Transaction im Job geändert? wenn ja, wie kommen wir an die alte?
	//die neue Transaction muss gespeichert werden
	int parseJobTypeModifyDatedTransfer(const AB_JOB *j);
	//wenn Successfull Ctx parsen (dort sind die DTs drin)
	int parseJobTypeGetDatedTransfers(const AB_JOB *j);

	//für StandingOrders siehe Kommentare bei DatedTransfers
	int parseJobTypeCreateStandingOrder(const AB_JOB *j);
	int parseJobTypeDeleteStandingOrder(const AB_JOB *j);
	int parseJobTypeModifyStandingOrder(const AB_JOB *j);
	int parseJobTypeGetStandingOrders(const AB_JOB *j);

	//hier einfach den Ctx parsen? Prüfung auf Erfolg sollte auch gemacht werden!
	//was machen wir bei Fehler? Job sollte in der jobQueue bleiben!
	int parseJobTypeTransfer(const AB_JOB *j);
	int parseJobTypeEuTransfer(const AB_JOB *j);
	int parseJobTypeSepaTransfer(const AB_JOB *j);
	int parseJobTypeInternalTransfer(const AB_JOB *j);
	int parseJobTypeDebitNote(const AB_JOB *j);
	int parseJobTypeSepaDebitNote(const AB_JOB *j);
	int parseJobTypeUnknown(const AB_JOB *j);

	int parseJobTypeGetBalance(const AB_JOB *j);
	int parseJobTypeGetTransactions(const AB_JOB *j);

	int parseJobTypeLoadCellPhone(const AB_JOB *j);



	bool checkJobStatus(AB_JOB_LIST2 *jl);

	bool isJobTypeInQueue(const AB_JOB_TYPE type, const abt_job_info *ji) const;

	//! Prüft die Übergebene Ausgeführte JobListe auf Fehler und parst deren Context
	bool parseExecutedJobListAndContext(AB_JOB_LIST2 *jobList, AB_IMEXPORTER_CONTEXT *ctx);


public:
	explicit abt_job_ctrl(QObject *parent = 0);
	~abt_job_ctrl();

	const QList<abt_job_info*> *jobqueueList() const { return this->jobqueue; }

	//! Static function for creating every TransactionLimit for an Account
	static void createTransactionLimitsFor(AB_ACCOUNT *a,
					       QHash<AB_JOB_TYPE, abt_transactionLimits*> *ah);

signals:
	void jobNotAvailable(AB_JOB_TYPE type);
	void jobQueueListChanged();
	void jobAdded(const abt_job_info *jobInfo);
	void log(const QString &str);
	//! wird gesendet wenn das parsen der Daueraufträge abgeschlossen ist
	void standingOrdersParsed();
	//! wird gesendet wenn das parsen der Terminüberweisungen abgeschlossen ist
	void datedTransfersParsed();

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

	void execQueuedTransactions();

	void moveJob(int JobListPos, int updown);
	void deleteJob(int JobListPos);

};

Q_DECLARE_METATYPE(abt_job_info*);
Q_DECLARE_METATYPE(const abt_job_info*);
//qRegisterMetaType<const abt_job_info*>("const abt_job_info*");


#endif // ABT_JOB_CTRL_H
