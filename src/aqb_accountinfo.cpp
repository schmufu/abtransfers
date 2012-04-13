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
#include "abt_conv.h"

aqb_AccountInfo::aqb_AccountInfo(AB_ACCOUNT *account, QObject *parent) :
	QObject(parent)
{
	this->m_account = account;
	this->m_ID = AB_Account_GetUniqueId(this->m_account);
	this->m_standingOrders = NULL;
	this->m_datedTransfers = NULL;
	this->m_availableJobs = NULL;
	this->account_status = NULL;

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

//	//alle bekannten Daueraufträge für diesen Account holen
//	this->loadKnownStandingOrders();
//	//alle bekannten Terminüberweisungen für diesen Account holen
//	this->loadKnownDatedTransfers();

	//alle Limits für die Jobs dieses Accounts auslesen und im QHash merken
	this->m_limits = new QHash<AB_JOB_TYPE, abt_transactionLimits*>;
	abt_job_ctrl::createTransactionLimitsFor(this->m_account, this->m_limits);

	//alle unterstützen Transaktionen für diesen Account merken
	this->m_availableJobs = new QHash<AB_JOB_TYPE, bool>;
	abt_job_ctrl::createAvailableHashFor(this->m_account, this->m_availableJobs);

	qDebug().nospace() << "AccountInfo for Account " << this->Number() << " created. (ID:" << this->get_ID() << ")";
}

aqb_AccountInfo::~aqb_AccountInfo()
{
	qDebug() << Q_FUNC_INFO << this << "destructor: started";
	//der account_status ist eine von uns angelegte Kopie, diese wieder
	//freigeben
	if (this->account_status)
		AB_AccountStatus_free(this->account_status);


	this->clearStandingOrders(); //Alle StandingOrders wieder freigeben
	delete this->m_standingOrders; //und liste löschen


	this->clearDatedTransfers(); //Alle DatedTransfers wieder freigeben
	delete this->m_datedTransfers; //und liste löschen


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


//public
const abt_transactionLimits* aqb_AccountInfo::limits(AB_JOB_TYPE type) const
{
	return this->m_limits->value(type,NULL);
}



//protected
void aqb_AccountInfo::setAccountStatus(AB_ACCOUNT_STATUS *as)
{
	//if we already have an account-status we must free this first
	if (this->account_status)
		AB_AccountStatus_free(this->account_status);

	//we need to store a copy of the supplied status
	this->account_status = AB_AccountStatus_dup(as);
	emit this->accountStatusChanged(this);
}

/**
  * der zurück gegebene AB_IMEXPORTER_CONTEXT muss über AB_ImExporterContext_free()
  * wieder freigegeben werden!
  */
//protected
AB_IMEXPORTER_CONTEXT *aqb_AccountInfo::getContext() const
{
	AB_IMEXPORTER_ACCOUNTINFO *iea = AB_ImExporterAccountInfo_new();
	AB_ImExporterAccountInfo_FillFromAccount(iea, this->m_account);

	if (this->account_status) { //Wenn wir einen AB_ACCOUNT_STATUS besitzen
		//davon eine Kopie erstellen und diese dem AccountInfoContext
		//hinzufügen (wird beim freigeben des ctx dann auch freigegeben)
		AB_ACCOUNT_STATUS *status = AB_AccountStatus_dup(this->account_status);
		AB_ImExporterAccountInfo_AddAccountStatus(iea, status);
	}


	/** \todo Hier dann auch noch die SOs und DTs hinzufügen

	  */

	if (this->m_standingOrders) {
		//alle vorhandenen Daueraufträge dem Context hinzufügen
		for(int i=0; i<this->m_standingOrders->size(); ++i) {
			abt_standingOrderInfo *so = this->m_standingOrders->at(i);
			AB_TRANSACTION *t = AB_Transaction_dup(so->getTransaction()->getAB_Transaction());
			AB_ImExporterAccountInfo_AddStandingOrder(iea, t);
		}
	}

	if (this->m_datedTransfers) {
		//alle vorhandenen terminierten Überweisungen dem Context hinzufügen
		for(int i=0; i<this->m_datedTransfers->size(); ++i) {
			abt_datedTransferInfo *dt = this->m_datedTransfers->at(i);
			AB_TRANSACTION *t = AB_Transaction_dup(dt->getTransaction()->getAB_Transaction());
			AB_ImExporterAccountInfo_AddDatedTransfer(iea, t);
		}
	}

	AB_IMEXPORTER_CONTEXT *ctx = AB_ImExporterContext_new();
	AB_ImExporterContext_AddAccountInfo(ctx, iea);

	return ctx;
}

/**
  * fügt einen Dauerauftrag in die Liste dieses Accounts ein
  */
//protected
void aqb_AccountInfo::addStandingOrder(abt_standingOrderInfo *so)
{
	if (!so) return; //wenn so == NULL --> Nicht speichern

	if (!this->m_standingOrders) {
		//die liste existiert noch nicht, wir erstellen eine
		this->m_standingOrders = new QList<abt_standingOrderInfo*>();
	}
	this->m_standingOrders->append(so);
	emit this->knownStandingOrdersChanged(this);
}

/**
  * fügt eine terminierte Überweisung in die Liste dieses Accounts ein
  */
//protected
void aqb_AccountInfo::addDatedTransfer(abt_datedTransferInfo *dt)
{
	if (!dt) return; //wenn dt == NULL --> Nicht speichern

	if (!this->m_datedTransfers) {
		//die liste existiert noch nicht, wir erstellen eine
		this->m_datedTransfers = new QList<abt_datedTransferInfo*>();
	}
	this->m_datedTransfers->append(dt);
	emit this->knownDatedTransfersChanged(this);
}


void aqb_AccountInfo::clearStandingOrders()
{
	if (!this->m_standingOrders) return; //abbruch, Liste existiert nicht

	//Die Liste der Daueraufträge leeren
	while (this->m_standingOrders->size() != 0) {
		delete this->m_standingOrders->takeFirst();
	}
	emit this->knownStandingOrdersChanged(this);
}

void aqb_AccountInfo::clearDatedTransfers()
{
	if (!this->m_datedTransfers) return; //abbruch, Liste existiert nicht

	//Die Liste der Daueraufträge leeren
	while (this->m_datedTransfers->size() != 0) {
		delete this->m_datedTransfers->takeFirst();
	}
	emit this->knownDatedTransfersChanged(this);
}

bool aqb_AccountInfo::removeStandingOrder(abt_standingOrderInfo *so)
{
	if (!this->m_standingOrders) return false; //abbruch, Liste existiert nicht

	QString FiId = so->getTransaction()->getFiId();

	for(int i=0; i<this->m_standingOrders->size(); ++i) {
		if (this->m_standingOrders->at(i)->getTransaction()->getFiId() == FiId) {
			//zu löschenden Auftrag gefunden
			delete this->m_standingOrders->takeAt(i);
			emit this->knownStandingOrdersChanged(this);
			return true;
		}
	}

	//wenn wir hierher kommen wurde der so nicht gefunden
	return false;
}

bool aqb_AccountInfo::removeDatedTransfer(abt_datedTransferInfo *dt)
{
	if (!this->m_datedTransfers) return false; //abbruch, Liste existiert nicht

	QString FiId = dt->getTransaction()->getFiId();

	for(int i=0; i<this->m_datedTransfers->size(); ++i) {
		if (this->m_datedTransfers->at(i)->getTransaction()->getFiId() == FiId) {
			//zu löschenden Auftrag gefunden
			delete this->m_datedTransfers->takeAt(i);
			emit this->knownDatedTransfersChanged(this);
			return true;
		}
	}

	//wenn wir hierher kommen wurde der so nicht gefunden
	return false;
}


//public
QString aqb_AccountInfo::getBankLine() const
{
	if (!this->account_status) return QString();

	const AB_VALUE *v;
	QString value = ""; //empty string als default

	v = AB_AccountStatus_GetBankLine(this->account_status);
	if (v) {
		value = QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2);
	}

	return value;
}

//public
QString aqb_AccountInfo::getNotedBalance() const
{
	if (!this->account_status) return QString();

	const AB_VALUE *v;
	const AB_BALANCE *b;
	QString value = ""; //empty string als default

	b = AB_AccountStatus_GetNotedBalance(this->account_status);
	if (b) {
		v = AB_Balance_GetValue(b);
		if (v) {
			value = QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2);
		}
	}

	return value;
}

//public
QString aqb_AccountInfo::getBookedBalance() const
{
	if (!this->account_status) return QString();

	const AB_VALUE *v;
	const AB_BALANCE *b;
	QString value = ""; //empty string als default

	b = AB_AccountStatus_GetBookedBalance(this->account_status);
	if (b) {
		v = AB_Balance_GetValue(b);
		if (v) {
			value = QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2);
		}
	}

	return value;
}

//public
QString aqb_AccountInfo::getDisposable() const
{
	if (!this->account_status) return QString();

	const AB_VALUE *v;
	QString value = ""; //empty string als default

	v = AB_AccountStatus_GetDisposable(this->account_status);
	if (v) {
		value = QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2);
	}

	return value;
}

//public
QString aqb_AccountInfo::getDisposed() const
{
	if (!this->account_status) return QString();

	const AB_VALUE *v;
	QString value = ""; //empty string als default

	v = AB_AccountStatus_GetDisposed(this->account_status);
	if (v) {
		value = QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2);
	}

	return value;
}

//public
QDate aqb_AccountInfo::getDate() const
{
	if (!this->account_status) return QDate();

	QDate date = abt_conv::GwenTimeToQDate(AB_AccountStatus_GetTime(this->account_status));

	//logmsg2 = QString("Time:\t%1").arg(date.toString(Qt::DefaultLocaleLongDate));

	return date;
}










