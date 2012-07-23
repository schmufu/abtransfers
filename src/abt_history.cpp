/******************************************************************************
 * Copyright (C) 2012 Patrick Wacker
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
 *	Diese Klasse wird genutzt um die durchgeführten Aufträge zu verwalten.
 *	Ein Widget kann diese Daten dann nutzen um die durchgeführten Aufträge
 *	anzuzeigen.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#include "abt_history.h"

abt_history::abt_history(QObject *parent) :
	QObject(parent)
{
	this->m_historyList = new QList<abt_jobInfo*>;
}

abt_history::~abt_history()
{	
	//Alle Objecte und die Liste selbst löschen
	this->clearAll();
	delete this->m_historyList;
}

//public
void abt_history::add(abt_jobInfo *job)
{
	if (!job) return; //no NULL objects

	//only at job to history if wanted
	switch (job->getAbJobType()) {
	case AB_Job_TypeCreateDatedTransfer:
	case AB_Job_TypeCreateStandingOrder:
	case AB_Job_TypeDebitNote:
//	case AB_Job_TypeDeleteDatedTransfer:
//	case AB_Job_TypeDeleteStandingOrder:
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
		break;
	}



}

//public
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
void abt_history::clearAll()
{
	//Alle Objecte löschen
	while (this->m_historyList->size() != 0) {
		abt_jobInfo *j = this->m_historyList->takeFirst();
		delete j;
	}
	emit this->historyListChanged(this);
}

/**
  * der zurück gegebene AB_IMEXPORTER_CONTEXT muss über AB_ImExporterContext_free()
  * wieder freigegeben werden!
  */
//public
AB_IMEXPORTER_CONTEXT *abt_history::getContext() const
{
	//wir erstellen unseren eigenen Account für die History
	AB_IMEXPORTER_ACCOUNTINFO *iea = AB_ImExporterAccountInfo_new();

	AB_ImExporterAccountInfo_SetAccountNumber(iea, "0000000000");
	AB_ImExporterAccountInfo_SetAccountName(iea, "AB-Transfers HistoryAccount");
	AB_ImExporterAccountInfo_SetBankCode(iea, "00000000");
	AB_ImExporterAccountInfo_SetBankName(iea, "AB-Transfers History");
	AB_ImExporterAccountInfo_SetOwner(iea, "AB-Transfers");
	AB_ImExporterAccountInfo_SetDescription(iea, "History Objects from AB-Transfers");
	AB_ImExporterAccountInfo_SetCurrency(iea, "EUR");

	Q_FOREACH(const abt_jobInfo* job, *this->m_historyList) {

		/** \todo Hier muss evt. noch der Status des Jobs in die
		 *        Transaction übertragen werden!
		 */

		/** \todo Hierdurch wird z.B. eine Umbuchung als Überweisung
		 *	  gespeichert. Beim laden der History wird somit eine
		 *	  Umbuchung als "Überweisung" und nicht mehr als
		 *	  Umbuchung angezeigt.
		 *	  Welches Feld von AB_TRANSACTION könnte "missbraucht"
		 *	  werden um den 'wahren' Typ zu Kennzeichnen? (Category?)
		 */

		AB_TRANSACTION *t = NULL;

		switch(job->getAbJobType()) {
		case AB_Job_TypeCreateDatedTransfer:
		case AB_Job_TypeModifyDatedTransfer:
		case AB_Job_TypeDeleteDatedTransfer:
			//DatedTransfer der History hinzufügen
			t = AB_Transaction_dup(job->getTransaction()->getAB_Transaction());
			AB_ImExporterAccountInfo_AddDatedTransfer(iea, t);
			break;
		case AB_Job_TypeCreateStandingOrder:
		case AB_Job_TypeModifyStandingOrder:
		case AB_Job_TypeDeleteStandingOrder:
			//StandingOrder der History hinzufügen
			t = AB_Transaction_dup(job->getTransaction()->getAB_Transaction());
			AB_ImExporterAccountInfo_AddStandingOrder(iea, t);
			break;
		case AB_Job_TypeTransfer:
		case AB_Job_TypeEuTransfer:
		case AB_Job_TypeInternalTransfer:
		case AB_Job_TypeLoadCellPhone:
		case AB_Job_TypeSepaDebitNote:
		case AB_Job_TypeDebitNote:
		case AB_Job_TypeSepaTransfer:
			//Transfer der History hinzufügen
			t = AB_Transaction_dup(job->getTransaction()->getAB_Transaction());
			AB_ImExporterAccountInfo_AddTransfer(iea, t);
			break;

		default: //andere Objecte vorerst nicht möglich
			break;
		}
	}

	//Jetzt sind alle History-Objecte dem History Account zugeordnet
	AB_IMEXPORTER_CONTEXT *iec = AB_ImExporterContext_new();
	AB_ImExporterContext_AddAccountInfo(iec, iea);

	return iec;
}

/**
 * In the IdForApplication the unix timestamp of the creation is saved.
 * This function sorts all items by this timestamp in an descending order (default)
 **/
//private
void abt_history::sortListByTimestamp(bool descending)
{
	//bubble sort
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
