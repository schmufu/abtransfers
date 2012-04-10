/******************************************************************************
 * Copyright (C) 2012 Patrick Wacker
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
 *	Funktionen die zum parsen des Contextes sowie zum Import und Export
 *	der Daten genutzt werden können
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#include "abt_parser.h"

#include <QtCore/QDebug>

#include "globalvars.h"

#include "abt_conv.h"

#include "aqb_accountinfo.h"
#include "abt_datedtransferinfo.h"
#include "abt_standingorderinfo.h"

#include <aqbanking/accstatus.h>


abt_parser::abt_parser()
{
}


//static
void abt_parser::parse_ctx(AB_IMEXPORTER_CONTEXT *iec, aqb_Accounts *allAccounts)
{
	AB_IMEXPORTER_ACCOUNTINFO *ai;
	const AB_SECURITY *security;
	const AB_MESSAGE *msg;
	const AB_VALUE *v;
	const AB_BALANCE *b;
	QString logmsg;
	QString logmsg2;
	int cnt = 0;
	aqb_AccountInfo *acc;


	/**********************************************************************/
	//this->parseImExporterContext_Messages(iec);
	/**********************************************************************/
	cnt = 0;
	logmsg = "PARSER - Messages: ";

	msg = AB_ImExporterContext_GetFirstMessage(iec);
	while (msg) {
		logmsg2 = QString("Empfangsdatum:\t%1").arg(
				abt_conv::GwenTimeToQDate(AB_Message_GetDateReceived(msg))
				.toString(Qt::DefaultLocaleLongDate));
		qDebug() << logmsg << logmsg2;
		logmsg2 = QString("Betreff:\t%1").arg(AB_Message_GetSubject(msg));
		qDebug() << logmsg << logmsg2;
		logmsg2 = QString("Text:\t%1").arg(AB_Message_GetText(msg));
		qDebug() << logmsg << logmsg2;

		msg = AB_ImExporterContext_GetNextMessage(iec);
		cnt++;
	}

	logmsg2 = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << logmsg2;


	/**********************************************************************/
	//this->parseImExporterContext_Securitys(iec);
	/**********************************************************************/
	logmsg = "PARSER - Security: ";
	cnt = 0;

	security = AB_ImExporterContext_GetFirstSecurity(iec);
	while (security) {
		logmsg2 = QString("Name:\t%1").arg(AB_Security_GetName(security));
		qDebug() << logmsg << logmsg2;

		v = AB_Security_GetUnitPriceValue(security);
		logmsg2 = QString("UnitPriceValue:\t%1").arg(AB_Value_GetValueAsDouble(v));
		qDebug() << logmsg << logmsg2;

		security = AB_ImExporterContext_GetNextSecurity(iec);
		cnt++;
	}

	logmsg2 = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << logmsg2;


	ai=AB_ImExporterContext_GetFirstAccountInfo(iec);
	while(ai) {
		//Jetzt folgen Daten für verschiedene Accounts
		QString KtoNr = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountNumber(ai));
		QString BLZ = QString::fromUtf8(AB_ImExporterAccountInfo_GetBankCode(ai));
		QString Owner = QString::fromUtf8(AB_ImExporterAccountInfo_GetOwner(ai));
		QString Name = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountName(ai));
		QString ID = QString("%1").arg(AB_ImExporterAccountInfo_GetAccountId(ai));

		//das interne accountInfo-Object besorgen
		acc = allAccounts->getAccount(KtoNr, BLZ, Owner, Name);

		/**********************************************************************/
		//this->parseImExporterAccountInfo_Status(ai);
		/**********************************************************************/
		AB_ACCOUNT_STATUS *as;
		logmsg = "PARSER - AccStats: ";
		cnt = 0;

		as = AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
		while (as) {
			logmsg2 = QString("%1 (%3) [%2] Owner: %4 ID: %5").arg(
					   KtoNr, BLZ, Name, Owner, ID);
			qDebug() << logmsg << logmsg2;

			QDate date = abt_conv::GwenTimeToQDate(AB_AccountStatus_GetTime(as));
			logmsg2 = QString("Time:\t%1").arg(date.toString(Qt::DefaultLocaleLongDate));

			acc->setDate(date);

			qDebug() << logmsg << logmsg2;

			v = AB_AccountStatus_GetBankLine(as);
			if (v) {
				logmsg2 = QString("BankLine:\t%1").arg(
						AB_Value_GetValueAsDouble(v), 0, 'f', 2);
				qDebug() << logmsg + logmsg2;
				acc->setBankLine(AB_Value_GetValueAsDouble(v));
			}

			b = AB_AccountStatus_GetNotedBalance(as);
			if (b) {
				v = AB_Balance_GetValue(b);
				if (v) {
					logmsg2 = QString("NotedBalance:\t%1").arg(
							AB_Value_GetValueAsDouble(v), 0, 'f', 2);
					qDebug() << logmsg << logmsg2;
					acc->setNotedBalance(AB_Value_GetValueAsDouble(v));
				}
			}

			b = AB_AccountStatus_GetBookedBalance(as);
			if (b) {
				v = AB_Balance_GetValue(b);
				if (v) {
					logmsg2 = QString("BookedBalance:\t%1").arg(
							AB_Value_GetValueAsDouble(v), 0, 'f', 2);
					qDebug() << logmsg << logmsg2;
					acc->setBookedBalance(AB_Value_GetValueAsDouble(v));
				}
			}

			v = AB_AccountStatus_GetDisposable(as);
			if (v) {
				logmsg2 = QString("Disposable:\t%1").arg(
						AB_Value_GetValueAsDouble(v), 0, 'f', 2);
				qDebug() << logmsg << logmsg2;
				acc->setDisposable(AB_Value_GetValueAsDouble(v));
			}

			v = AB_AccountStatus_GetDisposed(as);
			if (v) {
				logmsg2 = QString("Disposed:\t%1").arg(
						AB_Value_GetValueAsDouble(v), 0, 'f', 2);
				qDebug() << logmsg << logmsg2;
				acc->setDisposed(AB_Value_GetValueAsDouble(v));
			}

			as = AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
			cnt++;
		}

		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;



		/**********************************************************************/
		//this->parseImExporterAccountInfo_DatedTransfers(ai);	//Terminüberweisungen
		/**********************************************************************/
		AB_TRANSACTION *t;
		logmsg = "PARSER - DatedTra: ";
		QStringList strList;
		const GWEN_STRINGLIST *l;

		cnt = AB_ImExporterAccountInfo_GetDatedTransferCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstDatedTransfer(ai);
		while (t) {

			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			logmsg2 = QString("Value:\t%1").arg(AB_Value_GetValueAsDouble(v));
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;

//			switch (AB_Transaction_GetStatus(t)) {
//			case AB_Transaction_StatusRevoked:
//				//Bei der Bank hinterlegte Terminüberweisung wurde gelöscht
//				qDebug() << "Lösche bei der Bank gelöschte Terminüberweisung (ID:"
//					 << AB_Transaction_GetFiId(t) << ")";
//				break;
//			case AB_Transaction_StatusManuallyReconciled:
//			case AB_Transaction_StatusAutoReconciled:
//				//Bei der Bank hinterlegte Terminüberweisung wurde geändert
//				qDebug() << "Speichere bei der Bank geänderte Terminüberweisung (ID:"
//					 << AB_Transaction_GetFiId(t) << ")";
//				break;
//			default:
//				//Bei der Bank hinterlegte Terminüberweisung auch lokal speichern
//				qDebug() << "Speichere bei der Bank hinterlegte Terminüberweisung (ID:"
//					 << AB_Transaction_GetFiId(t) << ")";
//				break;
//			}

			t = AB_ImExporterAccountInfo_GetNextDatedTransfer(ai);
		}



		/**********************************************************************/
		//this->parseImExporterAccountInfo_NotedTransactions(ai);	//geplante Buchungen
		/**********************************************************************/
		logmsg = "PARSER - NotedTra: ";
		strList.clear();
		const AB_VALUE *v;
		//const GWEN_STRINGLIST *l;

		cnt = AB_ImExporterAccountInfo_GetNotedTransactionCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstNotedTransaction(ai);
		while (t) {
			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			logmsg2 = QString("Value:\t%1").arg(AB_Value_GetValueAsDouble(v));
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;

			t = AB_ImExporterAccountInfo_GetNextNotedTransaction(ai);
		}


		/**********************************************************************/
		//this->parseImExporterAccountInfo_StandingOrders(ai);	//Daueraufträge
		/**********************************************************************/
//		AB_TRANSACTION *t;
		logmsg = "PARSER - Standing: ";
		strList.clear();
//		const AB_VALUE *v;
//		const GWEN_STRINGLIST *l;

		cnt = AB_ImExporterAccountInfo_GetStandingOrderCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstStandingOrder(ai);
		while (t) {
			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			logmsg2 = QString("Value:\t%1").arg(AB_Value_GetValueAsDouble(v));
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;

//			switch (AB_Transaction_GetStatus(t)) {
//			case AB_Transaction_StatusRevoked:
//				//Bei der Bank hinterlegter Dauerauftrag wurde gelöscht
//				qDebug() << "Lösche bei der Bank gelöschten Dauerauftrag (ID:"
//					 << AB_Transaction_GetFiId(t) << ")";
//				break;
//			case AB_Transaction_StatusManuallyReconciled:
//			case AB_Transaction_StatusAutoReconciled:
//				//Bei der Bank hinterlegter Dauerauftrag wurde geändert
//				qDebug() << "Speichere bei der Bank geänderten Dauerauftrag (ID:"
//					 << AB_Transaction_GetFiId(t) << ")";
//				break;
//			default:
//				//Bei der Bank hinterlegten Dauerauftrag auch lokal speichern
//				qDebug() << "Speichere bei der Bank hinterlegten Dauerauftrag (ID:"
//					 << AB_Transaction_GetFiId(t) << ")";
//				break;
//			}

			t = AB_ImExporterAccountInfo_GetNextStandingOrder(ai);
		}


		/**********************************************************************/
		//this->parseImExporterAccountInfo_Transactions(ai);	//Buchungen
		/**********************************************************************/
//		AB_TRANSACTION *t;
		logmsg = "PARSER - Transact: ";
		strList.clear();;
//		const AB_VALUE *v;
//		const GWEN_STRINGLIST *l;

		cnt = AB_ImExporterAccountInfo_GetTransactionCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstTransaction(ai);
		while (t) {
			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			logmsg2 = QString("Value:\t%1").arg(AB_Value_GetValueAsDouble(v));
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;

			t = AB_ImExporterAccountInfo_GetNextTransaction(ai);
		}


		/**********************************************************************/
		//this->parseImExporterAccountInfo_Transfers(ai);		//Überweisungen
		/**********************************************************************/
//		AB_TRANSACTION *t;
		logmsg = "PARSER - Transfer: ";
		strList.clear();
//		const AB_VALUE *v;
//		const GWEN_STRINGLIST *l;

		cnt = AB_ImExporterAccountInfo_GetTransferCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstTransfer(ai);
		while (t) {
			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			logmsg2 = QString("Value:\t%1").arg(AB_Value_GetValueAsDouble(v));
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;


			t = AB_ImExporterAccountInfo_GetNextTransfer(ai);
		}

		//next account
		ai=AB_ImExporterContext_GetNextAccountInfo(iec);
	} /* while ai */

}
