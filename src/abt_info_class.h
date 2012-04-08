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
#include <aqbanking/job.h>

#include "abt_transaction_base.h"


class abt_info_class
{
public:
	explicit abt_info_class();
	~abt_info_class();
};









class abt_job_info
{
private:
	AB_JOB *job;
	QStringList *jobInfo;

	//! creates the information that is displayed at the "Ausgang" page
	void createJobInfoStringList(QStringList *strList) const;

	void createJobInfoStringList_Standard_Text(QStringList *strList) const;
	void createJobInfoStringList_Append_From(QStringList *strList) const;
	void createJobInfoStringList_Append_To(QStringList *strList) const;
	void createJobInfoStringList_Append_Purpose(QStringList *strList) const;
	void createJobInfoStringList_Append_Value(QStringList *strList) const;

	void createJobInfoStringList_ForStandingOrders(QStringList *strList) const;

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


public:
	abt_job_info(AB_JOB *j);
	~abt_job_info();

	AB_JOB_STATUS getAbJobStatus() const;
	const QString getStatus() const;

	AB_JOB_TYPE getAbJobType() const;
	const QString getType() const;

	const QStringList* getInfo() const;

	AB_JOB *getJob() const;

	const abt_transaction* getAbtTransaction() const;

	const AB_ACCOUNT* getAbAccount() const;

	const QString getKontoNr() const;
	const QString getBLZ() const;
};

























#endif // ABT_INFO_CLASS_H
