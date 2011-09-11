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

#include "widgettransfer.h"

#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

widgetTransfer::widgetTransfer(AB_JOB_TYPE type,
			       const abt_transactionLimits *limits,
			       QWidget *parent) :
	QWidget(parent)
{
	Q_ASSERT(limits);
	this->limits = limits;
	this->localAccount = NULL;
	this->remoteAccount = NULL;
	this->value = NULL;
	this->purpose = NULL;
	this->recurrence = NULL;
	this->textKey = NULL;
	this->layoutMain = new QVBoxLayout();

	switch (type) {
	case AB_Job_TypeTransfer : // Normal Transfer
		this->my_create_transfer_form(true);
		break;
	case AB_Job_TypeSepaTransfer :
	case AB_Job_TypeEuTransfer :

	case AB_Job_TypeInternalTransfer :

	case AB_Job_TypeCreateDatedTransfer :
	case AB_Job_TypeModifyDatedTransfer :

	case AB_Job_TypeCreateStandingOrder :
	case AB_Job_TypeModifyStandingOrder :

	case AB_Job_TypeDebitNote :
	case AB_Job_TypeSepaDebitNote :

	case AB_Job_TypeLoadCellPhone :
	case AB_Job_TypeGetTransactions :
	case AB_Job_TypeGetStandingOrders :
	case AB_Job_TypeDeleteStandingOrder :
	case AB_Job_TypeGetDatedTransfers :
	case AB_Job_TypeDeleteDatedTransfer :
	case AB_Job_TypeGetBalance :
	case AB_Job_TypeUnknown :
	default:
		qWarning() << "type" << type << "not supported for widgetTransfer!";
		break;
	}

	this->setLayout(this->layoutMain);

}

widgetTransfer::~widgetTransfer()
{
	qDebug() << this << "deleting";
	delete this->groupBoxLocal;
	delete this->groupBoxRemote;
	qDebug() << this << "deleted";
}

//private
void widgetTransfer::my_create_internal_transfer_form(bool newTransfer)
{

}

//private
void widgetTransfer::my_create_standing_order_form(bool newTransfer)
{

}

//private
void widgetTransfer::my_create_transfer_form(bool newTransfer)
{
	this->my_create_local_remote_horizontal(newTransfer);
	this->my_create_value_with_label_left();
	this->my_create_purpose();
	this->my_create_textKey();

	this->layoutMain->addLayout(this->layoutAccount);
	this->layoutMain->addLayout(this->layoutValue);
	this->layoutMain->addLayout(this->layoutPurpose);
	this->layoutMain->addWidget(this->textKey, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_local_remote_horizontal(bool newTransfer)
{
	this->my_create_localAccount_groupbox(newTransfer);
	this->my_create_remoteAccount_groupbox(newTransfer);

	this->layoutAccount = new QHBoxLayout();
	this->layoutAccount->addWidget(this->groupBoxLocal);
	this->layoutAccount->addWidget(this->groupBoxRemote);
}

//private
void widgetTransfer::my_create_local_remote_vertical(bool newTransfer)
{
	this->my_create_localAccount_groupbox(newTransfer);
	this->my_create_remoteAccount_groupbox(newTransfer);

	this->layoutAccount = new QVBoxLayout();
	this->layoutAccount->addWidget(this->groupBoxLocal);
	this->layoutAccount->addWidget(this->groupBoxRemote);
}

//private
void widgetTransfer::my_create_localAccount_groupbox(bool newTransfer)
{
	this->groupBoxLocal = new QGroupBox(tr("Absender"));
	QVBoxLayout *gbll = new QVBoxLayout();
	this->localAccount = new widgetAccountData(this);
	this->localAccount->setAllowDropAccount(newTransfer);
	this->localAccount->setAllowDropKnownRecipient(false);
	this->localAccount->setLimitAllowChangeAccountNumber(newTransfer);
	this->localAccount->setLimitAllowChangeBankCode(newTransfer);
	this->localAccount->setLimitAllowChangeBankName(newTransfer);
	this->localAccount->setLimitAllowChangeName(newTransfer);
	this->localAccount->setLimitMaxLenAccountNumber(this->limits->MaxLenLocalAccountNumber);
	this->localAccount->setLimitMaxLenBankCode(this->limits->MaxLenLocalBankCode);
	this->localAccount->setLimitMaxLenName(this->limits->MaxLenLocalName);
	gbll->addWidget(this->localAccount);
	this->groupBoxLocal->setLayout(gbll);
}

//private
void widgetTransfer::my_create_remoteAccount_groupbox(bool /* newTransfer */)
{
	this->groupBoxRemote = new QGroupBox(tr("Empfänger"));
	QVBoxLayout *gbrl = new QVBoxLayout();
	this->remoteAccount = new widgetAccountData(this);
	this->remoteAccount->setAllowDropAccount(false);
	this->remoteAccount->setAllowDropKnownRecipient(true); //this->limits->AllowChangeRecipientAccount);
	this->remoteAccount->setLimitAllowChangeAccountNumber(this->limits->AllowChangeRecipientAccount);
	this->remoteAccount->setLimitAllowChangeBankCode(this->limits->AllowChangeRecipientAccount);
	this->remoteAccount->setLimitAllowChangeBankName(this->limits->AllowChangeRecipientAccount);
	this->remoteAccount->setLimitAllowChangeName(this->limits->AllowChangeRecipientName);
	this->remoteAccount->setLimitMaxLenAccountNumber(this->limits->MaxLenRemoteAccountNumber);
	this->remoteAccount->setLimitMaxLenBankCode(this->limits->MaxLenRemoteBankCode);
	this->remoteAccount->setLimitMaxLenName(this->limits->MaxLenRemoteName);
	gbrl->addWidget(this->remoteAccount);
	this->groupBoxRemote->setLayout(gbrl);
}

//private
void widgetTransfer::my_create_value_with_label_left()
{
	QLabel *labelValue = new QLabel(tr("Betrag (Euro,Cent):"));
	this->value = new widgetValue(this);
	this->value->setLimitAllowChange(this->limits->AllowChangeValue);
	this->layoutValue = new QHBoxLayout();
	this->layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
	this->layoutValue->addWidget(this->value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_value_with_label_top()
{
	QLabel *labelValue = new QLabel(tr("Betrag: (Euro,Cent)"));
	this->value = new widgetValue(this);
	this->value->setLimitAllowChange(this->limits->AllowChangeValue);
	this->layoutValue = new QVBoxLayout();
	this->layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
	this->layoutValue->addWidget(this->value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_purpose()
{

	this->purpose = new widgetPurpose(this);
	this->purpose->setLimitAllowChange(this->limits->AllowChangePurpose);
	this->purpose->setLimitMaxLines(this->limits->MaxLinesPurpose);
	this->purpose->setLimitMaxLen(this->limits->MaxLenPurpose);

	QLabel *labelPurpose = new QLabel("Verwendungszweck");
	this->layoutPurpose = new QVBoxLayout();
	this->layoutPurpose->addWidget(labelPurpose);
	this->layoutPurpose->addWidget(this->purpose);
}

//private
void widgetTransfer::my_create_textKey()
{
	QList<int> allowedTextKeys;
	foreach (QString key, limits->ValuesTextKey) {
		allowedTextKeys.append(key.toInt());
	}

	this->textKey = new widgetTextKey(&allowedTextKeys);
	this->textKey->setLimitAllowChange(this->limits->AllowChangeTextKey);
}

/**** AB_JOB_TYPE's
	case AB_Job_TypeCreateDatedTransfer :
	case AB_Job_TypeCreateStandingOrder :
	case AB_Job_TypeDebitNote :
	case AB_Job_TypeDeleteDatedTransfer :
	case AB_Job_TypeDeleteStandingOrder :
	case AB_Job_TypeEuTransfer :
	case AB_Job_TypeGetBalance :
	case AB_Job_TypeGetDatedTransfers :
	case AB_Job_TypeGetStandingOrders :
	case AB_Job_TypeGetTransactions :
	case AB_Job_TypeInternalTransfer :
	case AB_Job_TypeLoadCellPhone :
	case AB_Job_TypeModifyDatedTransfer :
	case AB_Job_TypeModifyStandingOrder :
	case AB_Job_TypeSepaDebitNote :
	case AB_Job_TypeSepaTransfer :
	case AB_Job_TypeTransfer :
	case AB_Job_TypeUnknown :
*/
