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

#ifndef PAGE_DA_EDIT_DELETE_H
#define PAGE_DA_EDIT_DELETE_H

#include <QWidget>
#include <QCalendarWidget>
#include <QTreeWidgetItem>

#include "../aqb_accounts.h"
#include "../aqb_banking.h"

#include "../widgets/bankaccountswidget.h"
#include "../widgets/ueberweisungswidget.h"
#include "../widgets/knownempfaengerwidget.h"

namespace Ui {
	class Page_DA_Edit_Delete;
}

class Page_DA_Edit_Delete : public QWidget {
	Q_OBJECT
private:
	Ui::Page_DA_Edit_Delete *ui;
	aqb_Accounts *accounts;
	const aqb_banking *banking;
	BankAccountsWidget *accountwidget;
	UeberweisungsWidget *ueberweisungwidget;
	KnownEmpfaengerWidget *knownempfaengerwidget;

	QCalendarWidget *cal1,*cal2;

public:
	Page_DA_Edit_Delete(const aqb_banking *banking, aqb_Accounts *acc, QWidget *parent = 0);
	~Page_DA_Edit_Delete();

protected:
	void changeEvent(QEvent *e);

private:
	//void fillKnownDAs(const QList<abt_DAInfo*> *list);

private slots:
	void on_treeWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void on_pushButton_Revert_clicked();
	void debug_Slot(const abt_EmpfaengerInfo *data);
	void account_selected(const aqb_AccountInfo *account);
};

#endif // PAGE_DA_EDIT_DELETE_H
