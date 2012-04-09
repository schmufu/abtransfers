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

#ifndef ABT_INFO_CLASS_H
#define ABT_INFO_CLASS_H

#include <QString>
#include <QtCore/QDateTime>
#include <aqbanking/job.h>

class abt_info_class
{
public:
	explicit abt_info_class();
	~abt_info_class();
};





/** \brief Bietet eine Abstrahierung von AB_JOB
  *
  * Über die beiden Konstruktoren kann diese Klasse entweder als Container
  * für einen AB_JOB oder als "Historie"-Object verwendet werden.
  *
  * Bei einer Instantiierung mit einem AB_JOB werden dessen Informationen
  * genutzt um die weiteren Eigenschaften entsprechend zu setzen. Dies wird
  * in abt_job_ctrl verwendet um eine Liste aller noch auszuführenden Jobs
  * zu verwalten. Auch die Anzeige im Ausgang wird direkt mit den Eigenschaften
  * dieser Klasse dargestellt.
  *
  * Bei einer Instantiierung ohne AB_JOB müssen die benötigten Informationen
  * bei der Erstellung übergeben werden. Die Detail-Informationen werden dann
  * genauso erstellt als wenn ein AB_JOB übergeben worden wäre.
  *
  * Über getJob() kann selbstverständlich nur bei der Erstellung mit einem
  * AB_JOB ein gültiger Zeiger zurückgegeben werden (ansonsten NULL).
  *
  */

class abt_transaction; //Einbindung des Headers erst in der .cpp

class abt_jobInfo
{
public:
	/** \brief Constructor zur Verwendung als AB_JOB Container */
	abt_jobInfo(AB_JOB *j);
	/** \brief Constructor zur Verwendung als Historie Object */
	abt_jobInfo(const AB_JOB_TYPE type, const AB_JOB_STATUS status,
		    const abt_transaction *trans, const AB_ACCOUNT *acc,
		    const QDateTime date);
	~abt_jobInfo();

private:
	AB_JOB *m_job;
	QStringList *m_jobInfo;
	const abt_transaction *m_trans;
	const AB_ACCOUNT *m_ABAccount;
	AB_JOB_TYPE m_jobType;
	AB_JOB_STATUS m_jobStatus;
	QDateTime m_date;

	//! creates the information that is displayed at the "Ausgang" page
	void createJobInfoStringList(QStringList *strList) const;

	void createJobInfoStringList_Standard_Text(QStringList *strList) const;
	void createJobInfoStringList_Append_From(QStringList *strList) const;
	void createJobInfoStringList_Append_To(QStringList *strList) const;
	void createJobInfoStringList_Append_Purpose(QStringList *strList) const;
	void createJobInfoStringList_Append_Value(QStringList *strList) const;

	void createJobInfoStringList_ForStandingOrders(QStringList *strList) const;
	void createJobInfoStringList_ForDatedTransfers(QStringList *strList) const;

	void createJobInfoStringList_CreateDatedTransfer(QStringList *strList) const;
	void createJobInfoStringList_CreateStandingOrder(QStringList *strList) const;
	void createJobInfoStringList_DebitNote(QStringList *strList) const;
	void createJobInfoStringList_DeleteDatedTransfer(QStringList *strList) const;
	void createJobInfoStringList_DeleteStandingOrder(QStringList *strList) const;
	void createJobInfoStringList_EuTransfer(QStringList *strList) const;
	void createJobInfoStringList_GetBalance(QStringList *strList) const;
	void createJobInfoStringList_GetDatedTransfers(QStringList *strList) const;
	void createJobInfoStringList_GetStandingOrders(QStringList *strList) const;
	void createJobInfoStringList_GetTransactions(QStringList *strList) const;
	void createJobInfoStringList_InternalTransfer(QStringList *strList) const;
	void createJobInfoStringList_LoadCellPhone(QStringList *strList) const;
	void createJobInfoStringList_ModifyDatedTransfer(QStringList *strList) const;
	void createJobInfoStringList_ModifyStandingOrder(QStringList *strList) const;
	void createJobInfoStringList_SepaDebitNote(QStringList *strList) const;
	void createJobInfoStringList_SepaTransfer(QStringList *strList) const;
	void createJobInfoStringList_Transfer(QStringList *strList) const;
	void createJobInfoStringList_Unknown(QStringList *strList) const;

	void setMyTransactionFromJob();

public:
	AB_JOB_STATUS getAbJobStatus() const;
	const QString getStatus() const;

	AB_JOB_TYPE getAbJobType() const;
	const QString getType() const;

	/** \brief Der Info-Text zur Anzeige im Ausgang oder Historie */
	const QStringList* getInfo() const;

	/** \brief gibt den enthaltenen AB_Job zurück (could be NULL!) */
	AB_JOB *getJob() const;

	const abt_transaction* getTransaction() const { return this->m_trans; };

	const AB_ACCOUNT* getAbAccount() const;

	const QString getKontoNr() const;
	const QString getBLZ() const;
	int getAccountID() const;
};

























#endif // ABT_INFO_CLASS_H
