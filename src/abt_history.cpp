/******************************************************************************
 * Copyright (C) 2012-2013 Patrick Wacker
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
 *	This class is used to manage the executed jobs.
 *	A widget can use the data of this class and display the jobs from
 *	the past executions.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#include "abt_history.h"

#include <QtCore/QDebug>


abt_history::abt_history(QObject *parent) :
	QObject(parent)
{
	this->m_historyList = new QList<abt_jobInfo*>;
}

abt_history::~abt_history()
{	
	//delete all objects and the list itself
	this->clearAll();
	delete this->m_historyList;
}

//public
/**
 * the @a job is added to the history only if the kind of job is supported by
 * the history (all transfer types but no deletion or status updates).
 *
 * After adding the job to the list, the list is resorted in the descending
 * order of the time of execution.
 *
 * At least the signal historyListChanged() is emitted.
 */
void abt_history::add(abt_jobInfo *job)
{
	if (!job) {
		return; //no NULL objects
	}

	//only add job to history if wanted
	switch (job->getAbJobType()) {
	case AB_Job_TypeCreateDatedTransfer:
	case AB_Job_TypeCreateStandingOrder:
	case AB_Job_TypeDebitNote:
	case AB_Job_TypeEuTransfer:
	case AB_Job_TypeInternalTransfer:
	case AB_Job_TypeLoadCellPhone:
	case AB_Job_TypeModifyDatedTransfer:
	case AB_Job_TypeModifyStandingOrder:
	case AB_Job_TypeSepaDebitNote:
	case AB_Job_TypeSepaTransfer:
	case AB_Job_TypeTransfer:
		this->m_historyList->prepend(job);
		this->sortListByTimestamp();
		emit this->historyListChanged(this);
		break;
	default:
		qDebug() << Q_FUNC_INFO << "not supported in the history."
			 << "JobType:" << job->getType();
		break;
	}



}

//public
/**
 * the @a job is removed from the list.
 *
 * After a job is removed the list is sorted by the timestamp of execution
 * and the signal historyListChanged() is emitted.
 *
 * @returns true if the job was removed.
 * @returns false if the job could not be removed or isnt in the list.
 */
bool abt_history::remove(abt_jobInfo *job)
{
	if (this->m_historyList->removeOne(job)) {
		//job removed, also delete it
		delete job;
		this->sortListByTimestamp();
		emit this->historyListChanged(this);
		return true;
	}

	return false;
}

//public
bool abt_history::remove(int pos)
{
	if (pos >= this->m_historyList->size()) return false;

	abt_jobInfo *j = this->m_historyList->takeAt(pos);
	delete j;
	this->sortListByTimestamp();
	emit this->historyListChanged(this);
	return true;
}

//public
/**
 * After the list is empty the signal historyListChanged() is emitted.
 */
void abt_history::clearAll()
{
	//Alle Objecte löschen
	while (this->m_historyList->size() != 0) {
		abt_jobInfo *j = this->m_historyList->takeFirst();
		delete j;
	}
	emit this->historyListChanged(this);
}

//public
/**
 * Creates a new AB_IMEXPORTER_CONTEXT which could be exported (saved) by the
 * corresponding aqbanking function.
 *
 * The AB_JOB_TYPE and AB_JOB_STATUS is added to the category-fiels of the
 * AB_TRANSACTION which is then added to the returned AB_IMEXPORTER_CONTEXT.
 * At loading from the history-file this information is read from the category
 * and removed (see @ref abt_parser::parse_ctx() ).
 *
 * The returned AB_IMEXPORTER_CONTEXT @b must be deleted by the calling function!
 */
AB_IMEXPORTER_CONTEXT *abt_history::getContext() const
{
	//we create our own account for the history
	AB_IMEXPORTER_ACCOUNTINFO *iea = AB_ImExporterAccountInfo_new();

	AB_ImExporterAccountInfo_SetAccountNumber(iea, "0000000000");
	AB_ImExporterAccountInfo_SetAccountName(iea, "AB-Transfers HistoryAccount");
	AB_ImExporterAccountInfo_SetBankCode(iea, "00000000");
	AB_ImExporterAccountInfo_SetBankName(iea, "AB-Transfers History");
	AB_ImExporterAccountInfo_SetOwner(iea, "AB-Transfers");
	AB_ImExporterAccountInfo_SetDescription(iea, "History Objects from AB-Transfers");
	AB_ImExporterAccountInfo_SetCurrency(iea, "EUR");

	Q_FOREACH(const abt_jobInfo* job, *this->m_historyList) {

		//To store the state and type of the job in the transaction we
		//use the category stringlist of the transaction.
		//We append the state and the type to the list.
		//This information is read and removed by the history-parser at
		//the time where the transaction is loaded from the history file.

		AB_TRANSACTION *t = NULL;


		switch(job->getAbJobType()) {
		case AB_Job_TypeCreateDatedTransfer:
		case AB_Job_TypeModifyDatedTransfer:
		case AB_Job_TypeDeleteDatedTransfer:
			//append a dated transfer to the history
			t = AB_Transaction_dup(job->getTransaction()->getAB_Transaction());
			AB_Transaction_AddCategory(t, QString("JobStatus: %1").arg(job->getAbJobStatus()).toUtf8(), 1);
			AB_Transaction_AddCategory(t, QString("JobType: %1").arg(job->getAbJobType()).toUtf8(), 1);
			AB_ImExporterAccountInfo_AddDatedTransfer(iea, t);
			break;
		case AB_Job_TypeCreateStandingOrder:
		case AB_Job_TypeModifyStandingOrder:
		case AB_Job_TypeDeleteStandingOrder:
			//append a standing order to the history
			t = AB_Transaction_dup(job->getTransaction()->getAB_Transaction());
			AB_Transaction_AddCategory(t, QString("JobStatus: %1").arg(job->getAbJobStatus()).toUtf8(), 1);
			AB_Transaction_AddCategory(t, QString("JobType: %1").arg(job->getAbJobType()).toUtf8(), 1);
			AB_ImExporterAccountInfo_AddStandingOrder(iea, t);
			break;
		case AB_Job_TypeTransfer:
		case AB_Job_TypeEuTransfer:
		case AB_Job_TypeInternalTransfer:
		case AB_Job_TypeLoadCellPhone:
		case AB_Job_TypeSepaDebitNote:
		case AB_Job_TypeDebitNote:
		case AB_Job_TypeSepaTransfer:
			//append a transfer to the history
			t = AB_Transaction_dup(job->getTransaction()->getAB_Transaction());
			AB_Transaction_AddCategory(t, QString("JobStatus: %1").arg(job->getAbJobStatus()).toUtf8(), 1);
			AB_Transaction_AddCategory(t, QString("JobType: %1").arg(job->getAbJobType()).toUtf8(), 1);
			AB_ImExporterAccountInfo_AddTransfer(iea, t);
			break;

		default: //other objects not possible, by now
			break;
		}
	}

	//now all history objects are related to the corresponding account
	AB_IMEXPORTER_CONTEXT *iec = AB_ImExporterContext_new();
	AB_ImExporterContext_AddAccountInfo(iec, iea);

	return iec;
}

//private
/**
 * In the IdForApplication the unix timestamp of the creation is saved.
 * This function sorts all items by this timestamp in an descending order (default)
 **/
void abt_history::sortListByTimestamp(bool descending /* = true */)
{
	QList<abt_jobInfo*> *hl = this->m_historyList;	//history list

	bool swapped = true;
	while(swapped) {
		swapped = false;
		for(int i=0; i<hl->size()-1; ++i) {
			if (descending) {
				if (hl->at(i)->getTransaction()->getIdForApplication() <
				    hl->at(i+1)->getTransaction()->getIdForApplication()) {
					hl->swap(i, i+1);
					swapped = true;
				}
			} else {
				if (hl->at(i)->getTransaction()->getIdForApplication() >
				    hl->at(i+1)->getTransaction()->getIdForApplication()) {
					hl->swap(i, i+1);
					swapped = true;
				}
			}
		}
	}
}
