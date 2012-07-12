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

#ifndef BANKACCOUNTSWIDGET_H
#define BANKACCOUNTSWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "../aqb_accounts.h"

namespace Ui {
	class BankAccountsWidget;
}

/** \brief Widget zur Anzeige aller bei AqBanking registrierten Konten
 *
 * Dieses Widget zeigt alle Konten an die bei AqBanking eingetragen sind.
 *
 */

class BankAccountsWidget : public QWidget {
	Q_OBJECT
private:
	const aqb_Accounts *m_accounts;
public:
	BankAccountsWidget(const aqb_Accounts *accounts, QWidget *parent = 0);
	~BankAccountsWidget();

	aqb_AccountInfo *getSelectedAccount();

protected:
	QPoint dragStartPos;
	aqb_AccountInfo *dragObj;

	void changeEvent(QEvent *e);
	bool eventFilter(QObject *obj, QEvent *event);
	void twMouseMoveEvent(QMouseEvent *event);
	void twMousePressEvent(QMouseEvent *event);


private:
	Ui::BankAccountsWidget *ui;

	void setValuesForItem(QTreeWidgetItem *item, const aqb_AccountInfo *acc) const;

signals:
	void Account_Changed(const aqb_AccountInfo *new_account);

public slots:
	void setSelectedAccount(const aqb_AccountInfo *account);
	void accountChangedUpdateDisplay(const aqb_AccountInfo *account);
	void setAccounts(const aqb_Accounts *accounts);

private slots:
	void on_treeWidget_itemSelectionChanged();
};

#endif // BANKACCOUNTSWIDGET_H
