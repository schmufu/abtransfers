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
#include "abt_jobinfo.h"

#include <aqbanking/accstatus.h>


/**
 * When a AB_TRANSACTION is exported from the history by the AB_IMEXPORTER_CONTEXT,
 * that is created at abt_history::getContext(), the AB_JOB_TYPE and AB_JOB_STATUS
 * are stored in the category field of the AB_TRANSACTION (since svn rev310).
 *
 * This function searches the category fields for this values and sets the
 * @a jobType and @a jobStatus to the found values.
 *
 * If nothing is found, the @a jobType and @a jobStatus are not changed!
 *
 * The values will also be removed from the category field of the AB_TRANSACTION!
 *
 */
//private static
void abt_parser::getJobStatesFromTransaction(AB_TRANSACTION *t, AB_JOB_TYPE &jobType, AB_JOB_STATUS &jobStatus)
{
	const GWEN_STRINGLIST *gsl = AB_Transaction_GetCategory(t);
	if (!gsl) return; //abort, no category exist

	//we need a copy, so that we don not modify our stringlist while we
	//iterating over it.
	GWEN_STRINGLIST *sl = GWEN_StringList_dup(gsl);
	if (!sl) return; //abort

	for(unsigned int i=0; i<GWEN_StringList_Count(sl); i++) {
		QString s = QString::fromUtf8(GWEN_StringList_StringAt(sl, i));
		if (s.startsWith("JobType: ")) {
			jobType = AB_JOB_TYPE(s.right(s.length() - QString("JobType: ").length()).toInt());
			AB_Transaction_RemoveCategory(t, s.toUtf8());
		}
		if (s.startsWith("JobStatus: ")) {
			jobStatus = AB_JOB_STATUS(s.right(s.length() - QString("JobStatus: ").length()).toInt());
			AB_Transaction_RemoveCategory(t, s.toUtf8());
		}
	}

	GWEN_StringList_free(sl);
}

abt_parser::abt_parser()
{
}


/**
  * Der zurückgegebene Context muss über AB_ImExporterContext_free() wieder
  * freigegeben werden!
  */
//static
AB_IMEXPORTER_CONTEXT *abt_parser::load_local_ctx(const QString &filename,
						  const QString &importerName,
						  const QString &profileName)
{
	AB_IMEXPORTER_CONTEXT *ctx = AB_ImExporterContext_new();

	AB_Banking_ImportFileWithProfile(banking->getAqBanking(),
					 importerName.toUtf8(),
					 ctx,
					 profileName.toUtf8(),
					 NULL,
					 filename.toUtf8());

	//eventuelle "Lücken" versuchen zu füllen (z.B. Account ID)
	int ret;
	ret = AB_Banking_FillGapsInImExporterContext(banking->getAqBanking(),
						     ctx);
	if (ret) {
		qWarning() << Q_FUNC_INFO << "ERROR =" << ret
			   << " -- something went wrong on filling the gaps"
			   << "(this is not an serious error)";
	}



	return ctx;
}


//static
void abt_parser::save_local_ctx(AB_IMEXPORTER_CONTEXT *ctx,
				const QString &filename,
				const QString &exporterName,
				const QString &profileName)
{
	Q_ASSERT(ctx);
	int ret;

	ret = AB_Banking_FillGapsInImExporterContext(banking->getAqBanking(),
						     ctx);

	if (ret) {
		qWarning() << Q_FUNC_INFO << "ERROR:" << ret
			   << " -- something went wrong on filling the gaps"
			   << "(this is not an serious error)";
	}

	ret = AB_Banking_ExportToFile(banking->getAqBanking(),
				      ctx,
				      exporterName.toUtf8(),
				      profileName.toUtf8(),
				      filename.toUtf8());

	if (ret) {
		//Fehler aufgetreten!
		qWarning() << Q_FUNC_INFO << "ERROR:" << ret
			   << " -- something went wrong on storing the ctx to file";
	}

}


/**
  * Wenn keine Accounts übergeben wurden wird NULL zurück gegeben!
  *
  * Der zurückgegebene Context muss über AB_ImExporterContext_free() wieder
  * freigegeben werden!
  */
//static
AB_IMEXPORTER_CONTEXT *abt_parser::create_ctx_from(const aqb_Accounts *allAccounts)
{
	if (!allAccounts) return NULL; //Abbruch, keine Accounts vorhanden

	AB_IMEXPORTER_CONTEXT *ctx = AB_ImExporterContext_new();

	//wir gehen alle Objecte durch und holen uns den jeweiligen IE-Context
	QHashIterator<int, aqb_AccountInfo*> it(allAccounts->getAccountHash());
	while (it.hasNext()) {
		it.next();
		qDebug() << it.key() << ": " << it.value();
		AB_ImExporterContext_AddContext(ctx, it.value()->getContext());
	}

	return ctx;
}

/**
 * parses the supplied AB_IMEXPORTER_CONTEXT \a iec and adds the found
 * transactions to the corresponding aqb_Accounts \a allAccounts.
 *
 * This function should be used to parse the account data and the context which
 * is returned from the bank.
 *
 * (the parse_ctx() function wich also has an abt_history parameter should be
 *  used so load the saved history transactions).
 *
 */
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

	//wenn kein gültiger context vorhanden ist brauchen wir auch nichts machen
	if (!iec) return;

	//eventuelle Lücken im Context füllen (ID, Bankname etc)
	int ret = AB_Banking_FillGapsInImExporterContext(banking->getAqBanking(),
							 iec);
	if (ret) {
		qWarning() << Q_FUNC_INFO << "ERROR =" << ret
			   << " -- something went wrong on filling the gaps"
			   << "(this is not an serious error)";
	}


	/**********************************************************************/
	// Nachrichten
	/**********************************************************************/
	cnt = 0;
	logmsg = "PARSER - Messages: ";

	msg = AB_ImExporterContext_GetFirstMessage(iec);
	while (msg) {

		/** \todo Messages sollten noch gespeichert und angezeigt werden.
			  Momentan werden die Daten nur als Debug-Meldung
			  ausgegeben.
		*/

		logmsg2 = QString("Empfangsdatum:\t%1").arg(
				abt_conv::GwenTimeToQDate(AB_Message_GetDateReceived(msg))
				.toString(Qt::DefaultLocaleLongDate));
		qDebug() << logmsg << logmsg2;
		logmsg2 = QString("Betreff:\t%1").arg(AB_Message_GetSubject(msg));
		qDebug() << logmsg << logmsg2;
		logmsg2 = QString("Text:\t%1").arg(AB_Message_GetText(msg));
		qDebug() << logmsg << logmsg2;
		logmsg2 = QString("AccountID: %1  -  UserID: %2").arg(
				  AB_Message_GetAccountId(msg),
				  AB_Message_GetUserId(msg));
		qDebug() << logmsg << logmsg2;

		msg = AB_ImExporterContext_GetNextMessage(iec);
		cnt++;
	}

	logmsg2 = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << logmsg2;


	/**********************************************************************/
	// Securitys
	/**********************************************************************/
	logmsg = "PARSER - Security: ";
	cnt = 0;

	security = AB_ImExporterContext_GetFirstSecurity(iec);
	while (security) {

		/** \todo Securitys sollten noch gespeichert und angezeigt werden.
			  Momentan werden die Daten nur als Debug-Meldung
			  ausgegeben.
		*/

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
		logmsg = "PARSER - Acc-Info: ";

		//Jetzt folgen Daten für verschiedene Accounts
		QString KtoNr = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountNumber(ai));
		QString BLZ = QString::fromUtf8(AB_ImExporterAccountInfo_GetBankCode(ai));
		QString Owner = QString::fromUtf8(AB_ImExporterAccountInfo_GetOwner(ai));
		QString Name = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountName(ai));
		QString ID = QString("%1").arg(AB_ImExporterAccountInfo_GetAccountId(ai));

		//das interne accountInfo-Object besorgen (gibt NULL zurück wenn
		//keins gefunden wurde)
		acc = allAccounts->getAccount(KtoNr, BLZ, Owner, Name);

		//wenn kein lokaler account gefunden wurde können diesem auch
		//keine Daten zugewiesen werden
		if (!acc) {
			qWarning() << logmsg << "Keinen Account gefunden! Import für: "
					     << "KtoNr:" << KtoNr
					     << "BLZ:" << BLZ
					     << "Name:" << Name
					     << "Besitzer:" << Owner;
			//next account
			ai=AB_ImExporterContext_GetNextAccountInfo(iec);
			continue;
		}

		logmsg2 = QString("%1 (%3) [%2] Owner: %4 ID: %5").arg(
				   KtoNr, BLZ, Name, Owner, ID);
		qDebug() << logmsg << logmsg2;


		/**********************************************************************/
		// Kontostände
		/**********************************************************************/
		AB_ACCOUNT_STATUS *as;
		logmsg = "PARSER - AccStats: ";
		cnt = 0;

		as = AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
		while (as) {
			QDate date = abt_conv::GwenTimeToQDate(AB_AccountStatus_GetTime(as));
			logmsg2 = QString("Time:\t\t%1").arg(date.toString(Qt::DefaultLocaleLongDate));

			//Den neuen Account_Status im entsprechenden Object setzen
			acc->setAccountStatus(as);

			qDebug() << logmsg << logmsg2;

			v = AB_AccountStatus_GetBankLine(as);
			if (v) {
				logmsg2 = QString("BankLine:\t%1").arg(
						AB_Value_GetValueAsDouble(v), 0, 'f', 2);
				qDebug() << logmsg << logmsg2;
			}

			b = AB_AccountStatus_GetNotedBalance(as);
			if (b) {
				v = AB_Balance_GetValue(b);
				if (v) {
					logmsg2 = QString("NotedBalance:\t%1").arg(
							AB_Value_GetValueAsDouble(v), 0, 'f', 2);
					qDebug() << logmsg << logmsg2;
				}
			}

			b = AB_AccountStatus_GetBookedBalance(as);
			if (b) {
				v = AB_Balance_GetValue(b);
				if (v) {
					logmsg2 = QString("BookedBalance:\t%1").arg(
							AB_Value_GetValueAsDouble(v), 0, 'f', 2);
					qDebug() << logmsg << logmsg2;
				}
			}

			v = AB_AccountStatus_GetDisposable(as);
			if (v) {
				logmsg2 = QString("Disposable:\t%1").arg(
						AB_Value_GetValueAsDouble(v), 0, 'f', 2);
				qDebug() << logmsg << logmsg2;
			}

			v = AB_AccountStatus_GetDisposed(as);
			if (v) {
				logmsg2 = QString("Disposed:\t%1").arg(
						AB_Value_GetValueAsDouble(v), 0, 'f', 2);
				qDebug() << logmsg << logmsg2;
			}

			as = AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
			cnt++;
		}

		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;



		/**********************************************************************/
		// Terminüberweisungen
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
			//die terminierte Überweisung dem Account zufügen
			//oder evt. auch löschen. Abhängig vom Status.
			abt_datedTransferInfo *dt = new abt_datedTransferInfo(t);
			if (dt->getTransaction()->getStatus() == AB_Transaction_StatusRevoked) {
				//Dem account werden nur gültige Terminüberweisungen
				//zugeordnet, gelöschte erscheinen nur in der History
				acc->removeDatedTransfer(dt);
				delete dt; //dt wird nicht länger benötigt
			} else {
				acc->addDatedTransfer(dt);
				//dt wird durch den Account wieder gelöscht
			}

			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			if (v) {
				logmsg2 = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));

			} else {
				logmsg2 = QString("Value:\t\tNOT SET!");
			}
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;

			t = AB_ImExporterAccountInfo_GetNextDatedTransfer(ai);
		}



		/**********************************************************************/
		// geplante Buchungen
		/**********************************************************************/
		logmsg = "PARSER - NotedTra: ";
		strList.clear();
		const AB_VALUE *v;

		cnt = AB_ImExporterAccountInfo_GetNotedTransactionCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstNotedTransaction(ai);
		while (t) {

			/** \todo Wenn auch Transactionen angezeigt werden sollen
				  muss dies noch implementiert werden.
			*/

			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			logmsg2 = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;

			t = AB_ImExporterAccountInfo_GetNextNotedTransaction(ai);
		}


		/**********************************************************************/
		// Daueraufträge
		/**********************************************************************/
		logmsg = "PARSER - Standing: ";
		strList.clear();

		cnt = AB_ImExporterAccountInfo_GetStandingOrderCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstStandingOrder(ai);
		while (t) {
			//den Dauerauftrag dem Account zufügen
			//oder evt. auch löschen. Abhängig vom Status.
			abt_standingOrderInfo *so = new abt_standingOrderInfo(t);
			if (so->getTransaction()->getStatus() == AB_Transaction_StatusRevoked) {
				//Dem account werden nur gültige Daueraufträge
				//zugeordnet, gelöschte erscheinen nur in der History
				acc->removeStandingOrder(so);
				delete so; //so wird nicht länger benötigt
			} else {
				acc->addStandingOrder(so);
				//so wird durch den Account wieder gelöscht
			}

			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			logmsg2 = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;

			t = AB_ImExporterAccountInfo_GetNextStandingOrder(ai);
		}


		/**********************************************************************/
		// Buchungen
		/**********************************************************************/
		logmsg = "PARSER - Transact: ";
		strList.clear();;

		cnt = AB_ImExporterAccountInfo_GetTransactionCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstTransaction(ai);
		while (t) {

			/** \todo Wenn auch Transactionen angezeigt werden sollen
				  muss dies noch implementiert werden.
			*/

			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			logmsg2 = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;

			t = AB_ImExporterAccountInfo_GetNextTransaction(ai);
		}


		/**********************************************************************/
		// Überweisungen
		/**********************************************************************/
		logmsg = "PARSER - Transfer: ";
		strList.clear();

		cnt = AB_ImExporterAccountInfo_GetTransferCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstTransfer(ai);
		while (t) {

			/** \todo Wenn auch Transactionen angezeigt werden sollen
				  muss dies noch implementiert werden.
			*/

			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("Purpose:\t\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			v = AB_Transaction_GetValue(t);
			logmsg2 = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));
			qDebug() << logmsg << logmsg2;

			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2 = QString("RemoteName:\t%1").arg(strList.join(" - "));
			qDebug() << logmsg << logmsg2;

			AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
			logmsg2 = QString("Status:\t\t%1 (%2)").arg(state).arg(
					AB_Transaction_Status_toString(state));
			qDebug() << logmsg << logmsg2;


			t = AB_ImExporterAccountInfo_GetNextTransfer(ai);
		}

		//next account
		ai=AB_ImExporterContext_GetNextAccountInfo(iec);
	} /* while ai */

}


/**
 * parses the supplied AB_IMEXPORTER_CONTEXT \a iec and adds the found
 * transactions as abt_jobinfo types in the supplied abt_history \a history.
 *
 */
//static
void abt_parser::parse_ctx(AB_IMEXPORTER_CONTEXT *iec,
			   const aqb_Accounts *allAccounts,
			   abt_history *history)
{
	AB_IMEXPORTER_ACCOUNTINFO *ai;
	const AB_SECURITY *security;
	const AB_MESSAGE *msg;
	QString logmsg;
	QString logmsg2;
	int cnt = 0;

	//wenn kein gültiger context vorhanden ist brauchen wir auch nichts machen
	if (!iec) return;

	//die history sollte natürlich auch existieren
	if (!history) return;
	if (!allAccounts) return;

	/**********************************************************************/
	// Nachrichten
	/**********************************************************************/
	cnt = 0;
	logmsg = "PARSER HISTORY - Messages: ";

	msg = AB_ImExporterContext_GetFirstMessage(iec);
	while (msg) {
		/** \todo Sollte implementiert werden wenn Nachrichten in der
			  History gepeichert werden oder angezeigt werden sollen
		*/
		msg = AB_ImExporterContext_GetNextMessage(iec);
		cnt++;
	}

	logmsg2 = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << logmsg2;


	/**********************************************************************/
	// Securitys
	/**********************************************************************/
	logmsg = "PARSER HISTORY - Security: ";
	cnt = 0;

	security = AB_ImExporterContext_GetFirstSecurity(iec);
	while (security) {
		/** \todo Sollte implementiert werden wenn Securitys in der
			  History gepeichert werden oder angezeigt werden sollen
		*/
		//Must be implemented
		security = AB_ImExporterContext_GetNextSecurity(iec);
		cnt++;
	}

	logmsg2 = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << logmsg2;


	ai=AB_ImExporterContext_GetFirstAccountInfo(iec);
	while(ai) {
		logmsg = "PARSER HISTORY - Acc-Info: ";

		//Jetzt folgen Daten für verschiedene Accounts
		QString KtoNr = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountNumber(ai));
		QString BLZ = QString::fromUtf8(AB_ImExporterAccountInfo_GetBankCode(ai));
		QString Owner = QString::fromUtf8(AB_ImExporterAccountInfo_GetOwner(ai));
		//der AccountName wird nicht importiert! Und über fillGaps kann
		//er auch nicht nachträglich gesetzt werden, da der Account nur
		//"virtuell" existiert.

		//beim History account sollte nur einer mit vorgegebenen Daten
		//existieren!
		if (KtoNr != "0000000000" || BLZ != "00000000" ||
		    Owner != "AB-Transfers" ) {
			//Account ist fehlerhaft!
			qWarning() << logmsg << "History Account not correct! "
					     << "KtoNr:" << KtoNr
					     << "BLZ:" << BLZ
					     << "Owner:" << Owner;
			qWarning() << logmsg << "ABORTING HISTORY IMPORT";
			//perhaps the next account is correct, try it
			ai=AB_ImExporterContext_GetNextAccountInfo(iec);
			continue;
		}

		logmsg2 = QString("%1 [%2] Owner: %3").arg(KtoNr, BLZ, Owner);
		qDebug() << logmsg << logmsg2;


		/**********************************************************************/
		// Salden
		/**********************************************************************/
		AB_ACCOUNT_STATUS *as;
		logmsg = "PARSER HISTORY - AccStats: ";
		cnt = 0;

		as = AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
		while (as) {
			// The account states are not stored in the history!
			as = AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
			cnt++;
		}

		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;


		/**********************************************************************/
		// Terminüberweisungen
		/**********************************************************************/
		AB_TRANSACTION *t;
		logmsg = "PARSER HISTORY - DatedTra: ";
		QStringList strList;

		cnt = AB_ImExporterAccountInfo_GetDatedTransferCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstDatedTransfer(ai);
		while (t) {
			QString t_kto = AB_Transaction_GetLocalAccountNumber(t);
			QString t_blz = AB_Transaction_GetLocalBankCode(t);
			aqb_AccountInfo *acc = allAccounts->getAccount(t_kto, t_blz);
			if (!acc) {
				//keinen passenden Account gefunden, nächster
				qWarning() << logmsg << "No matching account found!"
						     << "( KTO:" << t_kto
						     << " - BLZ:" << t_blz << ")";
				t = AB_ImExporterAccountInfo_GetNextDatedTransfer(ai);
				continue;
			}
			AB_ACCOUNT *a = acc->get_AB_ACCOUNT();

			//default values for a dated transfer
			AB_JOB_TYPE jtype = AB_Job_TypeCreateDatedTransfer;
			AB_JOB_STATUS jstatus = AB_Job_StatusFinished;

			//get the jobType and jobStatus from the category field
			//of the saved transaction.
			getJobStatesFromTransaction(t, jtype, jstatus);

			abt_jobInfo *ji = new abt_jobInfo(jtype, jstatus, t, a);

			history->add(ji);

			t = AB_ImExporterAccountInfo_GetNextDatedTransfer(ai);
		}


		/**********************************************************************/
		// geplante Buchungen
		/**********************************************************************/
		logmsg = "PARSER HISTORY - NotedTra: ";
		strList.clear();

		cnt = AB_ImExporterAccountInfo_GetNotedTransactionCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstNotedTransaction(ai);
		while (t) {
			/** \todo Sollte implementiert werden wenn geplante Buchungen in der
				  History gepeichert werden oder angezeigt werden sollen
			*/
			t = AB_ImExporterAccountInfo_GetNextNotedTransaction(ai);
		}


		/**********************************************************************/
		// Daueraufträge
		/**********************************************************************/
		logmsg = "PARSER HISTORY - Standing: ";
		strList.clear();

		cnt = AB_ImExporterAccountInfo_GetStandingOrderCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstStandingOrder(ai);
		while (t) {
			QString t_kto = AB_Transaction_GetLocalAccountNumber(t);
			QString t_blz = AB_Transaction_GetLocalBankCode(t);
			aqb_AccountInfo *acc = allAccounts->getAccount(t_kto, t_blz);
			if (!acc) {
				// keinen passenden Account gefunden, Nächste
				qWarning() << logmsg << "No matching account found!"
						     << "( KTO:" << t_kto
						     << " - BLZ:" << t_blz << ")";
				t = AB_ImExporterAccountInfo_GetNextStandingOrder(ai);
				continue;
			}
			AB_ACCOUNT *a = acc->get_AB_ACCOUNT();

			//default values for a standing order
			AB_JOB_TYPE jtype = AB_Job_TypeCreateStandingOrder;
			AB_JOB_STATUS jstatus = AB_Job_StatusFinished;

			//get the jobType and jobStatus from the category field
			//of the saved transaction.
			getJobStatesFromTransaction(t, jtype, jstatus);

			abt_jobInfo *ji = new abt_jobInfo(jtype, jstatus, t, a);

			history->add(ji);

			t = AB_ImExporterAccountInfo_GetNextStandingOrder(ai);
		}


		/**********************************************************************/
		// Buchungen
		/**********************************************************************/
		logmsg = "PARSER HISTORY - Transact: ";
		strList.clear();;

		cnt = AB_ImExporterAccountInfo_GetTransactionCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstTransaction(ai);
		while (t) {
			/** \todo Sollte implementiert werden wenn Buchungen in der
				  History gepeichert werden oder angezeigt werden sollen
			*/
			t = AB_ImExporterAccountInfo_GetNextTransaction(ai);
		}


		/**********************************************************************/
		// Überweisungen
		/**********************************************************************/
		logmsg = "PARSER HISTORY - Transfer: ";
		strList.clear();

		cnt = AB_ImExporterAccountInfo_GetTransferCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		qDebug() << logmsg << logmsg2;

		t = AB_ImExporterAccountInfo_GetFirstTransfer(ai);
		while (t) {
			QString lclKto = AB_Transaction_GetLocalAccountNumber(t);
			QString lclBLZ = AB_Transaction_GetLocalBankCode(t);

			aqb_AccountInfo *lclAcc = allAccounts->getAccount(lclKto, lclBLZ);
			if (!lclAcc) {
				// keinen passenden Account gefunden, Nächste
				qWarning() << logmsg << "No matching account found!"
						     << "( KTO:" << lclKto
						     << " - BLZ:" << lclBLZ << ")";
				t = AB_ImExporterAccountInfo_GetNextTransfer(ai);
				continue;
			}

			//default values for a transfer
			AB_JOB_TYPE jtype = AB_Job_TypeTransfer;
			AB_JOB_STATUS jstatus = AB_Job_StatusFinished;

			//get the jobType and jobStatus from the category field
			//of the saved transaction.
			getJobStatesFromTransaction(t, jtype, jstatus);

			abt_jobInfo *ji = new abt_jobInfo(jtype, jstatus, t,
							  lclAcc->get_AB_ACCOUNT());

			history->add(ji);

			t = AB_ImExporterAccountInfo_GetNextTransfer(ai);
		}

		//next account
		ai=AB_ImExporterContext_GetNextAccountInfo(iec);
	} /* while ai */

}
