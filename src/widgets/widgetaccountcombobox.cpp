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

#include "widgetaccountcombobox.h"
#include <QtGui/QLayout>

#include "../aqb_accounts.h"
#include "../aqb_accountinfo.h"

widgetAccountComboBox::widgetAccountComboBox(const aqb_AccountInfo *acc,
					     const aqb_Accounts *allAccounts,
					     QWidget *parent) :
	QWidget(parent)
{
	this->m_startAccount = acc;
	this->m_allAccounts = allAccounts;

	QVBoxLayout *layoutMain = new QVBoxLayout();
	layoutMain->setContentsMargins(0,0,0,0);

	this->comboBox = new QComboBox(this);

	//Alle Accounts in der ComboBox darstellen
	foreach(const aqb_AccountInfo *account, allAccounts->getAccountHash().values()) {
		QString cbText = QString("%1").arg(account->Name());
		this->comboBox->insertItem(200, cbText, QVariant::fromValue(account));
	}

	layoutMain->addWidget(this->comboBox);

	this->setLayout(layoutMain);

	this->setSelectedAccount(acc);

	connect(this->comboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(comboBoxNewAccountSelected(int)));

}

//public
const aqb_AccountInfo* widgetAccountComboBox::getAccount() const
{
	int idx = this->comboBox->currentIndex();
	return this->comboBox->itemData(idx, Qt::UserRole).value<const aqb_AccountInfo*>();
}

//public
bool widgetAccountComboBox::hasChanges() const
{
	return this->m_startAccount != this->getAccount();
}

//private slot
void widgetAccountComboBox::comboBoxNewAccountSelected(int idx)
{
	const aqb_AccountInfo* newSelAcc;
	newSelAcc = this->comboBox->itemData(idx, Qt::UserRole).value<const aqb_AccountInfo*>();
	emit this->selectedAccountChanged(newSelAcc);
}

//public slot
void widgetAccountComboBox::setSelectedAccount(const aqb_AccountInfo *account)
{
	//den übergebenen Account auswählen
	int cbIdx = this->comboBox->findData(QVariant::fromValue(account));
	if (cbIdx != -1) {
		//qDebug("CBIDX != -1 IST TRUE");
		this->comboBox->setCurrentIndex(cbIdx);
	} else { //ersten Eintrag als default wählen
		qDebug("cbIDX == -1 - ES WIRD 0 ALS DEFAULT GESETZT");
		this->comboBox->setCurrentIndex(0);
	}
}
