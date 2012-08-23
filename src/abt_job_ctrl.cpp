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

	qDebug() << Q_FUNC_INFO << "deleted";
}

//static public
/**
 * Speichert in dem Übergebenen QHash \a hash zu jeden AB_JOB_TYPE ob dieser
 * von der Bank unterstützt wird oder nicht.
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


/**
 * Erstellt alle TransactionLimits die für einen Account verfügbar sind und
 * speichert diese in dem übergebenen QHash \a ah
 */
//static public
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

	/** \todo Wo bekommen wir die Limits für einen EU-Transfer her? */
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
void abt_job_ctrl::addlog(const QString &str)
{
	static QString time;
	time = QTime::currentTime().toString("HH:mm:ss.zzz");
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
	if (gwenStrList) { //nur wenn auch ein log existiert
		strList = abt_conv::GwenStringListToQStringList(gwenStrList);
		GWEN_StringList_free(gwenStrList); // \done macht jetzt abt_conv selbst oder?
					    //NEIN! QStringlistToGwenStringList löscht sich selbst!
					    //GwenToQ macht dies nicht! (und das aus guten Grund!)
	}

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
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobSingleTransfer_SetTransaction(job, t->getAB_Transaction());

	abt_jobInfo *ji = new abt_jobInfo(job);

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
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeDebitNote);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobSingleDebitNote_SetTransaction(job, t->getAB_Transaction());

	//Create Info for SingleDebitNote
	abt_jobInfo *ji = new abt_jobInfo(job);

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
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeEuTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobEuTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info for EuTransfer
	abt_jobInfo *ji = new abt_jobInfo(job);


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
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeInternalTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobInternalTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info for Internal Transfer
	abt_jobInfo *ji = new abt_jobInfo(job);


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
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeSepaTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobSepaTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info for SingleTransfer
	abt_jobInfo *ji = new abt_jobInfo(job);


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
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeCreateDatedTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobCreateDatedTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info for NewDatedTransfer
	abt_jobInfo *ji = new abt_jobInfo(job);


	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
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

//SLOT
void abt_job_ctrl::addModifyDatedTransfer(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobModifyDatedTransfer_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeModifyDatedTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobModifyDatedTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	abt_jobInfo *ji = new abt_jobInfo(job);

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
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeDeleteDatedTransfer);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobDeleteDatedTransfer_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	abt_jobInfo *ji = new abt_jobInfo(job);


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

	if (acc == NULL) {
		//Job is not available!
		qWarning() << Q_FUNC_INFO << "Job AB_Job_TypeGetDatedTransfers is not available (no valid account [NULL])";
		emit jobNotAvailable(AB_Job_TypeGetDatedTransfers);
		return; //Abbruch
	}

	AB_JOB *job = AB_JobGetDatedTransfers_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeGetDatedTransfers);
		return; //Abbruch
	}

	//Create Info
	abt_jobInfo *ji = new abt_jobInfo(job);


	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	if (!withoutInfo) { //Info nur verschicken wenn gewollt
		emit this->jobAdded(ji);
	}
	emit this->jobQueueListChanged();

	//Das zuständige AccountInfo Object darüber informieren wenn wir
	//mit dem parsen fertig sind (damit dies die DTs neu laden kann)
	//--> wird jetzt direkt beim parsen erledigt!
//	connect(this, SIGNAL(datedTransfersParsed()),
//		acc, SLOT(loadKnownDatedTransfers()));
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
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeCreateStandingOrder);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobCreateStandingOrder_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	abt_jobInfo *ji = new abt_jobInfo(job);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
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

//SLOT
void abt_job_ctrl::addModifyStandingOrder(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobModifyStandingOrder_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeModifyStandingOrder);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobModifyStandingOrder_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	abt_jobInfo *ji = new abt_jobInfo(job);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();

	if (!this->isJobTypeInQueue(AB_Job_TypeGetStandingOrders, ji)) {
		//nach dem ändern muss eine Aktualisierung stattfinden.
		this->addGetStandingOrders(acc, true);
	}

}

//SLOT
void abt_job_ctrl::addDeleteStandingOrder(const aqb_AccountInfo *acc, const abt_transaction *t)
{
	int rv;

	AB_JOB *job = AB_JobDeleteStandingOrder_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeDeleteStandingOrder);
		return; //Abbruch
	}

	//add transaction to the job
	rv = AB_JobDeleteStandingOrder_SetTransaction(job, t->getAB_Transaction());

	//Create Info
	abt_jobInfo *ji = new abt_jobInfo(job);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	emit this->jobAdded(ji);
	emit this->jobQueueListChanged();
}

//SLOT
void abt_job_ctrl::addGetStandingOrders(const aqb_AccountInfo *acc, bool withoutInfo /*=false*/)
{
	int rv;

	if (acc == NULL) {
		//Job is not available!
		qWarning() << Q_FUNC_INFO << "Job AB_Job_TypeGetStandingOrders is not available (no valid account [NULL])";
		emit jobNotAvailable(AB_Job_TypeGetStandingOrders);
		return; //Abbruch
	}

	AB_JOB *job = AB_JobGetStandingOrders_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeGetStandingOrders);
		return; //Abbruch
	}

	//Create Info
	abt_jobInfo *ji = new abt_jobInfo(job);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	if (!withoutInfo) { //Info nur verschicken wenn gewollt
		emit this->jobAdded(ji);
	}
	emit this->jobQueueListChanged();

	//Das zuständige AccountInfo Object darüber informieren wenn wir
	//mit dem parsen fertig sind (damit dies die SOs neu laden kann)
	//--> wird jetzt direkt beim parsen erledigt!
//	connect(this, SIGNAL(standingOrdersParsed()),
//		acc, SLOT(loadKnownStandingOrders()));

}


//SLOT
void abt_job_ctrl::addGetBalance(const aqb_AccountInfo *acc, bool withoutInfo /*=false*/)
{
	int rv;

	if (acc == NULL) {
		//Job is not available!
		qWarning() << Q_FUNC_INFO << "Job AB_Job_TypeGetBalance is not available (no valid account [NULL])";
		emit jobNotAvailable(AB_Job_TypeGetBalance);
		return; //Abbruch
	}

	AB_JOB *job = AB_JobGetBalance_new(acc->get_AB_ACCOUNT());

	rv = AB_Job_CheckAvailability(job);

	if (rv) {
		//Job is not available!
		qWarning() << Q_FUNC_INFO << "Job is not available (" << rv << ")";
		emit jobNotAvailable(AB_Job_TypeGetBalance);
		return; //Abbruch
	}

	//Create Info
	abt_jobInfo *ji = new abt_jobInfo(job);

	//job in die Ausführung einreihen. (Beim Ausführen wird daraus die
	//AB_JOB_LIST gebaut)
	this->jobqueue->append(ji);
	if (!withoutInfo) { //Info nur verschicken wenn gewollt
		emit this->jobAdded(ji);
	}
	emit this->jobQueueListChanged();

}



/******** Ausführung *******/


//SLOT
void abt_job_ctrl::execQueuedTransactions()
{
	int rv;
	AB_JOB_LIST2 *jl;
	AB_IMEXPORTER_CONTEXT *ctx;

	this->addlog(tr("Erstelle Job-Liste."));

	jl = AB_Job_List2_new();

	//Alle jobs in der reihenfolge wie in dem jobqueue einreihen
	for (int i=0; i<this->jobqueue->size(); ++i) {
		AB_Job_List2_PushBack(jl, this->jobqueue->at(i)->getJob());
	}

	this->addlog(tr("%1 Aufträge in die Jobliste übernommen").arg(this->jobqueue->count()));


	ctx = AB_ImExporterContext_new();

	this->addlog(tr("Führe Job-Liste aus."));

	rv = AB_Banking_ExecuteJobs(banking->getAqBanking(), jl, ctx);
	if (rv) {
		qWarning() << Q_FUNC_INFO << "Error on execQueuedTransactions ("
				<< rv << ")";
		//cleanup
		this->addlog(tr("***********************************************"
				"** E R R O R                                 **"
				"** Fehler bei AB_Banking_ExecuteJobs().      **"
				"** return value = %1                         **"
				"**                                           **"
				"** Es wird abgebrochen und keine weitere     **"
				"** Bearbeitung/Auswertung durchgeführt!      **"
				"***********************************************").arg(rv));

		/** \todo Hier müssen alle jobs des jobqueue gelöscht und neu
		  *       erstellt wieder hinzugefügt werden!
		  */
		AB_Job_List2_FreeAll(jl); // <-- dies löscht auch ALLE Jobs!
		AB_ImExporterContext_Clear(ctx);
		AB_ImExporterContext_free(ctx);
		return;

		/** \todo Was machen wir mit den Jobs in dem jobqueue? */

	}


	bool successfull = this->parseExecutedJobs(jl);

	//Die zurückgelieferten Informationen auswerten und in den
	//entsprechenden Accounts setzen
	abt_parser::parse_ctx(ctx, this->m_allAccounts);

	this->addlog(tr("Alle Jobs übertragen und Antworten ausgewertet"));

	//wir geben die JobList wieder frei. Dies löscht auch ALLE AB_JOBs die
	//darin noch enthalten sind!
	//Die Objekte an denen ein AB_JOB übergeben wurde dürfen mit diesem
	//nicht mehr arbeiten, sondern nur mit Kopien der Daten!
	AB_Job_List2_FreeAll(jl); //löscht alle Jobs und die Liste
	AB_ImExporterContext_Clear(ctx);
	AB_ImExporterContext_free(ctx);

	//JobListe hat sich definitiv geändert, jeden informieren der es benötigt
	emit this->jobQueueListChanged();

	//Wenn die Ausführung einzelner Aufträge fehlerhaft war eine Warnung ausgeben.
	if (!successfull) {
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

/**
  * überprüft den Status der ausgeführten Jobs und verschiebt diese, wenn
  * erfolgreich, in die History-Liste.
  * Wenn ein Fehler aufgetreten ist wird der fehlerhafte Job aus der übergebenen
  * AB_JOB_LIST2 \a jl entfernt und ist somit weiterhin verwendbar.
  *
  * Wenn alle Jobs erfolgreich ausgeführt wurden wird true zurückgegeben,
  * ansonsten false.
  */
//private
bool abt_job_ctrl::parseExecutedJobs(AB_JOB_LIST2 *jl)
{
	AB_JOB *j = NULL;
	AB_JOB_STATUS jobState;
	AB_JOB_TYPE jobType;
	QString strType;
	QStringList strList;
	qDebug() << Q_FUNC_INFO << "started";
	bool ret = true; //default - Alles fehlerfrei

	//Die Jobs wurden zur Bank übertragen und evt. durch das Backend geändert
	//jetzt alle Jobs durchgehen und entsprechend des Status parsen

	/** \todo Die Verwendung der AB_JOB_LIST2_ITERATOR und deren Funktionen
		  muss nochmal kontrolliert und nachgelesen werden!
	*/

	AB_JOB_LIST2_ITERATOR *jli;
	//jli = AB_Job_List2Iterator_new(jl); //Wird das benötigt???
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
			//Job wurde erfolgreich ausgeführt oder von der Bank
			//zur Ausführung entgegen genommen

			// \todo Müssen wir hier eine Kopie des AB_JOB erstellen?
			// -->	nicht mehr, da abt_jobInfo den Job nurnoch
			//	verwaltet und benötigte Daten des Jobs kopiert.


			/* \todo Status in der ausgeführten Transaction setzen.
			 * Der Status sollte innherhalb der Transaction gespeichert
			 * werden. Damit die History auch anzeigt wie der Status
			 * des Auftrages war.
			 *
			 * -->	Der Status wird jetzt in durch abt_history beim
			 *	speichern in der Transaction gesetzt und durch
			 *	den parser wieder in abt_jobInfo gesetzt.
			 *	Details siehe abt_history::getContext()
			 */
			//AB_Transaction_SetType(t, AB_Transaction_TypeTransaction);
			//AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
			//AB_Transaction_SetStatus(t, AB_Transaction_StatusRevoked);


			abt_jobInfo *jobInfo = new abt_jobInfo(j);
			this->m_history->add(jobInfo);

			this->addlog(tr("<b>Ausführung von '%1' erfolgreich.</b> "
					"Der Auftrag wurde zur Historie hinzugefügt").arg(
							abt_conv::JobTypeToQString(jobType)));

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


			//Je nachdem was gemacht wurde müssen evt. noch die
			//account-Objekte aktualisiert werden.
			AB_ACCOUNT *a = AB_Job_GetAccount(j);
			aqb_AccountInfo *acc = this->m_allAccounts->getAccount(a);
			const AB_TRANSACTION *t;
			abt_datedTransferInfo *dt;
			abt_standingOrderInfo *so;

			switch(jobType) {
			case AB_Job_TypeCreateDatedTransfer:
				//eine neue terminierte Überweisung wurde
				//erstellt im ctx befinden sich die Daten
				break;

			case AB_Job_TypeDeleteDatedTransfer:
				t = AB_JobDeleteDatedTransfer_GetTransaction(j);
				dt = new abt_datedTransferInfo(t);
				acc->removeDatedTransfer(dt);
				delete dt;
				break;

			case AB_Job_TypeModifyDatedTransfer:
				t = AB_JobModifyDatedTransfer_GetTransaction(j);
				dt = new abt_datedTransferInfo(t);
				acc->removeDatedTransfer(dt);
				delete dt;
				break;

			case AB_Job_TypeCreateStandingOrder:
				//ein neuer Dauerauftrag wurde erstellt im
				//ctx befinden sich die Daten
				break;

			case AB_Job_TypeDeleteStandingOrder:
				t = AB_JobDeleteStandingOrder_GetTransaction(j);
				so = new abt_standingOrderInfo(t);
				acc->removeStandingOrder(so);
				delete so;
				break;

			case AB_Job_TypeModifyStandingOrder:
				t = AB_JobModifyStandingOrder_GetTransaction(j);
				so = new abt_standingOrderInfo(t);
				acc->removeStandingOrder(so);
				delete so;
				break;

			case AB_Job_TypeGetDatedTransfers:
				//Alle DatedTransfers wurden aktualisiert
				acc->clearDatedTransfers();
				break;

			case AB_Job_TypeGetStandingOrders:
				//Alle StandingOrders wurden aktualisiert
				acc->clearStandingOrders();
				break;

			default:
				break; //nichts zu tun
			}

			//Job auch aus dem jobqueue entfernen

			//ACHTUNG!
			//Wenn bei deleteJob(job, free) free mit true [default]
			//übergeben wird, löscht dies den AB_JOB! Hiernach könnte
			//dann auf den AB_JOB [j] nicht mehr zugegriffen werden!

            abt_jobInfo *infojob;
			for(int i=0; i<this->jobqueue->size(); ++i) {
				if (this->jobqueue->at(i)->getJob() == j) {
					//JobPos, gefunden, diesen löschen
                    infojob = this->jobqueue->at(i); //UserRole enthält die Adresse
                    this->deleteJob( infojob , false);
					break; //kein weiterer Job möglich
				}
			}
		} else {
			//Es ist ein Fehler beim Ausführen des Jobs aufgetreten!
			ret = false; //wir werden false zurückgeben

			this->addlog(tr("<b><font color=red>Ausführung von '%1' fehlerhaft.</font></b> "
					"Der Auftrag bleibt im Ausgang erhalten").arg(
							abt_conv::JobTypeToQString(jobType)));

			//Job aus der AB_JOB_LIST2 entfernen, damit er nicht
			//durch das löschen der AB_JOB_LIST2 auch gelöscht wird.
			AB_Job_List2_Remove(jl, j);

			//den Job erstmal aus dem jobqueue entfernen
            abt_jobInfo *infojob;
			for(int i=0; i<this->jobqueue->size(); ++i) {
				if (this->jobqueue->at(i)->getJob() == j) {
					//JobPos, gefunden, diesen löschen
                    infojob = this->jobqueue->at(i); //UserRole enthält die Adresse
                    this->deleteJob( infojob , false);
					break; //kein weiterer Job möglich
				}
			}

			//dann für den Job ein neues abt_jobInfo erstellen
			//und dies dem jobqueue wieder hinzufügen
			this->jobqueue->append(new abt_jobInfo(j));
		}


		//Die Logs des Backends parsen
		strList = this->getParsedJobLogs(j);
		//Alle Strings der StringListe des jobLogs zu unserem Log hinzufügen
		foreach(QString line, strList) {
			this->addlog(tr("JobLog: %1").arg(line));
		}

		j = AB_Job_List2Iterator_Next(jli); //next Job in list
	} /* while (j) */

	AB_Job_List2Iterator_free(jli); //Joblist iterator wieder freigeben

	return ret;
}

/**
 * If the recipient from the \a jobInfo is already known, nothing happen.
 * Otherwise the recipient is added to the list of known recipients.
 */
//private
void abt_job_ctrl::addNewRecipient(const abt_jobInfo *jobInfo)
{
	QString rcp_name = jobInfo->getTransaction()->getRemoteName().at(0);
	QString rcp_kto = jobInfo->getTransaction()->getRemoteAccountNumber();
	QString rcp_blz = jobInfo->getTransaction()->getRemoteBankCode();
	abt_EmpfaengerInfo *ei = new abt_EmpfaengerInfo(rcp_name, rcp_kto, rcp_blz);

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



/** prüft ob ein job vom typ \a type in der queuelist vorhanden ist und ob
  dieser Job auch für dasselbe Konto wie der Job \a ji ist.

  Kann genutzt werden um zu überprüfen ob bereits ein Aktualisierungs-Job
  in der queuelist vorhanden ist oder nicht. */
bool abt_job_ctrl::isJobTypeInQueue(const AB_JOB_TYPE type, const abt_jobInfo *ji) const
{
	//Kontrollieren ob ein AktualisierungsAuftrag bereits vorhanden ist

	for(int i=0; i<this->jobqueue->size(); i++) {
		//jiiq = JobInfoInQueue
		const abt_jobInfo *jiiq = this->jobqueue->at(i);

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

/**
  * mit dieser Funktion kann vor der Bearbeitung eines Dauerauftrags bzw. einer
  * Terminüberweisung überprüft werden ob diese bereits im Ausgang vorhanden
  * ist. Wenn dies der Fall ist sollte die weitere Bearbeitung der Daten
  * unterlassen werden!
  */
bool abt_job_ctrl::isTransactionInQueue(const abt_transaction *t) const
{
	//Alle Jobs im Queue durchgehen und sobald die FiId
	for(int i=0; i<this->jobqueue->size(); i++) {
		//jiiq = JobInfoInQueue
		const abt_jobInfo *jiiq = this->jobqueue->at(i);

		if (jiiq->getTransaction()) {
			//Transaction im Job vorhanden
			if (jiiq->getTransaction()->getFiId() == t->getFiId()) {
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
		qWarning().nospace() << Q_FUNC_INFO
				     << " - JobListPos [" << JobListPos << "]"
				     << " is greater than the jobqueue->size() "
				     << "[" << this->jobqueue->size() << "]";
		return; //Abbruch
	}

	int newPos = JobListPos + updown;
	if ((newPos < 0) || (newPos >= this->jobqueue->size())) {
		qWarning().nospace() << Q_FUNC_INFO
				     << " - new position [" << newPos << "]"
				     << " is not reachable! size() "
				     << "[" << this->jobqueue->size() << "]";
		return; //Abbruch
	}

	this->jobqueue->move(JobListPos, newPos);

	//Alle die es wollen darüber Informieren das sich die Liste geändert hat
	emit this->jobQueueListChanged();
}

/**
  * Wenn dieser Job bereits in der JobList von AqBanking (AB_JOB_LIST2)
  * enthalten ist darf er NICHT freigegeben werden. Somit muss in diesem fall
  * \a free = false übergeben werden
  */
//public slot
void abt_job_ctrl::deleteJob( abt_jobInfo *jobinfo, bool free /*=true*/)
{
    this->jobqueue->removeAll( jobinfo ); //aus der Liste enfernen

    if (free) {
		AB_Job_free(jobinfo->getJob()); //aq_banking Job löschen
	}

	delete jobinfo; // und jobinfo löschen

	//Alle die es wollen darüber Informieren das sich die Liste geändert hat
	emit this->jobQueueListChanged();
}

