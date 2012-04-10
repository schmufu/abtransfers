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

#include "aqb_accountinfo.h"
#include <QObject>
#include <QDebug>

#include "globalvars.h"
#include "abt_job_ctrl.h"

aqb_AccountInfo::aqb_AccountInfo(AB_ACCOUNT *account, QObject *parent) :
	QObject(parent)
{
	this->m_account = account;
	this->m_ID = AB_Account_GetUniqueId(this->m_account);
	this->m_KnownStandingOrders = NULL;
	this->m_KnownDatedTransfers = NULL;
	this->m_AvailableJobs = NULL;

	this->m_BankCode = QString::fromUtf8(AB_Account_GetBankCode(this->m_account));
	this->m_BankName = QString::fromUtf8(AB_Account_GetBankName(this->m_account));
	this->m_Number = QString::fromUtf8(AB_Account_GetAccountNumber(this->m_account));
	this->m_Name = QString::fromUtf8(AB_Account_GetAccountName(this->m_account));
	this->m_BackendName = QString::fromUtf8(AB_Account_GetBackendName(this->m_account));
	this->m_SubAccountId = QString::fromUtf8(AB_Account_GetSubAccountId(this->m_account));
	this->m_IBAN = QString::fromUtf8(AB_Account_GetIBAN(this->m_account));
	this->m_BIC = QString::fromUtf8(AB_Account_GetBIC(this->m_account));
	this->m_OwnerName = QString::fromUtf8(AB_Account_GetOwnerName(this->m_account));
	this->m_Currency = QString::fromUtf8(AB_Account_GetCurrency(this->m_account));
	this->m_Country = QString::fromUtf8(AB_Account_GetCountry(this->m_account));
	this->m_BankLine = 0.0;
	this->m_NotedBalance = 0.0;
	this->m_BookedBalance = 0.0;
	this->m_Disposable = 0.0;
	this->m_Disposed = 0.0;

	AB_ACCOUNT_TYPE type;
	type = AB_Account_GetAccountType(this->m_account);
	switch (type) {
		case AB_AccountType_Unknown:
			this->m_AccountType = QObject::tr("unbekannt"); break;
		case AB_AccountType_Bank:
			this->m_AccountType = QObject::tr("Girokonto"); break;
		case AB_AccountType_CreditCard:
			this->m_AccountType = QObject::tr("Kredit-Karte"); break;
		case AB_AccountType_Checking:
			this->m_AccountType = QObject::tr("Checking"); break;
		case AB_AccountType_Savings:
			this->m_AccountType = QObject::tr("Sparkonto"); break;
		case AB_AccountType_Investment:
			this->m_AccountType = QObject::tr("Investment"); break;
		case AB_AccountType_Cash:
			this->m_AccountType = QObject::tr("Bargeld"); break;
		case AB_AccountType_MoneyMarket:
			this->m_AccountType = QObject::tr("MoneyMarket"); break;
		default:
			this->m_AccountType = QObject::tr("type unknown"); break;
	}

	//alle bekannten Daueraufträge für diesen Account holen
	this->loadKnownStandingOrders();
	//alle bekannten Terminüberweisungen für diesen Account holen
	this->loadKnownDatedTransfers();

	//alle Limits für die Jobs dieses Accounts auslesen und im QHash merken
	this->m_limits = new QHash<AB_JOB_TYPE, abt_transactionLimits*>;
	abt_job_ctrl::createTransactionLimitsFor(this->m_account, this->m_limits);

	//alle unterstützen Transaktionen für diesen Account merken
	this->m_AvailableJobs = new QHash<AB_JOB_TYPE, bool>;
	abt_job_ctrl::createAvailableHashFor(this->m_account, this->m_AvailableJobs);

	qDebug().nospace() << "AccountInfo for Account " << this->Number() << " created. (ID:" << this->get_ID() << ")";
}

aqb_AccountInfo::~aqb_AccountInfo()
{
	qDebug() << Q_FUNC_INFO << this << "destructor: started";
	//existierende Daueraufträge für diesen Account speichern
	//abt_settings::saveDAsForAccount(this->m_KnownDAs, this->m_Number, this->m_BankCode);
	//und die Objecte wieder freigeben
	abt_settings::freeStandingOrdersList(this->m_KnownStandingOrders);

	qDebug() << Q_FUNC_INFO << this << "destructor: " << "before foreach";
	//Alle abt_transactionLimits und den QHash wieder löschen
	int i=0;
	foreach (AB_JOB_TYPE type, this->m_limits->keys()) {
		qDebug() << Q_FUNC_INFO << this << "destructor: " << "in foreach -- i="<<i++;
		delete this->m_limits->take(type);
	}
	qDebug() << Q_FUNC_INFO << this << "destructor: " << "after foreach";
	delete this->m_limits;
	qDebug() << Q_FUNC_INFO << this << "deleted";
}

//public slot
/*! \brief lädt die Bekannten Daueraufträge für den übergebenen account */
void aqb_AccountInfo::loadKnownStandingOrders()
{
	//Alle SOs löschen
	abt_settings::freeStandingOrdersList(this->m_KnownStandingOrders);
	//und neu laden
	this->m_KnownStandingOrders = settings->getStandingOrdersForAccount(this->m_Number, this->m_BankCode);

	emit knownStandingOrdersChanged(this);
}

//public slot
/*! \brief lädt die Bekannten Terminüberweisungen für den übergebenen account */
void aqb_AccountInfo::loadKnownDatedTransfers()
{
	//Alle DTs löschen
	abt_settings::freeDatedTransfersList(this->m_KnownDatedTransfers);
	//und neu laden
	this->m_KnownDatedTransfers = settings->getDatedTransfersForAccount(this->m_Number, this->m_BankCode);

	emit knownDatedTransfersChanged(this);
}

//public
const abt_transactionLimits* aqb_AccountInfo::limits(AB_JOB_TYPE type) const
{
	return this->m_limits->value(type,NULL);
}



//protected
void aqb_AccountInfo::setBankLine(double value)
{
	this->m_BankLine = value;
	emit this->accountDataChanged(this);
}

//protected
void aqb_AccountInfo::setNotedBalance(double value)
{
	this->m_NotedBalance = value;
	emit this->accountDataChanged(this);
}

//protected
void aqb_AccountInfo::setBookedBalance(double value)
{
	this->m_BookedBalance = value;
	emit this->accountDataChanged(this);
}

//protected
void aqb_AccountInfo::setDisposable(double value)
{
	this->m_Disposable = value;
	emit this->accountDataChanged(this);
}

//protected
void aqb_AccountInfo::setDisposed(double value)
{
	this->m_Disposed = value;
	emit this->accountDataChanged(this);
}

//protected
void aqb_AccountInfo::setDate(QDate date)
{
	this->m_Date = date;
	emit this->accountDataChanged(this);
}










