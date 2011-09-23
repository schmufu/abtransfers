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

#ifndef WIDGETACCOUNTCOMBOBOX_H
#define WIDGETACCOUNTCOMBOBOX_H

#include <QWidget>
#include <QtGui/QComboBox>

class aqb_AccountInfo;
class aqb_Accounts;

class widgetAccountComboBox : public QWidget
{
	Q_OBJECT
public:
	explicit widgetAccountComboBox(const aqb_AccountInfo *acc,
				       const aqb_Accounts *allAccounts,
				       QWidget *parent = 0);

private:
	const aqb_AccountInfo *m_startAccount;
	const aqb_Accounts *m_allAccounts;
	QComboBox *comboBox;

public:
	const aqb_AccountInfo* getAccount() const;
	bool hasChanges() const;

signals:
	void selectedAccountChanged(const aqb_AccountInfo *newAccount);

private slots:
	void comboBoxNewAccountSelected(int idx);

public slots:
	void setSelectedAccount(const aqb_AccountInfo* account);

};

#endif // WIDGETACCOUNTCOMBOBOX_H
