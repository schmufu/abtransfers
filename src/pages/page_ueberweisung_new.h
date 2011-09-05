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

#ifndef PAGE_UEBERWEISUNG_NEW_H
#define PAGE_UEBERWEISUNG_NEW_H

#include <QWidget>
#include <QPushButton>
#include <QLayout>

#include "../aqb_accounts.h"
#include "../aqb_banking.h"
#include "../abt_transaction_base.h"
#include "../abt_job_ctrl.h"

#include "../widgets/bankaccountswidget.h"
#include "../widgets/ueberweisungswidget.h"
#include "../widgets/knownempfaengerwidget.h"

class Page_Ueberweisung_New : public QWidget
{
Q_OBJECT
private:
	aqb_Accounts *accounts;
	const aqb_banking *banking;
	const abt_job_ctrl *jobctrl;
	BankAccountsWidget *accountwidget;
	UeberweisungsWidget *ueberweisungwidget;
	KnownEmpfaengerWidget *knownempfaengerwidget;

	QPushButton *pushButton_Execute;
	QPushButton *pushButton_Revert;

	QVBoxLayout *main_layout;


public:
	explicit Page_Ueberweisung_New(const aqb_banking *banking,
				       const abt_job_ctrl *jobctrl,
				       aqb_Accounts *acc, QWidget *parent = 0);
	~Page_Ueberweisung_New();

signals:
	void createTransfer(aqb_AccountInfo *a, const abt_transaction *t);


public slots:
	void account_selected(const aqb_AccountInfo *account);

	void pushButton_Execute_clicked();
	void pushButton_Revert_clicked();


};

#endif // PAGE_UEBERWEISUNG_NEW_H
