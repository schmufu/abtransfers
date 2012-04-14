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

#ifndef ABT_TRANSACTIONLIMITS_H
#define ABT_TRANSACTIONLIMITS_H

#include <QtCore/QString>
#include <QtCore/QStringList>

#include <aqbanking/transactionlimits.h>

/** \brief wrapper zu den AB_TRANSACTION_LIMITS von AqBanking
 *
 * Diese Hilfe wurde aus der doxygen Dokumentation von AqBanking übernommen.
 *
 *
 * This page describes the properties of AB_TRANSACTION_LIMITS
 *
 * This type describes the limits for fields of an AB_TRANSACTION.
 *
 *  The limits have the following meanings:
 *    - maxLenSOMETHING: if 0 then this limit is unknown, if -1 then the described element is not allowed to be set in the transaction. All other values represent the maximum length of the described field.
 *    - minLenSOMETHING: if 0 then this limit is unknown. All other values represent the minimum length of the described field.
 *    - maxLinesSOMETHING: if 0 then this limit is unknown All other values represent the maximum number of lines for the described field.
 *    - minLinesSOMETHING: if 0 then this limit is unknown. All other values represent the minimum number of lines for the described field.
 *    - valuesSOMETHING: A list of allowed values (as string). If this list is empty then there all values are allowed (those lists exist in any case, so the appropriate getter function will never return NULL).
 *    - allowSOMETHING: If SOMETHING is allowed then the value is "1". If SOMETHING is NOT allowed then the value is "-1". If it is unknown whether SOMETHING is allowed or not then this value is "0".
 *
 * So if you want to check whether an given field is at all allowed you must check whether "maxLenSOMETHING" has a value of "-1".
 */

class abt_transactionLimits
{
public:
	/*! Sets all fields with the approprirate Value for the given TransactionLimits */
	abt_transactionLimits(const AB_TRANSACTION_LIMITS *el);
	~abt_transactionLimits();

	/** \name Issuer Name
	    Limits for the issuer name. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenLocalName, get it with AB_TransactionLimits_GetMaxLenLocalName
	int MaxLenLocalName;

	//! Set this property with AB_TransactionLimits_SetMinLenLocalName, get it with AB_TransactionLimits_GetMinLenLocalName
	int MinLenLocalName;

	/// @}


	/** \name Payee Name
	    Limits for the payee name. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenRemoteName, get it with AB_TransactionLimits_GetMaxLenRemoteName
	int MaxLenRemoteName;

	//! Set this property with AB_TransactionLimits_SetMinLenRemoteName, get it with AB_TransactionLimits_GetMinLenRemoteName
	int MinLenRemoteName;

	//! Set this property with AB_TransactionLimits_SetMaxLinesRemoteName, get it with AB_TransactionLimits_GetMaxLinesRemoteName
	int MaxLinesRemoteName;

	//! Set this property with AB_TransactionLimits_SetMinLinesRemoteName, get it with AB_TransactionLimits_GetMinLinesRemoteName
	int MinLinesRemoteName;

	/// @}


	/** \name Local Bank Code
	    Limits for local bank code. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenLocalBankCode, get it with AB_TransactionLimits_GetMaxLenLocalBankCode
	int MaxLenLocalBankCode;

	//! Set this property with AB_TransactionLimits_SetMinLenLocalBankCode, get it with AB_TransactionLimits_GetMinLenLocalBankCode
	int MinLenLocalBankCode;

	/// @}


	/** \name Local Account Id
	    Limits for local account id. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenLocalAccountNumber, get it with AB_TransactionLimits_GetMaxLenLocalAccountNumber
	int MaxLenLocalAccountNumber;

	//! Set this property with AB_TransactionLimits_SetMinLenLocalAccountNumber, get it with AB_TransactionLimits_GetMinLenLocalAccountNumber
	int MinLenLocalAccountNumber;

	/// @}


	/** \name Local Account Number
	    Limits for local account id suffix. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenLocalSuffix, get it with AB_TransactionLimits_GetMaxLenLocalSuffix
	int MaxLenLocalSuffix;

	//! Set this property with AB_TransactionLimits_SetMinLenLocalSuffix, get it with AB_TransactionLimits_GetMinLenLocalSuffix
	int MinLenLocalSuffix;

	/// @}


	/** \name Remote Bank Code
	    Limits for remote bank code. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenRemoteBankCode, get it with AB_TransactionLimits_GetMaxLenRemoteBankCode
	int MaxLenRemoteBankCode;

	//! Set this property with AB_TransactionLimits_SetMinLenRemoteBankCode, get it with AB_TransactionLimits_GetMinLenRemoteBankCode
	int MinLenRemoteBankCode;

	/// @}


	/** \name Remote Account Number
	    Limits for remote account number. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenRemoteAccountNumber, get it with AB_TransactionLimits_GetMaxLenRemoteAccountNumber
	int MaxLenRemoteAccountNumber;

	//! Set this property with AB_TransactionLimits_SetMinLenRemoteAccountNumber, get it with AB_TransactionLimits_GetMinLenRemoteAccountNumber
	int MinLenRemoteAccountNumber;

	/// @}


	/** \name Remote Account Number Suffix
	    Limits for remote account id suffix. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenRemoteSuffix, get it with AB_TransactionLimits_GetMaxLenRemoteSuffix
	int MaxLenRemoteSuffix;

	//! Set this property with AB_TransactionLimits_SetMinLenRemoteSuffix, get it with AB_TransactionLimits_GetMinLenRemoteSuffix
	int MinLenRemoteSuffix;

	/// @}


	/** \name Remote IBAN
	    Limits for remote IAN. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenRemoteIban, get it with AB_TransactionLimits_GetMaxLenRemoteIban
	int MaxLenRemoteIban;

	//! Set this property with AB_TransactionLimits_SetMinLenRemoteIban, get it with AB_TransactionLimits_GetMinLenRemoteIban
	int MinLenRemoteIban;

	/// @}


	/** \name Text Key
	    Limits for textKey.*/
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenTextKey, get it with AB_TransactionLimits_GetMaxLenTextKey
	int MaxLenTextKey;

	//! Set this property with AB_TransactionLimits_SetMinLenTextKey, get it with AB_TransactionLimits_GetMinLenTextKey
	int MinLenTextKey;

	//! Set this property with AB_TransactionLimits_SetValuesTextKey, get it with AB_TransactionLimits_GetValuesTextKey
	/** This string list contains one entry for every supported text key. The values must be positive integers in decimal form (no leading zero, no comma or decimal point). */
	QStringList ValuesTextKey;

	//! Set this property with AB_TransactionLimits_SetTextKeys, get it with AB_TransactionLimits_GetTextKeys
	/** This list of text key descriptions may contain an entry for every supported text key. However, not all backends fill this list and this list does not have to be complete. If you want to know which textkeys are supported please use valuesTextKey instead. */
	QString TextKeys;

	/// @}


	/** \name Customer Reference
	    Limits for customer reference. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenCustomerReference, get it with AB_TransactionLimits_GetMaxLenCustomerReference
	int MaxLenCustomerReference;

	//! Set this property with AB_TransactionLimits_SetMinLenCustomerReference, get it with AB_TransactionLimits_GetMinLenCustomerReference
	int MinLenCustomerReference;

	/// @}


	/** \name Bank Reference
	    Limits for bank reference. */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenBankReference, get it with AB_TransactionLimits_GetMaxLenBankReference
	int MaxLenBankReference;

	//! Set this property with AB_TransactionLimits_SetMinLenBankReference, get it with AB_TransactionLimits_GetMinLenBankReference
	int MinLenBankReference;

	/// @}


	/** \name Purpose
	    Limits for purpose (called memo in some apps). */
	/// @{

	//! Set this property with AB_TransactionLimits_SetMaxLenPurpose, get it with AB_TransactionLimits_GetMaxLenPurpose
	int MaxLenPurpose;

	//! Set this property with AB_TransactionLimits_SetMinLenPurpose, get it with AB_TransactionLimits_GetMinLenPurpose
	int MinLenPurpose;

	//! Set this property with AB_TransactionLimits_SetMaxLinesPurpose, get it with AB_TransactionLimits_GetMaxLinesPurpose
	int MaxLinesPurpose;

	//! Set this property with AB_TransactionLimits_SetMinLinesPurpose, get it with AB_TransactionLimits_GetMinLinesPurpose
	int MinLinesPurpose;

	/// @}


	/** \name Standing Orders And Dated Transfer
	    These limits apply to standing orders and dated transfers only.*/
	/// @{

	//! Set this property with AB_TransactionLimits_SetMinValueSetupTime, get it with AB_TransactionLimits_GetMinValueSetupTime
	/** Minimum time in days between issuing of a request and its first execution. */
	int MinValueSetupTime;

	//! Set this property with AB_TransactionLimits_SetMaxValueSetupTime, get it with AB_TransactionLimits_GetMaxValueSetupTime
	/** Maximum time in days between issuing of a request and its first execution. */
	int MaxValueSetupTime;

	//! Set this property with AB_TransactionLimits_SetValuesCycleWeek, get it with AB_TransactionLimits_GetValuesCycleWeek
	/** This string list contains one entry for every supported cycle. These value are accepted when "period" is "weekly". The values must be positive integers in decimal form (no leading zero, no comma or decimal point). Allowed values are "0" (all cycles possible) and "1"-"52". */
	QStringList ValuesCycleWeek;

	//! Set this property with AB_TransactionLimits_SetValuesCycleMonth, get it with AB_TransactionLimits_GetValuesCycleMonth
	/** This string list contains one entry for every supported cycle. These value are accepted when "period" is "monthly". The values must be positive integers in decimal form (no leading zero, no comma or decimal point). Allowed values are "0" (all cycles possible) and "1"-"12". */
	QStringList ValuesCycleMonth;

	//! Set this property with AB_TransactionLimits_SetValuesExecutionDayWeek, get it with AB_TransactionLimits_GetValuesExecutionDayWeek
	/** This string list contains one entry for every supported day of the week. These value are accepted when "period" is "weekly". The values must be positive integers in decimal form (no leading zero, no comma or decimal point). Allowed values are "0" (all days allowed) and "1"-"7". */
	QStringList ValuesExecutionDayWeek;

	//! Set this property with AB_TransactionLimits_SetValuesExecutionDayMonth, get it with AB_TransactionLimits_GetValuesExecutionDayMonth
	/** This string list contains one entry for every supported monthly cycle. These value are accepted when "period" is "monthly". The values must be positive integers in decimal form (no leading zero, no comma or decimal point). Allowed are "0" (all days possible), "1"-"30", "97" (ultimo-2), "98" (ultimo-1) and "99" (ultimo). */
	QStringList ValuesExecutionDayMonth;

	//! Set this property with AB_TransactionLimits_SetAllowMonthly, get it with AB_TransactionLimits_GetAllowMonthly
	int AllowMonthly;

	//! Set this property with AB_TransactionLimits_SetAllowWeekly, get it with AB_TransactionLimits_GetAllowWeekly
	int AllowWeekly;

	//! Set this property with AB_TransactionLimits_SetAllowChangeRecipientAccount, get it with AB_TransactionLimits_GetAllowChangeRecipientAccount
	int AllowChangeRecipientAccount;

	//! Set this property with AB_TransactionLimits_SetAllowChangeRecipientName, get it with AB_TransactionLimits_GetAllowChangeRecipientName
	int AllowChangeRecipientName;

	//! Set this property with AB_TransactionLimits_SetAllowChangeValue, get it with AB_TransactionLimits_GetAllowChangeValue
	int AllowChangeValue;

	//! Set this property with AB_TransactionLimits_SetAllowChangeTextKey, get it with AB_TransactionLimits_GetAllowChangeTextKey
	int AllowChangeTextKey;

	//! Set this property with AB_TransactionLimits_SetAllowChangePurpose, get it with AB_TransactionLimits_GetAllowChangePurpose
	int AllowChangePurpose;

	//! Set this property with AB_TransactionLimits_SetAllowChangeFirstExecutionDate, get it with AB_TransactionLimits_GetAllowChangeFirstExecutionDate
	int AllowChangeFirstExecutionDate;

	//! Set this property with AB_TransactionLimits_SetAllowChangeLastExecutionDate, get it with AB_TransactionLimits_GetAllowChangeLastExecutionDate
	int AllowChangeLastExecutionDate;

	//! Set this property with AB_TransactionLimits_SetAllowChangeCycle, get it with AB_TransactionLimits_GetAllowChangeCycle
	int AllowChangeCycle;

	//! Set this property with AB_TransactionLimits_SetAllowChangePeriod, get it with AB_TransactionLimits_GetAllowChangePeriod
	int AllowChangePeriod;

	//! Set this property with AB_TransactionLimits_SetAllowChangeExecutionDay, get it with AB_TransactionLimits_GetAllowChangeExecutionDay
	int AllowChangeExecutionDay;

	/// @}

	//! \deprecated Only use this for Debugging!
	void printAllAsDebug() const;
};

#endif // ABT_TRANSACTIONLIMITS_H
