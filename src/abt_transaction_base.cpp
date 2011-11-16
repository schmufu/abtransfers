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
#include <QStringList>
#include <QString>
#include <QSettings>
#include <QDebug>

#include "globalvars.h"
#include "abt_conv.h"


abt_transaction::abt_transaction(AB_TRANSACTION *t, bool freeOnDelete)
{
	if (t == NULL) {
		//Keine Transaction übergeben, wir erstellen uns selber eine
		this->aqb_transaction = AB_Transaction_new();
		//Diese muss zum Schluss durch uns auch wieder gelöscht werden
		this->FreeTransactionOnDelete = true;
	} else {
		//Übergebene Transaction nutzen
		this->aqb_transaction = t;
		//Löschen nur auf Aufforderung (default: false)
		this->FreeTransactionOnDelete = freeOnDelete;
	}
}

//copy constructor
abt_transaction::abt_transaction(const abt_transaction &abt_t)
{
	AB_TRANSACTION *t = AB_Transaction_dup(abt_t.getAB_Transaction());
	this->aqb_transaction = t;
	//Wir haben die transaction neu erstellt, also müssen wir sie auch wieder löschen
	this->FreeTransactionOnDelete = true;
}

abt_transaction::~abt_transaction()
{
	if (this->FreeTransactionOnDelete) {
		AB_Transaction_free(this->aqb_transaction);
	}
}


/**
 * Die Werte der Transaction \a t werden in dem QSettings Objekt gespeichert.
 * Die eingestellte Group von \a s wird nicht verändert!
 */
//static
void abt_transaction::saveTransaction(AB_TRANSACTION *t, QSettings *s)
{
	//erst ein Object mit der übergebenen AB_TRANSACTION erstellen
	const abt_transaction *trans = new abt_transaction(t, false);
	//und dieses dann speichern
	abt_transaction::saveTransaction(trans, s);
	//und wieder freigeben (AB_TRANSACTION bleibt erhalten, create mit "false")
	delete trans;
}


/**
 * Die Werte der Transaction \a t werden in dem QSettings Objekt gespeichert.
 * Die eingestellte Group von \a s wird nicht verändert!
 */
//static
void abt_transaction::saveTransaction(const abt_transaction *t, QSettings *s)
{
	const AB_VALUE *v;

	s->setValue("LocalCountry", t->getLocalCountry());
	s->setValue("LocalBankCode", t->getLocalBankCode() );
	s->setValue("LocalBranchId", t->getLocalBranchId() );
	s->setValue("LocalAccountNumber", t->getLocalAccountNumber() );
	s->setValue("LocalSuffix", t->getLocalSuffix() );
	s->setValue("LocalIban", t->getLocalIban() );
	s->setValue("LocalName", t->getLocalName() );
	s->setValue("LocalBic", t->getLocalBic() );
	s->setValue("RemoteCountry", t->getRemoteCountry() );
	s->setValue("RemoteBankName", t->getRemoteBankName() );
	s->setValue("RemoteBankLocation", t->getRemoteBankLocation() );
	s->setValue("RemoteBankCode", t->getRemoteBankCode() );
	s->setValue("RemoteBranchId", t->getRemoteBranchId() );
	s->setValue("RemoteAccountNumber", t->getRemoteAccountNumber() );
	s->setValue("RemoteSuffix", t->getRemoteSuffix() );
	s->setValue("RemoteIban", t->getRemoteIban() );
	s->setValue("RemoteName", t->getRemoteName() );
	s->setValue("RemoteBic", t->getRemoteBic() );
	s->setValue("ValutaDate", t->getValutaDate() );
	s->setValue("Date", t->getDate() );

	v = t->getValue();
	s->setValue("Value", abt_conv::ABValueToString(v));
	if (v) s->setValue("ValueCurrency", AB_Value_GetCurrency(v));

	s->setValue("TextKey", t->getTextKey() );
	s->setValue("TextKeyExt", t->getTextKeyExt() );
	s->setValue("TransactionKey", t->getTransactionKey() );
	s->setValue("CustomerReference", t->getCustomerReference() );
	s->setValue("BankReference", t->getBankReference() );
	s->setValue("EndToEndReference", t->getEndToEndReference() );
	s->setValue("MandateReference", t->getMandateReference() );
	s->setValue("CretitorIdentifier", t->getCreditorIdentifier() );
	s->setValue("OriginatorIdentifier", t->getOriginatorIdentifier() );
	s->setValue("TransactionCode", t->getTransactionCode() );
	s->setValue("TransactionText", t->getTransactionText() );
	s->setValue("Primanota", t->getPrimanota() );
	s->setValue("FiId", t->getFiId() );
	s->setValue("Purpose", t->getPurpose() );
	s->setValue("Category", t->getCategory() );
	s->setValue("Period", t->getPeriod() );
	s->setValue("Cycle", t->getCycle() );
	s->setValue("ExecutionDay", t->getExecutionDay() );
	s->setValue("FirstExecutionDate", t->getFirstExecutionDate() );
	s->setValue("LastExecutionDate", t->getLastExecutionDate() );
	s->setValue("NextExecutionDate", t->getNextExecutionDate() );
	s->setValue("Type", t->getType() );
	s->setValue("SubType", t->getSubType() );
	s->setValue("Status", t->getStatus() );
	s->setValue("Charge", t->getCharge() );
	s->setValue("RemoteAddrStreet", t->getRemoteAddrStreet() );
	s->setValue("RemoteAddrZipcode", t->getRemoteAddrZipcode() );
	s->setValue("RemoteAddrCity", t->getRemoteAddrCity() );
	s->setValue("RemotePhone", t->getRemotePhone() );
	s->setValue("UnitId", t->getUnitId() );
	s->setValue("UnitIdNameSpace", t->getUnitIdNameSpace() );

	v = t->getUnits();
	s->setValue("Units", abt_conv::ABValueToString(v));
	if (v) s->setValue("UnitsCurrency", AB_Value_GetCurrency(v));

	v = t->getUnitPrice();
	s->setValue("UnitPrice", abt_conv::ABValueToString(v));
	if (v) s->setValue("UnitPriceCurrency", AB_Value_GetCurrency(v));

	v = t->getCommission();
	s->setValue("Commission", abt_conv::ABValueToString(v));
	if (v) s->setValue("CommissionCurrency", AB_Value_GetCurrency(v));

	s->setValue("UniqueId", t->getUniqueId() );
	s->setValue("IdForApplication", t->getIdForApplication() );
	s->setValue("GroupId", t->getGroupId() );

	v = t->getFees();
	s->setValue("Fees", abt_conv::ABValueToString(v));
	if (v) s->setValue("FeesCurrency", AB_Value_GetCurrency(v));

}


/** Das übergebene Settings-Object muss schon auf die entsprechende Transaction
 *  eingestellt sein!
 *
 * Es werden alle Values des Settings-Objekts der zurückgegebenen Transaction
 * zugewiesen.
 */
//static
abt_transaction* abt_transaction::loadTransaction(const QSettings *s)
{
	Q_ASSERT(s);

	abt_transaction *t = new abt_transaction();
	AB_VALUE *v;

	t->setLocalCountry(s->value("LocalCountry").toString());
	t->setLocalBankCode(s->value("LocalBankCode").toString());
	t->setLocalBranchId(s->value("LocalBranchId").toString());
	t->setLocalAccountNumber(s->value("LocalAccountNumber").toString() );
	t->setLocalSuffix(s->value("LocalSuffix").toString() );
	t->setLocalIban(s->value("LocalIban").toString() );
	t->setLocalName(s->value("LocalName").toString() );
	t->setLocalBic(s->value("LocalBic").toString() );
	t->setRemoteCountry(s->value("RemoteCountry").toString() );
	t->setRemoteBankName(s->value("RemoteBankName").toString() );
	t->setRemoteBankLocation(s->value("RemoteBankLocation").toString() );
	t->setRemoteBankCode(s->value("RemoteBankCode").toString() );
	t->setRemoteBranchId(s->value("RemoteBranchId").toString() );
	t->setRemoteAccountNumber(s->value("RemoteAccountNumber").toString() );
	t->setRemoteSuffix(s->value("RemoteSuffix").toString() );
	t->setRemoteIban(s->value("RemoteIban").toString() );
	t->setRemoteName(s->value("RemoteName").toStringList() );
	t->setRemoteBic(s->value("RemoteBic").toString() );
	t->setValutaDate(s->value("ValutaDate").toDate() );
	t->setDate(s->value("Date").toDate() );

	v = abt_conv::ABValueFromString(s->value("Value").toString());
	if (v) AB_Value_SetCurrency(v,s->value("ValueCurrency").toString().toStdString().c_str());
	t->setValue(v);

	t->setTextKey(s->value("TextKey").toInt() );
	t->setTextKeyExt(s->value("TextKeyExt").toInt() );
	t->setTransactionKey(s->value("TransactionKey").toString() );
	t->setCustomerReference(s->value("CustomerReference").toString() );
	t->setBankReference(s->value("BankReference").toString() );
	t->setEndToEndReference(s->value("EndToEndReference").toString() );
	t->setMandateReference(s->value("MandateReference").toString() );
	t->setCreditorIdentifier(s->value("CretitorIdentifier").toString() );
	t->setOriginatorIdentifier(s->value("OriginatorIdentifier").toString() );
	t->setTransactionCode(s->value("TransactionCode").toInt() );
	t->setTransactionText(s->value("TransactionText").toString() );
	t->setPrimanota(s->value("Primanota").toString() );
	t->setFiId(s->value("FiId").toString() );
	t->setPurpose(s->value("Purpose").toStringList() );
	t->setCategory(s->value("Category").toStringList() );
	t->setPeriod((AB_TRANSACTION_PERIOD)s->value("Period").toInt() );
	t->setCycle(s->value("Cycle").toInt() );
	t->setExecutionDay(s->value("ExecutionDay").toInt() );
	t->setFirstExecutionDate(s->value("FirstExecutionDate").toDate() );
	t->setLastExecutionDate(s->value("LastExecutionDate").toDate() );
	t->setNextExecutionDate(s->value("NextExecutionDate").toDate() );
	t->setType((AB_TRANSACTION_TYPE)s->value("Type").toInt() );
	t->setSubType((AB_TRANSACTION_SUBTYPE)s->value("SubType").toInt() );
	t->setStatus((AB_TRANSACTION_STATUS)s->value("Status").toInt() );
	t->setCharge((AB_TRANSACTION_CHARGE)s->value("Charge").toInt() );
	t->setRemoteAddrStreet(s->value("RemoteAddrStreet").toString() );
	t->setRemoteAddrZipcode(s->value("RemoteAddrZipcode").toString() );
	t->setRemoteAddrCity(s->value("RemoteAddrCity").toString() );
	t->setRemotePhone(s->value("RemotePhone").toString() );
	t->setUnitId(s->value("UnitId").toString() );
	t->setUnitIdNameSpace(s->value("UnitIdNameSpace").toString() );

	v = abt_conv::ABValueFromString(s->value("Units").toString());
	if (v) AB_Value_SetCurrency(v,s->value("UnitsCurrency").toString().toStdString().c_str());
	t->setUnits(v);

	v = abt_conv::ABValueFromString(s->value("UnitPrice").toString());
	if (v) AB_Value_SetCurrency(v,s->value("UnitPriceCurrency").toString().toStdString().c_str());
	t->setUnitPrice(v);

	v = abt_conv::ABValueFromString(s->value("Commission").toString());
	if (v) AB_Value_SetCurrency(v,s->value("CommissionCurrency").toString().toStdString().c_str());
	t->setCommission(v);

	t->setUniqueId(s->value("UniqueId").toUInt() );
	t->setIdForApplication(s->value("IdForApplication").toUInt() );
	t->setGroupId(s->value("GroupId").toUInt() );

	v = abt_conv::ABValueFromString(s->value("Fees").toString());
	if (v) AB_Value_SetCurrency(v,s->value("FeesCurrency").toString().toStdString().c_str());
	t->setFees(abt_conv::ABValueFromString(s->value("Fees").toString()) );

	return t;
}

/*****************************************************************************
 * helper functions                                                          *
 *****************************************************************************/
bool abt_transaction::isModified() const
{
	return (AB_Transaction_IsModified(this->aqb_transaction) == 0);
}

void abt_transaction::setModified(bool mod)
{
	int m;
	m = (mod) ? 1 : 0;

	AB_Transaction_SetModified(this->aqb_transaction, mod);
}

void abt_transaction::fillLocalFromAccount(const AB_ACCOUNT *a)
{
	AB_Transaction_FillLocalFromAccount(this->aqb_transaction, a);
}

const AB_TRANSACTION* abt_transaction::getAB_Transaction() const
{
	return this->aqb_transaction;
}

/**************************
 * Local Account Info     *
 **************************/

const QString abt_transaction::getLocalCountry() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalCountry(this->aqb_transaction));
	return ret;
}

void abt_transaction::setLocalCountry(const QString &Country)
{
	AB_Transaction_SetLocalCountry(this->aqb_transaction,
				       Country.toUtf8());
}


const QString abt_transaction::getLocalBankCode() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalBankCode(this->aqb_transaction));
	return ret;
}

void abt_transaction::setLocalBankCode(const QString &BankCode)
{
	AB_Transaction_SetLocalBankCode(this->aqb_transaction,
					BankCode.toUtf8());
}


const QString abt_transaction::getLocalBranchId() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalBranchId(this->aqb_transaction));
	return ret;
}

void abt_transaction::setLocalBranchId(const QString &BranchId)
{
	AB_Transaction_SetLocalBranchId(this->aqb_transaction,
					BranchId.toUtf8());
}


const QString abt_transaction::getLocalAccountNumber() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalAccountNumber(this->aqb_transaction));
	return ret;
}

void abt_transaction::setLocalAccountNumber(const QString &AccountNumber)
{
	AB_Transaction_SetLocalAccountNumber(this->aqb_transaction,
					     AccountNumber.toUtf8());
}


const QString abt_transaction::getLocalSuffix() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalSuffix(this->aqb_transaction));
	return ret;
}

void abt_transaction::setLocalSuffix(const QString &Suffix)
{
	AB_Transaction_SetLocalSuffix(this->aqb_transaction,
				      Suffix.toUtf8());
}


const QString abt_transaction::getLocalIban() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalIban(this->aqb_transaction));
	return ret;
}

void abt_transaction::setLocalIban(const QString &Iban)
{
	AB_Transaction_SetLocalIban(this->aqb_transaction,
				    Iban.toUtf8());
}


const QString abt_transaction::getLocalName() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalName(this->aqb_transaction));
	return ret;
}

void abt_transaction::setLocalName(const QString &Name)
{
	AB_Transaction_SetLocalName(this->aqb_transaction,
				    Name.toUtf8());
}


const QString abt_transaction::getLocalBic() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetLocalBic(this->aqb_transaction));
	return ret;
}

void abt_transaction::setLocalBic(const QString &Bic)
{
	AB_Transaction_SetLocalBic(this->aqb_transaction,
				   Bic.toUtf8());
}


/**************************
 * Remote Account Info    *
 **************************/

const QString abt_transaction::getRemoteCountry() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteCountry(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteCountry(const QString &Country)
{
	AB_Transaction_SetRemoteCountry(this->aqb_transaction,
					Country.toUtf8());
}


const QString abt_transaction::getRemoteBankName() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBankName(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteBankName(const QString &BankName)
{
	AB_Transaction_SetRemoteBankName(this->aqb_transaction,
					 BankName.toUtf8());
}


const QString abt_transaction::getRemoteBankLocation() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBankLocation(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteBankLocation(const QString &BankLocation)
{
	AB_Transaction_SetRemoteBankLocation(this->aqb_transaction,
					     BankLocation.toUtf8());
}


const QString abt_transaction::getRemoteBankCode() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBankCode(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteBankCode(const QString &BankCode)
{
	AB_Transaction_SetRemoteBankCode(this->aqb_transaction,
					 BankCode.toUtf8());
}


const QString abt_transaction::getRemoteBranchId() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBranchId(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteBranchId(const QString &BranchId)
{
	AB_Transaction_SetRemoteBranchId(this->aqb_transaction,
					 BranchId.toUtf8());
}


const QString abt_transaction::getRemoteAccountNumber() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteAccountNumber(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteAccountNumber(const QString &AccountNumber)
{
	AB_Transaction_SetRemoteAccountNumber(this->aqb_transaction,
					      AccountNumber.toUtf8());
}


const QString abt_transaction::getRemoteSuffix() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteSuffix(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteSuffix(const QString &Suffix)
{
	AB_Transaction_SetRemoteSuffix(this->aqb_transaction,
				       Suffix.toUtf8());
}


const QString abt_transaction::getRemoteIban() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteIban(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteIban(const QString &Iban)
{
	AB_Transaction_SetRemoteIban(this->aqb_transaction,
				     Iban.toUtf8());
}

const QStringList abt_transaction::getRemoteName() const
{
	return abt_conv::GwenStringListToQStringList(
			AB_Transaction_GetRemoteName(this->aqb_transaction));
}

void abt_transaction::setRemoteName(const QStringList &Name)
{
	const GWEN_STRINGLIST *gwl = abt_conv::QStringListToGwenStringList(Name);

	AB_Transaction_SetRemoteName(this->aqb_transaction, gwl);
}



const QString abt_transaction::getRemoteBic() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteBic(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteBic(const QString &Bic)
{
	AB_Transaction_SetRemoteBic(this->aqb_transaction,
				    Bic.toUtf8());
}


/**************************
 * Dates		  *
 **************************/

const QDate abt_transaction::getValutaDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetValutaDate(this->aqb_transaction);

	return abt_conv::GwenTimeToQDate(gwen_date);
}

void abt_transaction::setValutaDate(const QDate &ValutaDate)
{
	AB_Transaction_SetValutaDate(this->aqb_transaction,
				     abt_conv::QDateToGwenTime(ValutaDate));
}

const QDate abt_transaction::getDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetDate(this->aqb_transaction);

	return abt_conv::GwenTimeToQDate(gwen_date);
}

void abt_transaction::setDate(const QDate &Date)
{
	AB_Transaction_SetDate(this->aqb_transaction,
			       abt_conv::QDateToGwenTime(Date));
}


/**************************
 * Value		  *
 **************************/

/** \todo AB_VALUE in eine sparate Klasse kapseln, damit es auch in anderen
	  Qt Anwenungen verwendet werden kann und + - * / auf Objecte von
	  abt_value angewendet werden können.
*/

const AB_VALUE *abt_transaction::getValue() const
{
	return AB_Transaction_GetValue(this->aqb_transaction);
}

void abt_transaction::setValue(const AB_VALUE *Value)
{
	AB_Transaction_SetValue(this->aqb_transaction,Value);
}


/***********************************************
 * Info which is not supported by all backends *
 ***********************************************/
/* This group contains information which differ between backends.
 * Some of this information might not even be supported by every backends.
 */
int abt_transaction::getTextKey() const
{
	return AB_Transaction_GetTextKey(this->aqb_transaction);
}

void abt_transaction::setTextKey(int TextKey)
{
	AB_Transaction_SetTextKey(this->aqb_transaction, TextKey);
}

int abt_transaction::getTextKeyExt() const
{
	return AB_Transaction_GetTextKeyExt(this->aqb_transaction);
}

void abt_transaction::setTextKeyExt(int TextKeyExt)
{
	AB_Transaction_SetTextKeyExt(this->aqb_transaction, TextKeyExt);
}


const QString abt_transaction::getTransactionKey() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetTransactionKey(this->aqb_transaction));
	return ret;
}

void abt_transaction::setTransactionKey(const QString &TransactionKey)
{
	AB_Transaction_SetTransactionKey(this->aqb_transaction,
					 TransactionKey.toUtf8());
}


const QString abt_transaction::getCustomerReference() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetCustomerReference(this->aqb_transaction));
	return ret;
}

void abt_transaction::setCustomerReference(const QString &CustomerReference)
{
	AB_Transaction_SetCustomerReference(this->aqb_transaction,
					    CustomerReference.toUtf8());
}


const QString abt_transaction::getBankReference() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetBankReference(this->aqb_transaction));
	return ret;
}

void abt_transaction::setBankReference(const QString &BankReference)
{
	AB_Transaction_SetBankReference(this->aqb_transaction,
					BankReference.toUtf8());
}


const QString abt_transaction::getEndToEndReference() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetEndToEndReference(this->aqb_transaction));
	return ret;
}

void abt_transaction::setEndToEndReference(const QString &EndToEndReference)
{
	AB_Transaction_SetEndToEndReference(this->aqb_transaction,
					    EndToEndReference.toUtf8());
}


const QString abt_transaction::getMandateReference() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetMandateReference(this->aqb_transaction));
	return ret;
}

void abt_transaction::setMandateReference(const QString &MandateReference)
{
	AB_Transaction_SetMandateReference(this->aqb_transaction,
					   MandateReference.toUtf8());
}


const QString abt_transaction::getCreditorIdentifier() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetCreditorIdentifier(this->aqb_transaction));
	return ret;
}

void abt_transaction::setCreditorIdentifier(const QString &CreditorIdentifier)
{
	AB_Transaction_SetCreditorIdentifier(this->aqb_transaction,
					     CreditorIdentifier.toUtf8());
}


const QString abt_transaction::getOriginatorIdentifier() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetOriginatorIdentifier(this->aqb_transaction));
	return ret;
}

void abt_transaction::setOriginatorIdentifier(const QString &OriginatorIdentifier)
{
	AB_Transaction_SetOriginatorIdentifier(this->aqb_transaction,
					       OriginatorIdentifier.toUtf8());
}


int abt_transaction::getTransactionCode() const
{		
	return AB_Transaction_GetTransactionCode(this->aqb_transaction);
}

void abt_transaction::setTransactionCode(int TransactionCode)
{
	AB_Transaction_SetTransactionCode(this->aqb_transaction,
					  TransactionCode);
}


const QString abt_transaction::getTransactionText() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetTransactionText(this->aqb_transaction));
	return ret;
}

void abt_transaction::setTransactionText(const QString &TransactionText)
{
	AB_Transaction_SetTransactionText(this->aqb_transaction,
					  TransactionText.toUtf8());
}


const QString abt_transaction::getPrimanota() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetPrimanota(this->aqb_transaction));
	return ret;
}

void abt_transaction::setPrimanota(const QString &Primanota)
{
	AB_Transaction_SetPrimanota(this->aqb_transaction,
				    Primanota.toUtf8());
}


const QString abt_transaction::getFiId() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetFiId(this->aqb_transaction));
	return ret;
}

void abt_transaction::setFiId(const QString &FiId)
{
	AB_Transaction_SetFiId(this->aqb_transaction,
			       FiId.toUtf8());
}

const QStringList abt_transaction::getPurpose() const
{
	return abt_conv::GwenStringListToQStringList(
			AB_Transaction_GetPurpose(this->aqb_transaction));
}

void abt_transaction::setPurpose(const QStringList &Purpose)
{
	const GWEN_STRINGLIST *gwl = abt_conv::QStringListToGwenStringList(Purpose);

	AB_Transaction_SetPurpose(this->aqb_transaction, gwl);
}

const QStringList abt_transaction::getCategory() const
{
	return abt_conv::GwenStringListToQStringList(
			AB_Transaction_GetCategory(this->aqb_transaction));
}

void abt_transaction::setCategory(const QStringList &Category)
{
	const GWEN_STRINGLIST *gwl = abt_conv::QStringListToGwenStringList(Category);

	AB_Transaction_SetCategory(this->aqb_transaction, gwl);
}

/**********************************
 * Additional for Standing Orders *
 **********************************/
/* This group contains information which is used with standing orders.
 * It is not needed for other usage of this type.
 */
AB_TRANSACTION_PERIOD abt_transaction::getPeriod() const
{
	return AB_Transaction_GetPeriod(this->aqb_transaction);
}

void abt_transaction::setPeriod(AB_TRANSACTION_PERIOD Period)
{
	AB_Transaction_SetPeriod(this->aqb_transaction, Period);
}


int abt_transaction::getCycle() const
{
	return AB_Transaction_GetCycle(this->aqb_transaction);
}

void abt_transaction::setCycle(int Cycle)
{
	AB_Transaction_SetCycle(this->aqb_transaction, Cycle);
}

int abt_transaction::getExecutionDay() const
{
	return AB_Transaction_GetExecutionDay(this->aqb_transaction);
}

void abt_transaction::setExecutionDay(int ExecutionDay)
{
	AB_Transaction_SetExecutionDay(this->aqb_transaction, ExecutionDay);
}


const QDate abt_transaction::getFirstExecutionDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetFirstExecutionDate(this->aqb_transaction);

	return abt_conv::GwenTimeToQDate(gwen_date);
}

void abt_transaction::setFirstExecutionDate(const QDate &Date)
{
	AB_Transaction_SetFirstExecutionDate(this->aqb_transaction,
					     abt_conv::QDateToGwenTime(Date));
}


const QDate abt_transaction::getLastExecutionDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetLastExecutionDate(this->aqb_transaction);

	return abt_conv::GwenTimeToQDate(gwen_date);
}

void abt_transaction::setLastExecutionDate(const QDate &Date)
{
	AB_Transaction_SetLastExecutionDate(this->aqb_transaction,
					    abt_conv::QDateToGwenTime(Date));
}


const QDate abt_transaction::getNextExecutionDate() const
{
	const GWEN_TIME *gwen_date;
	gwen_date = AB_Transaction_GetNextExecutionDate(this->aqb_transaction);

	return abt_conv::GwenTimeToQDate(gwen_date);
}

void abt_transaction::setNextExecutionDate(const QDate &Date)
{
	AB_Transaction_SetNextExecutionDate(this->aqb_transaction,
					    abt_conv::QDateToGwenTime(Date));
}


/************************************
 * Additional for Transfers	    *
 ************************************/
/* This group contains information which is used with all kinds of
 * transfers. It is setup by the function AB_Banking_GatherResponses
 * for transfers but not used by AqBanking otherwise.
 */
AB_TRANSACTION_TYPE abt_transaction::getType() const
{
	return AB_Transaction_GetType(this->aqb_transaction);
}

void abt_transaction::setType(AB_TRANSACTION_TYPE Type)
{
	AB_Transaction_SetType(this->aqb_transaction, Type);
}


AB_TRANSACTION_SUBTYPE abt_transaction::getSubType() const
{
	return AB_Transaction_GetSubType(this->aqb_transaction);
}
void abt_transaction::setSubType(AB_TRANSACTION_SUBTYPE SubType)
{
	AB_Transaction_SetSubType(this->aqb_transaction, SubType);
}


AB_TRANSACTION_STATUS abt_transaction::getStatus() const
{
	return AB_Transaction_GetStatus(this->aqb_transaction);
}

void abt_transaction::setStatus(AB_TRANSACTION_STATUS Status)
{
	AB_Transaction_SetStatus(this->aqb_transaction, Status);
}


AB_TRANSACTION_CHARGE abt_transaction::getCharge() const
{
	return AB_Transaction_GetCharge(this->aqb_transaction);
}

void abt_transaction::setCharge(AB_TRANSACTION_CHARGE Charge)
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

const QString abt_transaction::getRemoteAddrStreet() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteAddrStreet(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteAddrStreet(const QString &RemoteAddrStreet)
{
	AB_Transaction_SetRemoteAddrStreet(this->aqb_transaction,
					   RemoteAddrStreet.toUtf8());
}


const QString abt_transaction::getRemoteAddrZipcode() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteAddrZipcode(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteAddrZipcode(const QString &RemoteAddrZipcode)
{
	AB_Transaction_SetRemoteAddrZipcode(this->aqb_transaction,
					    RemoteAddrZipcode.toUtf8());
}


const QString abt_transaction::getRemoteAddrCity() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemoteAddrCity(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemoteAddrCity(const QString &RemoteAddrCity)
{
	AB_Transaction_SetRemoteAddrCity(this->aqb_transaction,
					 RemoteAddrCity.toUtf8());
}


const QString abt_transaction::getRemotePhone() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetRemotePhone(this->aqb_transaction));
	return ret;
}

void abt_transaction::setRemotePhone(const QString &RemotePhone)
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

const QString abt_transaction::getUnitId() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetUnitId(this->aqb_transaction));
	return ret;
}

void abt_transaction::setUnitId(const QString &UnitId)
{
	AB_Transaction_SetUnitId(this->aqb_transaction, UnitId.toUtf8());
}


const QString abt_transaction::getUnitIdNameSpace() const
{
	QString ret;
	ret = QString::fromUtf8(
		AB_Transaction_GetUnitIdNameSpace(this->aqb_transaction));
	return ret;
}

void abt_transaction::setUnitIdNameSpace(const QString &UnitIdNameSpace)
{
	AB_Transaction_SetUnitIdNameSpace(this->aqb_transaction,
					  UnitIdNameSpace.toUtf8());
}


const AB_VALUE* abt_transaction::getUnits() const
{
	return AB_Transaction_GetUnits(this->aqb_transaction);
}

void abt_transaction::setUnits(const AB_VALUE *Units)
{
	AB_Transaction_SetUnits(this->aqb_transaction, Units);
}


const AB_VALUE* abt_transaction::getUnitPrice() const
{
	return AB_Transaction_GetUnitPrice(this->aqb_transaction);
}

void abt_transaction::setUnitPrice(const AB_VALUE *UnitPrice)
{
	AB_Transaction_SetUnitPrice(this->aqb_transaction, UnitPrice);
}


const AB_VALUE* abt_transaction::getCommission() const
{
	return AB_Transaction_GetCommission(this->aqb_transaction);
}

void abt_transaction::setCommission(const AB_VALUE *Commission)
{
	AB_Transaction_SetCommission(this->aqb_transaction, Commission);
}

/***************************************
 * ID functions *
 ***************************************/
quint32 abt_transaction::getUniqueId() const
{
	return AB_Transaction_GetUniqueId(this->aqb_transaction);
}
void abt_transaction::setUniqueId(quint32 id)
{
	AB_Transaction_SetUniqueId(this->aqb_transaction, id);
}

quint32 abt_transaction::getIdForApplication() const
{
	return AB_Transaction_GetIdForApplication(this->aqb_transaction);
}
void abt_transaction::setIdForApplication(quint32 id)
{
	AB_Transaction_SetIdForApplication(this->aqb_transaction, id);
}

quint32 abt_transaction::getGroupId() const
{
	return AB_Transaction_GetGroupId(this->aqb_transaction);
}
void abt_transaction::setGroupId(quint32 id)
{
	AB_Transaction_SetGroupId(this->aqb_transaction, id);
}

const AB_VALUE* abt_transaction::getFees() const
{
	return AB_Transaction_GetFees(this->aqb_transaction);
}
void abt_transaction::setFees(const AB_VALUE* value)
{
	AB_Transaction_SetFees(this->aqb_transaction, value);
}

