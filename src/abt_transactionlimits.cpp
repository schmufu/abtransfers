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
        this->m_MaxLenLocalName = AB_TransactionLimits_GetMaxLenLocalName(el);
        this->m_MinLenLocalName = AB_TransactionLimits_GetMinLenLocalName(el);
        this->m_MaxLenLocalBankCode = AB_TransactionLimits_GetMaxLenLocalBankCode(el);
        this->m_MinLenLocalBankCode = AB_TransactionLimits_GetMinLenLocalBankCode(el);
        this->m_MaxLenLocalAccountNumber = AB_TransactionLimits_GetMaxLenLocalAccountNumber(el);
        this->m_MinLenLocalAccountNumber = AB_TransactionLimits_GetMinLenLocalAccountNumber(el);
        this->m_MaxLenLocalSuffix = AB_TransactionLimits_GetMaxLenLocalSuffix(el);
        this->m_MinLenLocalSuffix = AB_TransactionLimits_GetMinLenLocalSuffix(el);

        this->m_MaxLenRemoteName = AB_TransactionLimits_GetMaxLenRemoteName(el);
        this->m_MinLenRemoteName = AB_TransactionLimits_GetMinLenRemoteName(el);
        this->m_MaxLinesRemoteName = AB_TransactionLimits_GetMaxLinesRemoteName(el);
        this->m_MinLinesRemoteName = AB_TransactionLimits_GetMinLinesRemoteName(el);

        this->m_MaxLenRemoteBankCode = AB_TransactionLimits_GetMaxLenRemoteBankCode(el);
        this->m_MinLenRemoteBankCode = AB_TransactionLimits_GetMinLenRemoteBankCode(el);

        this->m_MaxLenRemoteAccountNumber = AB_TransactionLimits_GetMaxLenRemoteAccountNumber(el);
        this->m_MinLenRemoteAccountNumber = AB_TransactionLimits_GetMinLenRemoteAccountNumber(el);

        this->m_MaxLenRemoteSuffix = AB_TransactionLimits_GetMaxLenRemoteSuffix(el);
        this->m_MinLenRemoteSuffix = AB_TransactionLimits_GetMinLenRemoteSuffix(el);

        this->m_MaxLenRemoteIban = AB_TransactionLimits_GetMaxLenRemoteIban(el);
        this->m_MinLenRemoteIban = AB_TransactionLimits_GetMinLenRemoteIban(el);

        this->m_MaxLenTextKey = AB_TransactionLimits_GetMaxLenTextKey(el);
        this->m_MinLenTextKey = AB_TransactionLimits_GetMinLenTextKey(el);
        this->m_ValuesTextKey = abt_conv::GwenStringListToQStringList(
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
        this->m_TextKeys = "not used by AB-Transfers, yet.";

        this->m_MaxLenCustomerReference = AB_TransactionLimits_GetMaxLenCustomerReference(el);
        this->m_MinLenCustomerReference = AB_TransactionLimits_GetMinLenCustomerReference(el);

        this->m_MaxLenBankReference = AB_TransactionLimits_GetMaxLenBankReference(el);
        this->m_MinLenBankReference = AB_TransactionLimits_GetMinLenBankReference(el);

        this->m_MaxLenPurpose = AB_TransactionLimits_GetMaxLenPurpose(el);
        this->m_MinLenPurpose = AB_TransactionLimits_GetMinLenPurpose(el);
        this->m_MaxLinesPurpose = AB_TransactionLimits_GetMaxLinesPurpose(el);
        this->m_MinLinesPurpose = AB_TransactionLimits_GetMinLinesPurpose(el);

        this->m_MinValueSetupTime = AB_TransactionLimits_GetMinValueSetupTime(el);
        this->m_MaxValueSetupTime = AB_TransactionLimits_GetMaxValueSetupTime(el);
        this->m_ValuesCycleWeek = abt_conv::GwenStringListToQStringList(
					AB_TransactionLimits_GetValuesCycleWeek(el));
        this->m_ValuesCycleMonth = abt_conv::GwenStringListToQStringList(
					AB_TransactionLimits_GetValuesCycleMonth(el));
        this->m_ValuesExecutionDayWeek = abt_conv::GwenStringListToQStringList(
					AB_TransactionLimits_GetValuesExecutionDayWeek(el));
        this->m_ValuesExecutionDayMonth = abt_conv::GwenStringListToQStringList(
					AB_TransactionLimits_GetValuesExecutionDayMonth(el));
        this->m_AllowMonthly = AB_TransactionLimits_GetAllowMonthly(el);
        this->m_AllowWeekly = AB_TransactionLimits_GetAllowWeekly(el);
        this->m_AllowChangeRecipientAccount = AB_TransactionLimits_GetAllowChangeRecipientAccount(el);
        this->m_AllowChangeRecipientName = AB_TransactionLimits_GetAllowChangeRecipientName(el);
        this->m_AllowChangeValue = AB_TransactionLimits_GetAllowChangeValue(el);
        this->m_AllowChangeTextKey = AB_TransactionLimits_GetAllowChangeTextKey(el);
        this->m_AllowChangePurpose = AB_TransactionLimits_GetAllowChangePurpose(el);
        this->m_AllowChangeFirstExecutionDate = AB_TransactionLimits_GetAllowChangeFirstExecutionDate(el);
        this->m_AllowChangeLastExecutionDate = AB_TransactionLimits_GetAllowChangeLastExecutionDate(el);
        this->m_AllowChangeCycle = AB_TransactionLimits_GetAllowChangeCycle(el);
        this->m_AllowChangePeriod = AB_TransactionLimits_GetAllowChangePeriod(el);
        this->m_AllowChangeExecutionDay = AB_TransactionLimits_GetAllowChangeExecutionDay(el);

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
        << "MaxLenLocalName" << this->m_MaxLenLocalName << "\n"
        << "MinLenLocalName" << this->m_MinLenLocalName << "\n"
        << "MaxLenLocalBankCode" << this->m_MaxLenLocalBankCode << "\n"
        << "MinLenLocalBankCode" << this->m_MinLenLocalBankCode << "\n"
        << "MaxLenLocalAccountNumber" << this->m_MaxLenLocalAccountNumber << "\n"
        << "MinLenLocalAccountNumber" << this->m_MinLenLocalAccountNumber << "\n"
        << "MaxLenLocalSuffix" << this->m_MaxLenLocalSuffix << "\n"
        << "MinLenLocalSuffix" << this->m_MinLenLocalSuffix << "\n"
        << "MaxLenRemoteName" << this->m_MaxLenRemoteName << "\n"
        << "MinLenRemoteName" << this->m_MinLenRemoteName << "\n"
        << "MaxLinesRemoteName" << this->m_MaxLinesRemoteName << "\n"
        << "MinLinesRemoteName" << this->m_MinLinesRemoteName << "\n"
        << "MaxLenRemoteBankCode" << this->m_MaxLenRemoteBankCode << "\n"
        << "MinLenRemoteBankCode" << this->m_MinLenRemoteBankCode << "\n"
        << "MaxLenRemoteAccountNumber" << this->m_MaxLenRemoteAccountNumber << "\n"
        << "MinLenRemoteAccountNumber" << this->m_MinLenRemoteAccountNumber << "\n"
        << "MaxLenRemoteSuffix" << this->m_MaxLenRemoteSuffix << "\n"
        << "MinLenRemoteSuffix" << this->m_MinLenRemoteSuffix << "\n"
        << "MaxLenRemoteIban" << this->m_MaxLenRemoteIban << "\n"
        << "MinLenRemoteIban" << this->m_MinLenRemoteIban << "\n"
        << "MaxLenTextKey" << this->m_MaxLenTextKey << "\n"
        << "MinLenTextKey" << this->m_MinLenTextKey << "\n"
        << "ValuesTextKey" << this->m_ValuesTextKey << "\n"
        << "TextKeys" << this->m_TextKeys << "\n"
        << "MaxLenCustomerReference" << this->m_MaxLenCustomerReference << "\n"
        << "MinLenCustomerReference" << this->m_MinLenCustomerReference << "\n"
        << "MaxLenBankReference" << this->m_MaxLenBankReference << "\n"
        << "MinLenBankReference" << this->m_MinLenBankReference << "\n"
        << "MaxLenPurpose" << this->m_MaxLenPurpose << "\n"
        << "MinLenPurpose" << this->m_MinLenPurpose << "\n"
        << "MaxLinesPurpose" << this->m_MaxLinesPurpose << "\n"
        << "MinLinesPurpose" << this->m_MinLinesPurpose << "\n"
        << "MinValueSetupTime" << this->m_MinValueSetupTime << "\n"
        << "MaxValueSetupTime" << this->m_MaxValueSetupTime << "\n"
        << "ValuesCycleWeek" << this->m_ValuesCycleWeek << "\n"
        << "ValuesCycleMonth" << this->m_ValuesCycleMonth << "\n"
        << "ValuesExecutionDayWeek" << this->m_ValuesExecutionDayWeek << "\n"
        << "ValuesExecutionDayMonth" << this->m_ValuesExecutionDayMonth << "\n"
        << "AllowMonthly" << this->m_AllowMonthly << "\n"
        << "AllowWeekly" << this->m_AllowWeekly << "\n"
        << "AllowChangeRecipientAccount" << this->m_AllowChangeRecipientAccount << "\n"
        << "AllowChangeRecipientName" << this->m_AllowChangeRecipientName << "\n"
        << "AllowChangeValue" << this->m_AllowChangeValue << "\n"
        << "AllowChangeTextKey" << this->m_AllowChangeTextKey << "\n"
        << "AllowChangePurpose" << this->m_AllowChangePurpose << "\n"
        << "AllowChangeFirstExecutionDate" << this->m_AllowChangeFirstExecutionDate << "\n"
        << "AllowChangeLastExecutionDate" << this->m_AllowChangeLastExecutionDate << "\n"
        << "AllowChangeCycle" << this->m_AllowChangeCycle << "\n"
        << "AllowChangePeriod" << this->m_AllowChangePeriod << "\n"
        << "AllowChangeExecutionDay" << this->m_AllowChangeExecutionDay << "\n";
}
