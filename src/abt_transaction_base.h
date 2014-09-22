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

#ifndef ABT_TRANSACTION_BASE_H
#define ABT_TRANSACTION_BASE_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QMetaType>

#include <aqbanking/transaction.h>
#include <aqbanking/transactionfns.h>

//#include "aqb_accountinfo.h"

class QSettings;

/** \brief simple wrapper to AB_Transaction_Set* -Get* functions
  *
  * Diese Klasse stellt alle Funktionen von AqBanking die AB_TRANSACTION
  * betreffen zur Verfügung.
  */

class abt_transaction
{
private:
	bool FreeTransactionOnDelete;

protected:
	AB_TRANSACTION* aqb_transaction;
	const AB_TRANSACTION* aqb_transaction_C;

public:
	abt_transaction(AB_TRANSACTION *t = NULL, bool freeOnDelete = false);
	abt_transaction(const AB_TRANSACTION *t);
	abt_transaction(const abt_transaction &abt_t); //copy contructor
	~abt_transaction();

	/*****************************
	 * Local Account Info        *
	 *****************************/
	const QString getLocalCountry() const;
	void setLocalCountry(const QString &Country);

	const QString getLocalBankCode() const;
	void setLocalBankCode(const QString &BankCode);

	const QString getLocalBranchId() const;
	void setLocalBranchId(const QString &BranchId);

	const QString getLocalAccountNumber() const;
	void setLocalAccountNumber(const QString &AccountNumber);

	const QString getLocalSuffix() const;
	void setLocalSuffix(const QString &Suffix);

	const QString getLocalIban() const;
	void setLocalIban(const QString &Iban);

	const QString getLocalName() const;
	void setLocalName(const QString &Name);

	const QString getLocalBic() const;
	void setLocalBic(const QString &Bic);

	/*****************************
	 * Remote Account Info        *
	 *****************************/
	const QString getRemoteCountry() const;
	void setRemoteCountry(const QString &Country);

	const QString getRemoteBankName() const;
	void setRemoteBankName(const QString &BankName);

	const QString getRemoteBankLocation() const;
	void setRemoteBankLocation(const QString &BankLocation);

	const QString getRemoteBankCode() const;
	void setRemoteBankCode(const QString &BankCode);

	const QString getRemoteBranchId() const;
	void setRemoteBranchId(const QString &BranchId);

	const QString getRemoteAccountNumber() const;
	void setRemoteAccountNumber(const QString &AccountNumber);

	const QString getRemoteSuffix() const;
	void setRemoteSuffix(const QString &Suffix);

	const QString getRemoteIban() const;
	void setRemoteIban(const QString &Iban);

	const QStringList getRemoteName() const;
	void setRemoteName(const QStringList &Name);
	//! \todo not implemented yet
	void addRemoteName(const QString &Name, int chk);
	//! \todo not implemented yet
	void removeRemoteName(const QString &Name);
	//! \todo not implemented yet
	void clearRemoteName();
	//! \todo not implemented yet
	bool hasRemoteName(const QString &Name);

	const QString getRemoteBic() const;
	void setRemoteBic(const QString &Bic);

	/*****************************
	 * Dates		     *
	 *****************************/
	//! Wertstellung
	const QDate getValutaDate() const;
	//! Wertstellung
	void setValutaDate(const QDate &ValutaDate);

	//! Buchungsdatum
	const QDate getDate() const;
	//! Buchungsdatum
	void setDate(const QDate &Date);

	/*****************************
	 * Values		     *
	 *****************************/
	const AB_VALUE* getValue() const;
	//const AB::Value* getValue() const;
	void setValue(const AB_VALUE *Value);
	//void setValue(const AB::Value *Value);

	/*********************************
	 * Not supported by all backends *
	 *********************************/
	int getTextKey() const;
	void setTextKey(int TextKey);
	int getTextKeyExt() const;
	void setTextKeyExt(int TextKeyExt);

	const QString getTransactionKey() const;
	void setTransactionKey(const QString &TransactionKey);

	const QString getCustomerReference() const;
	void setCustomerReference(const QString &CustomerReference);

	const QString getBankReference() const;
	void setBankReference(const QString &BankReference);

	const QString getEndToEndReference() const;
	void setEndToEndReference(const QString &EndToEndReference);

	const QString getMandateReference() const;
	void setMandateReference(const QString &MandateReference);

	const QString getCreditorSchemeId() const;
	void setCreditorSchemeId(const QString &CreditorIdentifier);

	const QString getOriginatorIdentifier() const;
	void setOriginatorIdentifier(const QString &OriginatorIdentifier);

	int getTransactionCode() const;
	void setTransactionCode(int TransactionCode);

	const QString getTransactionText() const;
	void setTransactionText(const QString &TransactionText);

	const QString getPrimanota() const;
	void setPrimanota(const QString &Primanota);

	const QString getFiId() const;
	void setFiId(const QString &FiId);

	const QStringList getPurpose() const;
	void setPurpose(const QStringList &Purpose);

	const QStringList getCategory() const;
	void setCategory(const QStringList &Category);
	//! \todo not implemented yet
	void addCategory(const QString &Category, int chk);
	//! \todo not implemented yet
	void removeCategory(const QString &Category);
	//! \todo not implemented yet
	void clearCategory();
	//! \todo not implemented yet
	bool hasCategory(const QString &Category);


	/**********************************
	 * Additional for Standing Orders *
	 **********************************/
	/* This group contains information which is used with standing orders.
	 * It is not needed for other usage of this type.
	 */
	AB_TRANSACTION_PERIOD getPeriod() const;
	void setPeriod(AB_TRANSACTION_PERIOD Period);

	int getCycle() const;
	void setCycle(int Cycle);

	int getExecutionDay() const;
	void setExecutionDay(int ExecutionDay);

	const QDate getFirstExecutionDate() const;
	void setFirstExecutionDate(const QDate &Date);

	const QDate getLastExecutionDate() const;
	void setLastExecutionDate(const QDate &Date);

	const QDate getNextExecutionDate() const;
	void setNextExecutionDate(const QDate &Date);

	/************************************
	 * Additional for Transfers	    *
	 ************************************/
	/* This group contains information which is used with all kinds of
	 * transfers. It is setup by the function AB_Banking_GatherResponses
	 * for transfers but not used by AqBanking otherwise.
	 */
	AB_TRANSACTION_TYPE getType() const;
	void setType(AB_TRANSACTION_TYPE Type);

	AB_TRANSACTION_SUBTYPE getSubType() const;
	void setSubType(AB_TRANSACTION_SUBTYPE SubType);

	AB_TRANSACTION_STATUS getStatus() const;
	void setStatus(AB_TRANSACTION_STATUS Status);

	AB_TRANSACTION_CHARGE getCharge() const;
	void setCharge(AB_TRANSACTION_CHARGE Charge);

	/************************************
	 * Additional for Foreign Transfers *
	 ************************************/
	/* This group contains information which is used with transfers to
	 * other countries in the world. It is used by backends and
	 * applications but not by AqBanking itself.
	 */

	const QString getRemoteAddrStreet() const;
	void setRemoteAddrStreet(const QString &RemoteAddrStreet);

	const QString getRemoteAddrZipcode() const;
	void setRemoteAddrZipcode(const QString &RemoteAddrZipcode);

	const QString getRemoteAddrCity() const;
	void setRemoteAddrCity(const QString &RemoteAddrCity);

	const QString getRemotePhone() const;
	void setRemotePhone(const QString &RemotePhone);

	/***************************************
	 * Additional for Investment Transfers *
	 ***************************************/
	/* This group contains information which is used with investment/stock
	 * transfers. It is used by backends and applications but not by
	 * AqBanking itself.
	 */

	const QString getUnitId() const;
	void setUnitId(const QString &UnitId);

	const QString getUnitIdNameSpace() const;
	void setUnitIdNameSpace(const QString &UnitIdNameSpace);

	const AB_VALUE* getUnits() const;
	void setUnits(const AB_VALUE *Units);

	const AB_VALUE* getUnitPrice() const;
	void setUnitPrice(const AB_VALUE *UnitPrice);

	const AB_VALUE* getCommission() const;
	void setCommission(const AB_VALUE *Commission);


	/***************************************
	 * ID functions *
	 ***************************************/
	quint32 getUniqueId() const;
	void setUniqueId(quint32 id);

	quint32 getIdForApplication() const;
	void setIdForApplication(quint32 id);

	quint32 getGroupId() const;
	void setGroupId(quint32 id);

	const AB_VALUE* getFees() const;
	void setFees(const AB_VALUE* value);


	/**********************************************************************
	 * Helper Functions                                                   *
	 **********************************************************************/
	bool isModified() const;
	void setModified(bool mod);
	void fillLocalFromAccount(const AB_ACCOUNT *a);

	const AB_TRANSACTION* getAB_Transaction() const;

};

Q_DECLARE_METATYPE(abt_transaction);
Q_DECLARE_METATYPE(abt_transaction*);
Q_DECLARE_METATYPE(const abt_transaction*);

#endif // TRANS_JOB_H
