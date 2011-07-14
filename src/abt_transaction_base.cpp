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

#include "abt_transaction_base.h"

abt_transaction_base::abt_transaction_base()
{
	this->aqb_transaction = NULL;

	// die abgeleiteten Klassen müssen die private AB_TRANSACTION
	// entsprechend setzen!
}

abt_transaction_base::~abt_transaction_base()
{
	AB_Transaction_free(this->aqb_transaction);
}

/** \todo Vielleicht diese Funktionen in ein separates Objekt
	  z.B. aqConv::GwenTimeToQDate()
*/
/*****************************************************************************
 * static helper functions for type conversions                              *
 *****************************************************************************/
//static
const QDate abt_transaction_base::GwenTimeToQDate(const GWEN_TIME *gwentime)
{
	QDate date;
	if (gwentime) {
		struct tm tmtime;
		tmtime = GWEN_Time_toTm(gwentime);
		date.setDate(tmtime.tm_year+1900, tmtime.tm_mon+1, tmtime.tm_mday);
	} else {
		date.setDate(2011,2,30); //invalid Date wenn gwen_date==NULL
	}

	return date;
}

//static
const GWEN_TIME* abt_transaction_base::QDateToGwenTime(const QDate &date)
{
	/** \todo Der Pointer zu GWEN_TIME muss bei Programmende
		  freigegeben werden!
	*/
	return GWEN_Time_new(date.year(), date.month(), date.day(), 0, 0, 0, 0);
}



/**************************
 * Local Account Info     *
 **************************/

const QString abt_transaction_base::getLocalCountry() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalCountry(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setLocalCountry(const QString &Country)
{
	AB_Transaction_SetLocalCountry(this->aqb_transaction,
				       Country.toUtf8());
}


const QString abt_transaction_base::getLocalBankCode() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalBankCode(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setLocalBankCode(const QString &BankCode)
{
	AB_Transaction_SetLocalBankCode(this->aqb_transaction,
					BankCode.toUtf8());
}


const QString abt_transaction_base::getLocalBranchId() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalBranchId(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setLocalBranchId(const QString &BranchId)
{
	AB_Transaction_SetLocalBranchId(this->aqb_transaction,
					BranchId.toUtf8());
}


const QString abt_transaction_base::getLocalAccountNumber() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalAccountNumber(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setLocalAccountNumber(const QString &AccountNumber)
{
	AB_Transaction_SetLocalAccountNumber(this->aqb_transaction,
					     AccountNumber.toUtf8());
}


const QString abt_transaction_base::getLocalSuffix() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalSuffix(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setLocalSuffix(const QString &Suffix)
{
	AB_Transaction_SetLocalSuffix(this->aqb_transaction,
				      Suffix.toUtf8());
}


const QString abt_transaction_base::getLocalIban() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalIban(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setLocalIban(const QString &Iban)
{
	AB_Transaction_SetLocalIban(this->aqb_transaction,
				    Iban.toUtf8());
}


const QString abt_transaction_base::getLocalName() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalName(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setLocalName(const QString &Name)
{
	AB_Transaction_SetLocalName(this->aqb_transaction,
				    Name.toUtf8());
}


const QString abt_transaction_base::getLocalBic() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalBic(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setLocalBic(const QString &Bic)
{
	AB_Transaction_SetLocalBic(this->aqb_transaction,
				   Bic.toUtf8());
}


/**************************
 * Remote Account Info    *
 **************************/

const QString abt_transaction_base::getRemoteCountry() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteCountry(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteCountry(const QString &Country)
{
	AB_Transaction_SetRemoteCountry(this->aqb_transaction,
					Country.toUtf8());
}


const QString abt_transaction_base::getRemoteBankName() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBankName(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteBankName(const QString &BankName)
{
	AB_Transaction_SetRemoteBankName(this->aqb_transaction,
					 BankName.toUtf8());
}


const QString abt_transaction_base::getRemoteBankLocation() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBankLocation(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteBankLocation(const QString &BankLocation)
{
	AB_Transaction_SetRemoteBankLocation(this->aqb_transaction,
					     BankLocation.toUtf8());
}


const QString abt_transaction_base::getRemoteBankCode() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBankCode(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteBankCode(const QString &BankCode)
{
	AB_Transaction_SetRemoteBankCode(this->aqb_transaction,
					 BankCode.toUtf8());
}


const QString abt_transaction_base::getRemoteBranchId() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBranchId(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteBranchId(const QString &BranchId)
{
	AB_Transaction_SetRemoteBranchId(this->aqb_transaction,
					 BranchId.toUtf8());
}


const QString abt_transaction_base::getRemoteAccountNumber() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteAccountNumber(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteAccountNumber(const QString &AccountNumber)
{
	AB_Transaction_SetRemoteAccountNumber(this->aqb_transaction,
					      AccountNumber.toUtf8());
}


const QString abt_transaction_base::getRemoteSuffix() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteSuffix(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteSuffix(const QString &Suffix)
{
	AB_Transaction_SetRemoteSuffix(this->aqb_transaction,
				       Suffix.toUtf8());
}


const QString abt_transaction_base::getRemoteIban() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteIban(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteIban(const QString &Iban)
{
	AB_Transaction_SetRemoteIban(this->aqb_transaction,
				     Iban.toUtf8());
}


/** \todo REMOTE_NAME
	  HIER NOCH DIE FUNKTIONEN FÜR REMOTE_NAME EINFÜGEN!
*/


const QString abt_transaction_base::getRemoteBic() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBic(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteBic(const QString &Bic)
{
	AB_Transaction_SetRemoteBic(this->aqb_transaction,
				    Bic.toUtf8());
}


/**************************
 * Dates		  *
 **************************/

const QDate abt_transaction_base::getValutaDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetValutaDate(this->aqb_transaction);

	return this->GwenTimeToQDate(gwen_date);
}

void abt_transaction_base::setValutaDate(const QDate &ValutaDate)
{
	AB_Transaction_SetValutaDate(this->aqb_transaction,
				     this->QDateToGwenTime(ValutaDate));
}

const QDate abt_transaction_base::getDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetDate(this->aqb_transaction);

	return this->GwenTimeToQDate(gwen_date);
}

void abt_transaction_base::setDate(const QDate &Date)
{
	AB_Transaction_SetDate(this->aqb_transaction,
			       this->QDateToGwenTime(Date));
}


/**************************
 * Value		  *
 **************************/

/** \todo AB_VALUE in eine sparate Klasse kapseln, damit es auch in anderen
	  Qt Anwenungen verwendet werden kann und + - * / auf Objecte von
	  abt_value angewendet werden können.
*/

const AB_VALUE *abt_transaction_base::getValue() const
{
	return AB_Transaction_GetValue(this->aqb_transaction);
}

void abt_transaction_base::setValue(const AB_VALUE *Value)
{
	AB_Transaction_SetValue(this->aqb_transaction,Value);
}


/***********************************************
 * Info which is not supported by all backends *
 ***********************************************/
/* This group contains information which differ between backends.
 * Some of this information might not even be supported by every backends.
 */
int abt_transaction_base::getTextKey() const
{
	return AB_Transaction_GetTextKey(this->aqb_transaction);
}

void abt_transaction_base::setTextKey(int TextKey)
{
	AB_Transaction_SetTextKey(this->aqb_transaction, TextKey);
}

int abt_transaction_base::getTextKeyExt() const
{
	return AB_Transaction_GetTextKeyExt(this->aqb_transaction);
}

void abt_transaction_base::setTextKeyExt(int TextKeyExt)
{
	AB_Transaction_SetTextKeyExt(this->aqb_transaction, TextKeyExt);
}


const QString abt_transaction_base::getTransactionKey() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetTransactionKey(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setTransactionKey(const QString &TransactionKey)
{
	AB_Transaction_SetTransactionKey(this->aqb_transaction,
					 TransactionKey.toUtf8());
}


const QString abt_transaction_base::getCustomerReference() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetCustomerReference(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setCustomerReference(const QString &CustomerReference)
{
	AB_Transaction_SetCustomerReference(this->aqb_transaction,
					    CustomerReference.toUtf8());
}


const QString abt_transaction_base::getBankReference() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetBankReference(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setBankReference(const QString &BankReference)
{
	AB_Transaction_SetBankReference(this->aqb_transaction,
					BankReference.toUtf8());
}


const QString abt_transaction_base::getEndToEndReference() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetEndToEndReference(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setEndToEndReference(const QString &EndToEndReference)
{
	AB_Transaction_SetEndToEndReference(this->aqb_transaction,
					    EndToEndReference.toUtf8());
}


const QString abt_transaction_base::getMandateReference() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetMandateReference(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setMandateReference(const QString &MandateReference)
{
	AB_Transaction_SetMandateReference(this->aqb_transaction,
					   MandateReference.toUtf8());
}


const QString abt_transaction_base::getCreditorIdentifier() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetCreditorIdentifier(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setCreditorIdentifier(const QString &CreditorIdentifier)
{
	AB_Transaction_SetCreditorIdentifier(this->aqb_transaction,
					     CreditorIdentifier.toUtf8());
}


const QString abt_transaction_base::getOriginatorIdentifier() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetOriginatorIdentifier(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setOriginatorIdentifier(const QString &OriginatorIdentifier)
{
	AB_Transaction_SetOriginatorIdentifier(this->aqb_transaction,
					       OriginatorIdentifier.toUtf8());
}


int abt_transaction_base::getTransactionCode() const
{		
	return AB_Transaction_GetTransactionCode(this->aqb_transaction);
}

void abt_transaction_base::setTransactionCode(int TransactionCode)
{
	AB_Transaction_SetTransactionCode(this->aqb_transaction,
					  TransactionCode);
}


const QString abt_transaction_base::getTransactionText() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetTransactionText(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setTransactionText(const QString &TransactionText)
{
	AB_Transaction_SetTransactionText(this->aqb_transaction,
					  TransactionText.toUtf8());
}


const QString abt_transaction_base::getPrimanota() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetPrimanota(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setPrimanota(const QString &Primanota)
{
	AB_Transaction_SetPrimanota(this->aqb_transaction,
				    Primanota.toUtf8());
}


const QString abt_transaction_base::getFiId() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetFiId(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setFiId(const QString &FiId)
{
	AB_Transaction_SetFiId(this->aqb_transaction,
			       FiId.toUtf8());
}


/** \todo GWEN_STRINGLIST to QStringList muss implementiert werden */

/** \todo PURPOSE Implementieren
	  HIER NOCH DIE FUNKTIONEN FÜR PURPOSE EINFÜGEN!
*/

/** \todo Category Implementieren
	  HIER NOCH DIE FUNKTIONEN FÜR CATEGORY EINFÜGEN!
*/

/**********************************
 * Additional for Standing Orders *
 **********************************/
/* This group contains information which is used with standing orders.
 * It is not needed for other usage of this type.
 */
AB_TRANSACTION_PERIOD abt_transaction_base::getPeriod() const
{
	return AB_Transaction_GetPeriod(this->aqb_transaction);
}

void abt_transaction_base::setPeriod(AB_TRANSACTION_PERIOD Period)
{
	AB_Transaction_SetPeriod(this->aqb_transaction, Period);
}


int abt_transaction_base::getCycle() const
{
	return AB_Transaction_GetCycle(this->aqb_transaction);
}

void abt_transaction_base::setCycle(int Cycle)
{
	AB_Transaction_SetCycle(this->aqb_transaction, Cycle);
}

int abt_transaction_base::getExecutionDay() const
{
	return AB_Transaction_GetExecutionDay(this->aqb_transaction);
}

void abt_transaction_base::setExecutionDay(int ExecutionDay)
{
	AB_Transaction_SetExecutionDay(this->aqb_transaction, ExecutionDay);
}


const QDate abt_transaction_base::getFirstExecutionDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetFirstExecutionDate(this->aqb_transaction);

	return this->GwenTimeToQDate(gwen_date);
}

void abt_transaction_base::setFirstExecutionDate(const QDate &Date)
{
	AB_Transaction_SetFirstExecutionDate(this->aqb_transaction,
					     this->QDateToGwenTime(Date));
}


const QDate abt_transaction_base::getLastExecutionDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetLastExecutionDate(this->aqb_transaction);

	return this->GwenTimeToQDate(gwen_date);
}

void abt_transaction_base::setLastExecutionDate(const QDate &Date)
{
	AB_Transaction_SetLastExecutionDate(this->aqb_transaction,
					    this->QDateToGwenTime(Date));
}


const QDate abt_transaction_base::getNextExecutionDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetNextExecutionDate(this->aqb_transaction);

	return this->GwenTimeToQDate(gwen_date);
}

void abt_transaction_base::setNextExecutionDate(const QDate &Date)
{
	AB_Transaction_SetNextExecutionDate(this->aqb_transaction,
					    this->QDateToGwenTime(Date));
}


/************************************
 * Additional for Transfers	    *
 ************************************/
/* This group contains information which is used with all kinds of
 * transfers. It is setup by the function AB_Banking_GatherResponses
 * for transfers but not used by AqBanking otherwise.
 */
AB_TRANSACTION_TYPE abt_transaction_base::getType() const
{
	return AB_Transaction_GetType(this->aqb_transaction);
}

void abt_transaction_base::setType(AB_TRANSACTION_TYPE Type)
{
	AB_Transaction_SetType(this->aqb_transaction, Type);
}


AB_TRANSACTION_SUBTYPE abt_transaction_base::getSubType() const
{
	return AB_Transaction_GetSubType(this->aqb_transaction);
}
void abt_transaction_base::setSubType(AB_TRANSACTION_SUBTYPE SubType)
{
	AB_Transaction_SetSubType(this->aqb_transaction, SubType);
}


AB_TRANSACTION_STATUS abt_transaction_base::getStatus() const
{
	return AB_Transaction_GetStatus(this->aqb_transaction);
}

void abt_transaction_base::setStatus(AB_TRANSACTION_STATUS Status)
{
	AB_Transaction_SetStatus(this->aqb_transaction, Status);
}


AB_TRANSACTION_CHARGE abt_transaction_base::getCharge() const
{
	return AB_Transaction_GetCharge(this->aqb_transaction);
}

void abt_transaction_base::setCharge(AB_TRANSACTION_CHARGE Charge)
{
	AB_Transaction_SetCharge(this->aqb_transaction, Charge);
}


/************************************
 * Additional for Foreign Transfers *
 ************************************/
/* This group contains information which is used with transfers to
 * other countries in the world. It is used by backends and
 * applications but not by AqBanking itself.
 */

const QString abt_transaction_base::getRemoteAddrStreet() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteAddrStreet(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteAddrStreet(const QString &RemoteAddrStreet)
{
	AB_Transaction_SetRemoteAddrStreet(this->aqb_transaction,
					   RemoteAddrStreet.toUtf8());
}


const QString abt_transaction_base::getRemoteAddrZipcode() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteAddrZipcode(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteAddrZipcode(const QString &RemoteAddrZipcode)
{
	AB_Transaction_SetRemoteAddrZipcode(this->aqb_transaction,
					    RemoteAddrZipcode.toUtf8());
}


const QString abt_transaction_base::getRemoteAddrCity() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteAddrCity(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemoteAddrCity(const QString &RemoteAddrCity)
{
	AB_Transaction_SetRemoteAddrCity(this->aqb_transaction,
					 RemoteAddrCity.toUtf8());
}


const QString abt_transaction_base::getRemotePhone() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemotePhone(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setRemotePhone(const QString &RemotePhone)
{
	AB_Transaction_SetRemotePhone(this->aqb_transaction,
				      RemotePhone.toUtf8());
}


/***************************************
 * Additional for Investment Transfers *
 ***************************************/
/* This group contains information which is used with investment/stock
 * transfers. It is used by backends and applications but not by
 * AqBanking itself.
 */

const QString abt_transaction_base::getUnitId() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetUnitId(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setUnitId(const QString &UnitId)
{
	AB_Transaction_SetUnitId(this->aqb_transaction, UnitId.toUtf8());
}


const QString abt_transaction_base::getUnitIdNameSpace() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetUnitIdNameSpace(this->aqb_transaction));
	return ret;
}

void abt_transaction_base::setUnitIdNameSpacet(const QString &UnitIdNameSpace)
{
	AB_Transaction_SetUnitIdNameSpace(this->aqb_transaction,
					  UnitIdNameSpace.toUtf8());
}


const AB_VALUE* abt_transaction_base::getUnits() const
{
	return AB_Transaction_GetUnits(this->aqb_transaction);
}

void abt_transaction_base::setUnits(const AB_VALUE *Units)
{
	AB_Transaction_SetUnits(this->aqb_transaction, Units);
}


const AB_VALUE* abt_transaction_base::getUnitPrice() const
{
	return AB_Transaction_GetUnitPrice(this->aqb_transaction);
}

void abt_transaction_base::setUnitPrice(const AB_VALUE *UnitPrice)
{
	AB_Transaction_SetUnitPrice(this->aqb_transaction, UnitPrice);
}


const AB_VALUE* abt_transaction_base::getCommission() const
{
	return AB_Transaction_GetCommission(this->aqb_transaction);
}

void abt_transaction_base::setCommission(const AB_VALUE *Commission)
{
	AB_Transaction_SetCommission(this->aqb_transaction, Commission);
}















