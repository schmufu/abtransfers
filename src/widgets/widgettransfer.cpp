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

#include "../aqb_accountinfo.h"

widgetTransfer::widgetTransfer(AB_JOB_TYPE type,
			       const abt_transactionLimits *limits,
			       QWidget *parent) :
	QWidget(parent)
{
	this->m_limits = limits;
	this->m_type = type;
	this->localAccount = NULL;
	this->remoteAccount = NULL;
	this->value = NULL;
	this->purpose = NULL;
	this->recurrence = NULL;
	this->textKey = NULL;
	this->layoutMain = new QVBoxLayout();

	if (this->m_limits == NULL) {
		QLabel *notAvailable = new QLabel(tr("Der \"Job\" '%1' ist bei "
						     "dem ausgewähltem Konto "
						     "nicht verfügbar!").arg(
							abt_conv::JobTypeToQString(type)), this);
		notAvailable->setWordWrap(true);
		QFont labelFont(notAvailable->font());
		labelFont.setBold(true);
		labelFont.setPixelSize(18);
		notAvailable->setFont(labelFont);
		this->layoutMain->addWidget(notAvailable,1,Qt::AlignCenter);
		type = AB_Job_TypeUnknown;
	}

	switch (type) {
	case AB_Job_TypeTransfer : // Normal Transfer
		this->my_create_transfer_form(true);
		break;
	case AB_Job_TypeCreateStandingOrder :
		this->my_create_standing_order_form(true);
		break;
	case AB_Job_TypeModifyStandingOrder :
		this->my_create_standing_order_form(false);
		break;
	case AB_Job_TypeInternalTransfer :
		this->my_create_internal_transfer_form(true);
		break;

	case AB_Job_TypeSepaTransfer :
	case AB_Job_TypeEuTransfer :

	case AB_Job_TypeCreateDatedTransfer :
	case AB_Job_TypeModifyDatedTransfer :

	case AB_Job_TypeDebitNote :
	case AB_Job_TypeSepaDebitNote : {
		this->setWindowTitle(tr("nicht Implementiert"));
		QLabel *notImplementet = new QLabel(tr("Der \"Job\" '%1' ist leider "
						     "noch nicht implementiert.\n"
						     "Bitte haben Sie noch etwas "
						     "Geduld und warten auf eine "
						     "Aktualisierung").arg(
							abt_conv::JobTypeToQString(type)), this);
		notImplementet->setWordWrap(true);
		QFont labelFontNI(notImplementet->font());
		labelFontNI.setBold(true);
		labelFontNI.setPixelSize(14);
		notImplementet->setFont(labelFontNI);
		this->layoutMain->addWidget(notImplementet,1, Qt::AlignLeft | Qt::AlignVCenter);
		}
		break;

	case AB_Job_TypeLoadCellPhone:
	case AB_Job_TypeGetTransactions :
	case AB_Job_TypeGetStandingOrders :
	case AB_Job_TypeDeleteStandingOrder :
	case AB_Job_TypeGetDatedTransfers :
	case AB_Job_TypeDeleteDatedTransfer :
	case AB_Job_TypeGetBalance :
	case AB_Job_TypeUnknown :
	default:
		qWarning() << "type" << type << "not supported for widgetTransfer!";
		this->setWindowTitle(tr("Programmierfehler"));
		QLabel *programError = new QLabel(tr("PROGRAMMIERFEHLER!\n"
						     "Der \"Job\" '%1' wird von "
						     "widgetTransfer nicht "
						     "unterstützt!").arg(
							abt_conv::JobTypeToQString(type)), this);
		programError->setWordWrap(true);
		QFont labelFontPE(programError->font());
		labelFontPE.setBold(true);
		labelFontPE.setPixelSize(18);
		programError->setFont(labelFontPE);
		this->layoutMain->addWidget(programError,1, Qt::AlignLeft | Qt::AlignVCenter);


	}

	this->setAllLimits(this->m_limits);

	this->setLayout(this->layoutMain);

}

widgetTransfer::~widgetTransfer()
{
	qDebug() << this << "deleting";
// Werden wohl durch das hinzufügen zum layout ein child dieses Widgets und
// werden deswegen automatisch beim löschen des parentWidgets auch gelöscht
//	delete this->groupBoxLocal;
//	delete this->groupBoxRemote;
//	delete this->groubBoxRecurrence;
	qDebug() << this << "deleted";
}

//private
void widgetTransfer::my_create_internal_transfer_form(bool newTransfer)
{
	this->setWindowTitle(tr("Umbuchung"));

}

//private
void widgetTransfer::my_create_standing_order_form(bool newTransfer)
{
	if (newTransfer)
		this->setWindowTitle(tr("Dauerauftrag anlegen"));
	else
		this->setWindowTitle(tr("Dauerauftrag bearbeiten"));

	this->my_create_local_remote_horizontal(newTransfer);
	this->my_create_value_with_label_left();
	this->my_create_purpose();
	this->my_create_recurrence();
	this->my_create_textKey();

	this->layoutMain->addLayout(this->layoutAccount);
	this->layoutMain->addLayout(this->layoutValue);
	this->layoutMain->addLayout(this->layoutPurpose);
	this->layoutMain->addWidget(this->groubBoxRecurrence);
	this->layoutMain->addWidget(this->textKey, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_transfer_form(bool newTransfer)
{
	this->setWindowTitle(tr("Überweisung"));
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
	gbll->addWidget(this->localAccount);
	this->groupBoxLocal->setLayout(gbll);

	connect(this->localAccount, SIGNAL(accountChanged(const aqb_AccountInfo*)),
		this, SLOT(onAccountChange(const aqb_AccountInfo*)));
}

//private
void widgetTransfer::my_create_remoteAccount_groupbox(bool /* newTransfer */)
{
	this->groupBoxRemote = new QGroupBox(tr("Empfänger"));
	QVBoxLayout *gbrl = new QVBoxLayout();
	this->remoteAccount = new widgetAccountData(this);
	this->remoteAccount->setAllowDropAccount(false);
	this->remoteAccount->setAllowDropKnownRecipient(true);
	gbrl->addWidget(this->remoteAccount);
	this->groupBoxRemote->setLayout(gbrl);

	/** \todo wird die connection auch für den remoteAccount benötigt?
	  *       Eigentlich nicht, auch bei einem Internal Transfer sind nur
	  *       die Limits des Sendenden Accounts relevant!?!?
	  *	  Das Signal wird sowiso nur gesendet wenn sich der Account
	  *	  ändert! Bei einem KnownRecipient Drop wird nichts gesendet!
	  */
	//connect(this->remoteAccount, SIGNAL(accountChanged(const aqb_AccountInfo*)),
	//	this, SLOT(onAccountChange(const aqb_AccountInfo*)));
}

//private
void widgetTransfer::my_create_value_with_label_left()
{
	QLabel *labelValue = new QLabel(tr("Betrag (Euro,Cent):"));
	this->value = new widgetValue(this);
	this->layoutValue = new QHBoxLayout();
	this->layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
	this->layoutValue->addWidget(this->value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_value_with_label_top()
{
	QLabel *labelValue = new QLabel(tr("Betrag: (Euro,Cent)"));
	this->value = new widgetValue(this);
	this->layoutValue = new QVBoxLayout();
	this->layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
	this->layoutValue->addWidget(this->value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_purpose()
{
	this->purpose = new widgetPurpose(this);

	QLabel *labelPurpose = new QLabel("Verwendungszweck");
	this->layoutPurpose = new QVBoxLayout();
	this->layoutPurpose->addWidget(labelPurpose);
	this->layoutPurpose->addWidget(this->purpose);
}

//private
void widgetTransfer::my_create_textKey()
{
	this->textKey = new widgetTextKey(NULL);
}

//private
void widgetTransfer::my_create_recurrence()
{
	this->groubBoxRecurrence = new QGroupBox(tr("Ausführungsdaten"));
	this->recurrence = new widgetRecurrence(this);

	QVBoxLayout *grbl = new QVBoxLayout();
	grbl->addWidget(this->recurrence);
	this->groubBoxRecurrence->setLayout(grbl);
}
















//private
void widgetTransfer::setAllLimits(const abt_transactionLimits *limits)
{
	//wenn keine Limits vorhanden sind alle Widgets disablen, da ein Job
	//ohne Limits von der Bank nicht unterstützt wird!
	bool dis = limits == NULL;
	if (this->localAccount) this->localAccount->setDisabled(dis);
	if (this->remoteAccount) this->remoteAccount->setDisabled(dis);
	if (this->value) this->value->setDisabled(dis);
	if (this->purpose) this->purpose->setDisabled(dis);
	if (this->textKey) this->textKey->setDisabled(dis);
	if (this->recurrence) this->recurrence->setDisabled(dis);

	if (dis) return; //Abbruch wenn keine Limits vorhanden sind


	if (this->localAccount != NULL) {
		this->localAccount->setLimitMaxLenAccountNumber(limits->MaxLenLocalAccountNumber);
		this->localAccount->setLimitMaxLenBankCode(limits->MaxLenLocalBankCode);
		this->localAccount->setLimitMaxLenName(limits->MaxLenLocalName);
	}

	if (this->remoteAccount != NULL) {
		this->remoteAccount->setLimitAllowChangeAccountNumber(limits->AllowChangeRecipientAccount);
		this->remoteAccount->setLimitAllowChangeBankCode(limits->AllowChangeRecipientAccount);
		this->remoteAccount->setLimitAllowChangeBankName(limits->AllowChangeRecipientAccount);
		this->remoteAccount->setLimitAllowChangeName(limits->AllowChangeRecipientName);
		this->remoteAccount->setLimitMaxLenAccountNumber(limits->MaxLenRemoteAccountNumber);
		this->remoteAccount->setLimitMaxLenBankCode(limits->MaxLenRemoteBankCode);
		this->remoteAccount->setLimitMaxLenName(limits->MaxLenRemoteName);
	}

	if (this->value != NULL) {
		this->value->setLimitAllowChange(limits->AllowChangeValue);
	}

	if (this->purpose != NULL) {
		this->purpose->setLimitAllowChange(limits->AllowChangePurpose);
		this->purpose->setLimitMaxLines(limits->MaxLinesPurpose);
		this->purpose->setLimitMaxLen(limits->MaxLenPurpose);
	}

	if (this->textKey != NULL) {
		int oldKey = this->textKey->getTextKey();
		QList<int> allowedTextKeys;
		foreach (QString key, limits->ValuesTextKey) {
			allowedTextKeys.append(key.toInt());
		}
		this->textKey->fillTextKeys(&allowedTextKeys);
		//den vorher gewählten key wieder setzen. Wenn er in der neuen
		//Liste nicht vorhanden ist oder ungültig (-1) ist wird
		//automatisch der erste Wert der neuen Liste gewählt.
		this->textKey->setTextKey(oldKey);
		this->textKey->setLimitAllowChange(limits->AllowChangeTextKey);
	}

	if (this->recurrence != NULL) {
		this->recurrence->setLimitAllowChangeCycle(limits->AllowChangeCycle);
		this->recurrence->setLimitAllowChangeExecutionDay(limits->AllowChangeExecutionDay);
		this->recurrence->setLimitAllowChangePeriod(limits->AllowChangePeriod);
		this->recurrence->setLimitAllowMonthly(limits->AllowMonthly);
		this->recurrence->setLimitAllowWeekly(limits->AllowWeekly);
		this->recurrence->setLimitAllowChangeFirstExecutionDate(limits->AllowChangeFirstExecutionDate);
		this->recurrence->setLimitAllowChangeLastExecutionDate(limits->AllowChangeLastExecutionDate);

		this->recurrence->setLimitValuesCycleMonth(limits->ValuesCycleMonth);
		this->recurrence->setLimitValuesCycleWeek(limits->ValuesCycleWeek);
		this->recurrence->setLimitValuesExecutionDayMonth(limits->ValuesExecutionDayMonth);
		this->recurrence->setLimitValuesExecutionDayWeek(limits->ValuesExecutionDayWeek);
	}

}

//private slot
void widgetTransfer::onAccountChange(const aqb_AccountInfo *accInfo)
{
	//Neuer Account als Absender wurde gewählt, somit treten evt. neue
	//Limits von diesem Account in kraft.
	this->m_limits = accInfo->limits(this->m_type); //limits merken
	this->setAllLimits(this->m_limits); //und alle limits neu setzen
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


