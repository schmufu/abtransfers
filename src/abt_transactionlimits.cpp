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

#include "abt_transactionlimits.h"

#include <QtCore/QDebug>

#include "abt_conv.h"

abt_transactionLimits::abt_transactionLimits(const AB_TRANSACTION_LIMITS *el)
{
	this->MaxLenLocalName = AB_TransactionLimits_GetMaxLenLocalName(el);
	this->MinLenLocalName = AB_TransactionLimits_GetMinLenLocalName(el);
	this->MaxLenLocalBankCode = AB_TransactionLimits_GetMaxLenLocalBankCode(el);
	this->MinLenLocalBankCode = AB_TransactionLimits_GetMinLenLocalBankCode(el);
	this->MaxLenLocalAccountNumber = AB_TransactionLimits_GetMaxLenLocalAccountNumber(el);
	this->MinLenLocalAccountNumber = AB_TransactionLimits_GetMinLenLocalAccountNumber(el);
	this->MaxLenLocalSuffix = AB_TransactionLimits_GetMaxLenLocalSuffix(el);
	this->MinLenLocalSuffix = AB_TransactionLimits_GetMinLenLocalSuffix(el);

	this->MaxLenRemoteName = AB_TransactionLimits_GetMaxLenRemoteName(el);
	this->MinLenRemoteName = AB_TransactionLimits_GetMinLenRemoteName(el);
	this->MaxLinesRemoteName = AB_TransactionLimits_GetMaxLinesRemoteName(el);
	this->MinLinesRemoteName = AB_TransactionLimits_GetMinLinesRemoteName(el);

	this->MaxLenRemoteBankCode = AB_TransactionLimits_GetMaxLenRemoteBankCode(el);
	this->MinLenRemoteBankCode = AB_TransactionLimits_GetMinLenRemoteBankCode(el);

	this->MaxLenRemoteAccountNumber = AB_TransactionLimits_GetMaxLenRemoteAccountNumber(el);
	this->MinLenRemoteAccountNumber = AB_TransactionLimits_GetMinLenRemoteAccountNumber(el);

	this->MaxLenRemoteSuffix = AB_TransactionLimits_GetMaxLenRemoteSuffix(el);
	this->MinLenRemoteSuffix = AB_TransactionLimits_GetMinLenRemoteSuffix(el);

	this->MaxLenRemoteIban = AB_TransactionLimits_GetMaxLenRemoteIban(el);
	this->MinLenRemoteIban = AB_TransactionLimits_GetMinLenRemoteIban(el);

	this->MaxLenTextKey = AB_TransactionLimits_GetMaxLenTextKey(el);
	this->MinLenTextKey = AB_TransactionLimits_GetMinLenTextKey(el);
	this->ValuesTextKey = abt_conv::GwenStringListToQStringList(
					AB_TransactionLimits_GetValuesTextKey(el));

	//TextKeys werden anscheinend von der Sparkasse Bremen nicht unterstützt
	//oder das HBCI Backend setzt diese nicht, deswegen konnte der folgende
	//Code nicht getestet werden!
	/** \todo Wenn die TextKeys von der Bank gesendet werden sollten diese
		  im settings-Objekt gesetzt werden und nicht aus der ini-Datei
		  gelesen werden!
	*/
	//Erstmal auskommentiert, da momentan nicht genutzt!
//	AB_TEXTKEY_DESCR_LIST *dl = AB_TransactionLimits_GetTextKeys(el);
//	int lc = AB_TextKeyDescr_List_GetCount(dl); //ListCount
//	qDebug() << Q_FUNC_INFO << "GetCount = " << lc;
//	if (lc > 0) {
//		QString text;
//		AB_TEXTKEY_DESCR *descr = AB_TextKeyDescr_List_First(dl);
//		for (int i=0; i<lc; ++i) {
//			text.append("Value: ");
//			text.append(AB_TextKeyDescr_GetValue(descr));
//			text.append(" Name: ");
//			text.append(AB_TextKeyDescr_GetName(descr));
//			text.append(" Descr: ");
//			text.append(AB_TextKeyDescr_GetDescr(descr));
//			text.append("\n");
//			descr = AB_TextKeyDescr_List_Next(descr);
//			if (descr == NULL) {
//				qDebug() << "AB_TEXTKEY_DESCR_LIST ZÄHLER ZU WEIT!"
//						<< " -- lc="<<lc << " -- i="<<i;
//				break;
//			}
//		}
//		this->TextKeys = text;
//	} else {
//		this->TextKeys = "Not available";
//	}
	this->TextKeys = QString::fromUtf8("not used by AB-Transfers, yet.");

	this->MaxLenCustomerReference = AB_TransactionLimits_GetMaxLenCustomerReference(el);
	this->MinLenCustomerReference = AB_TransactionLimits_GetMinLenCustomerReference(el);

	this->MaxLenBankReference = AB_TransactionLimits_GetMaxLenBankReference(el);
	this->MinLenBankReference = AB_TransactionLimits_GetMinLenBankReference(el);

	this->MaxLenPurpose = AB_TransactionLimits_GetMaxLenPurpose(el);
	this->MinLenPurpose = AB_TransactionLimits_GetMinLenPurpose(el);
	this->MaxLinesPurpose = AB_TransactionLimits_GetMaxLinesPurpose(el);
	this->MinLinesPurpose = AB_TransactionLimits_GetMinLinesPurpose(el);

	this->MinValueSetupTime = AB_TransactionLimits_GetMinValueSetupTime(el);
	this->MaxValueSetupTime = AB_TransactionLimits_GetMaxValueSetupTime(el);
	this->ValuesCycleWeek = abt_conv::GwenStringListToQStringList(
					AB_TransactionLimits_GetValuesCycleWeek(el));
	this->ValuesCycleMonth = abt_conv::GwenStringListToQStringList(
					AB_TransactionLimits_GetValuesCycleMonth(el));
	this->ValuesExecutionDayWeek = abt_conv::GwenStringListToQStringList(
					AB_TransactionLimits_GetValuesExecutionDayWeek(el));
	this->ValuesExecutionDayMonth = abt_conv::GwenStringListToQStringList(
					AB_TransactionLimits_GetValuesExecutionDayMonth(el));
	this->AllowMonthly = AB_TransactionLimits_GetAllowMonthly(el);
	this->AllowWeekly = AB_TransactionLimits_GetAllowWeekly(el);
	this->AllowChangeRecipientAccount = AB_TransactionLimits_GetAllowChangeRecipientAccount(el);
	this->AllowChangeRecipientName = AB_TransactionLimits_GetAllowChangeRecipientName(el);
	this->AllowChangeValue = AB_TransactionLimits_GetAllowChangeValue(el);
	this->AllowChangeTextKey = AB_TransactionLimits_GetAllowChangeTextKey(el);
	this->AllowChangePurpose = AB_TransactionLimits_GetAllowChangePurpose(el);
	this->AllowChangeFirstExecutionDate = AB_TransactionLimits_GetAllowChangeFirstExecutionDate(el);
	this->AllowChangeLastExecutionDate = AB_TransactionLimits_GetAllowChangeLastExecutionDate(el);
	this->AllowChangeCycle = AB_TransactionLimits_GetAllowChangeCycle(el);
	this->AllowChangePeriod = AB_TransactionLimits_GetAllowChangePeriod(el);
	this->AllowChangeExecutionDay = AB_TransactionLimits_GetAllowChangeExecutionDay(el);

	qDebug() << Q_FUNC_INFO << "created";
}

abt_transactionLimits::~abt_transactionLimits()
{
	qDebug() << Q_FUNC_INFO << "deleted";
}

//public
void abt_transactionLimits::printAllAsDebug() const
{
	qDebug()
	<< "MaxLenLocalName" << this->MaxLenLocalName << "\n"
	<< "MinLenLocalName" << this->MinLenLocalName << "\n"
	<< "MaxLenLocalBankCode" << this->MaxLenLocalBankCode << "\n"
	<< "MinLenLocalBankCode" << this->MinLenLocalBankCode << "\n"
	<< "MaxLenLocalAccountNumber" << this->MaxLenLocalAccountNumber << "\n"
	<< "MinLenLocalAccountNumber" << this->MinLenLocalAccountNumber << "\n"
	<< "MaxLenLocalSuffix" << this->MaxLenLocalSuffix << "\n"
	<< "MinLenLocalSuffix" << this->MinLenLocalSuffix << "\n"
	<< "MaxLenRemoteName" << this->MaxLenRemoteName << "\n"
	<< "MinLenRemoteName" << this->MinLenRemoteName << "\n"
	<< "MaxLinesRemoteName" << this->MaxLinesRemoteName << "\n"
	<< "MinLinesRemoteName" << this->MinLinesRemoteName << "\n"
	<< "MaxLenRemoteBankCode" << this->MaxLenRemoteBankCode << "\n"
	<< "MinLenRemoteBankCode" << this->MinLenRemoteBankCode << "\n"
	<< "MaxLenRemoteAccountNumber" << this->MaxLenRemoteAccountNumber << "\n"
	<< "MinLenRemoteAccountNumber" << this->MinLenRemoteAccountNumber << "\n"
	<< "MaxLenRemoteSuffix" << this->MaxLenRemoteSuffix << "\n"
	<< "MinLenRemoteSuffix" << this->MinLenRemoteSuffix << "\n"
	<< "MaxLenRemoteIban" << this->MaxLenRemoteIban << "\n"
	<< "MinLenRemoteIban" << this->MinLenRemoteIban << "\n"
	<< "MaxLenTextKey" << this->MaxLenTextKey << "\n"
	<< "MinLenTextKey" << this->MinLenTextKey << "\n"
	<< "ValuesTextKey" << this->ValuesTextKey << "\n"
	<< "TextKeys" << this->TextKeys << "\n"
	<< "MaxLenCustomerReference" << this->MaxLenCustomerReference << "\n"
	<< "MinLenCustomerReference" << this->MinLenCustomerReference << "\n"
	<< "MaxLenBankReference" << this->MaxLenBankReference << "\n"
	<< "MinLenBankReference" << this->MinLenBankReference << "\n"
	<< "MaxLenPurpose" << this->MaxLenPurpose << "\n"
	<< "MinLenPurpose" << this->MinLenPurpose << "\n"
	<< "MaxLinesPurpose" << this->MaxLinesPurpose << "\n"
	<< "MinLinesPurpose" << this->MinLinesPurpose << "\n"
	<< "MinValueSetupTime" << this->MinValueSetupTime << "\n"
	<< "MaxValueSetupTime" << this->MaxValueSetupTime << "\n"
	<< "ValuesCycleWeek" << this->ValuesCycleWeek << "\n"
	<< "ValuesCycleMonth" << this->ValuesCycleMonth << "\n"
	<< "ValuesExecutionDayWeek" << this->ValuesExecutionDayWeek << "\n"
	<< "ValuesExecutionDayMonth" << this->ValuesExecutionDayMonth << "\n"
	<< "AllowMonthly" << this->AllowMonthly << "\n"
	<< "AllowWeekly" << this->AllowWeekly << "\n"
	<< "AllowChangeRecipientAccount" << this->AllowChangeRecipientAccount << "\n"
	<< "AllowChangeRecipientName" << this->AllowChangeRecipientName << "\n"
	<< "AllowChangeValue" << this->AllowChangeValue << "\n"
	<< "AllowChangeTextKey" << this->AllowChangeTextKey << "\n"
	<< "AllowChangePurpose" << this->AllowChangePurpose << "\n"
	<< "AllowChangeFirstExecutionDate" << this->AllowChangeFirstExecutionDate << "\n"
	<< "AllowChangeLastExecutionDate" << this->AllowChangeLastExecutionDate << "\n"
	<< "AllowChangeCycle" << this->AllowChangeCycle << "\n"
	<< "AllowChangePeriod" << this->AllowChangePeriod << "\n"
	<< "AllowChangeExecutionDay" << this->AllowChangeExecutionDay << "\n";
}
