/******************************************************************************
 * Copyright (C) 2012-2014 Patrick Wacker
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

#include <QDebug>

#include "globalvars.h"

#include "abt_conv.h"

#include "aqb_accountinfo.h"
#include "abt_datedtransferinfo.h"
#include "abt_standingorderinfo.h"
#include "abt_jobinfo.h"

#include <aqbanking/accstatus.h>


//private static
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
void abt_parser::getJobStatesFromTransaction(AB_TRANSACTION *t, AB_JOB_TYPE *jobType,
					     AB_JOB_STATUS *jobStatus)
{
	const GWEN_STRINGLIST *gsl = AB_Transaction_GetCategory(t);
	if (!gsl) return; //abort, no category exist

	//we need a copy, so that we don not modify our stringlist while we
	//iterating over it.
	GWEN_STRINGLIST *sl = GWEN_StringList_dup(gsl);
	if (!sl) return; //abort

	for (unsigned int i=0; i<GWEN_StringList_Count(sl); i++) {
		QString s = QString::fromUtf8(GWEN_StringList_StringAt(sl, i));
		QString search = "JobType: ";
		if (s.startsWith(search)) {
			*jobType = AB_JOB_TYPE(s.right(s.length() - search.length()).toInt());
			AB_Transaction_RemoveCategory(t, s.toUtf8());
		}
		search = "JobStatus: ";
		if (s.startsWith(search)) {
			*jobStatus = AB_JOB_STATUS(s.right(s.length() - search.length()).toInt());
			AB_Transaction_RemoveCategory(t, s.toUtf8());
		}
	}

	GWEN_StringList_free(sl);
}

//private static
/** \brief adds a new jobInfo object to the history.
 *
 * Gets the local used account from the transaction \a t and creates a new
 * abt_jobInfo object.
 *
 * When the transaction \a t has a type and status set at the category list
 * these values are used, otherwise the supplied default type \a defType and
 * the default status \a defStatus is used to create the abt_jobInfo and
 * add it to the history.
 */
void abt_parser::addJobInfoToHistory(abt_history *history,
				     const aqb_Accounts *allAccounts,
				     AB_TRANSACTION *t, AB_JOB_TYPE defType,
				     AB_JOB_STATUS defStatus)
{
	QString t_kto = AB_Transaction_GetLocalAccountNumber(t);
	QString t_blz = AB_Transaction_GetLocalBankCode(t);
	const aqb_AccountInfo *acc = allAccounts->getAccount(t_kto, t_blz);
	if (!acc) {
		//no matching account
		qWarning() << "No matching account found!"
			   << "( KTO:" << t_kto
			   << " - BLZ:" << t_blz << ")";
		return; //cancel further processing
	}

	AB_ACCOUNT *a = acc->get_AB_ACCOUNT();

	//default values for a dated transfer
	AB_JOB_TYPE jtype = defType;
	AB_JOB_STATUS jstatus = defStatus;

	//get the jobType and jobStatus from the category field of the transaction.
	abt_parser::getJobStatesFromTransaction(t, &jtype, &jstatus);

	abt_jobInfo *ji = new abt_jobInfo(jtype, jstatus, t, a);

	history->add(ji);
}

//private static
/** \brief parses the messages of the supplied context.
 *
 * Parses all messages that are in the supplied context.
 *
 * At the moment these values were not stored nor handled by abtransfers at
 * all! They are only printed through the qDebug function of Qt.
 *
 * \returns the count of messages that could be parsed.
 */
int abt_parser::parse_ctx_messages(AB_IMEXPORTER_CONTEXT *iec)
{
	int cnt = 0;
	const char *logmsg = "PARSER - Messages: ";
	const AB_MESSAGE *msg;
	QString tmp;

	msg = AB_ImExporterContext_GetFirstMessage(iec);
	while (msg) {

		/** \todo Messages should be stored and handled!
		 *
		 * At the moment they are only printed as debug values.
		 */

#if DEBUG_ABTPARSER
		QDate date = abt_conv::GwenTimeToQDate(AB_Message_GetDateReceived(msg));
		tmp = QString("Date:\t%1").arg(date.toString(Qt::DefaultLocaleLongDate));
		qDebug() << logmsg << tmp;

		tmp = QString("Subject:\t%1").arg(AB_Message_GetSubject(msg));
		qDebug() << logmsg << tmp;

		tmp = QString("Text:\t%1").arg(AB_Message_GetText(msg));
		qDebug() << logmsg << tmp;

		int aid = AB_Message_GetAccountId(msg);
		int uid = AB_Message_GetUserId(msg);
		tmp = QString("account id: %1  -  user id: %2").arg(aid, uid);
		qDebug() << logmsg << tmp;
#endif

		msg = AB_ImExporterContext_GetNextMessage(iec);
		cnt++;
	}

	tmp = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << tmp;

	return cnt;
}

//private static
/** \brief parses the securities of the supplied context.
 *
 * Parses all securities that are in the supplied context.
 *
 * At the moment these values were not stored nor handled by abtransfers at
 * all! They are only printed as debug messages (if enabled).
 *
 * \returns the count of securities that could be parsed.
 */
int abt_parser::parse_ctx_securities(AB_IMEXPORTER_CONTEXT *iec)
{
	const char *logmsg = "PARSER - Security: ";
	const AB_SECURITY *sec;
	QString tmp;
	int cnt = 0;

	sec = AB_ImExporterContext_GetFirstSecurity(iec);
	while (sec) {

		/** \todo Securities should be stored and handled!
		 *
		 * At the moment they are only printed as debug values.
		 */

#if DEBUG_ABTPARSER
		const AB_VALUE *v;

		tmp = QString("Name:\t%1").arg(AB_Security_GetName(sec));
		qDebug() << logmsg << tmp;

		v = AB_Security_GetUnitPriceValue(sec);
		tmp = QString("UnitPriceValue:\t%1").arg(AB_Value_GetValueAsDouble(v));
		qDebug() << logmsg << tmp;
#endif

		sec = AB_ImExporterContext_GetNextSecurity(iec);
		cnt++;
	}

	tmp = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << tmp;

	return cnt;
}

//private static
/** \brief parses all account infos of the supplied context \a iec. */
int abt_parser::parse_ctx_accountInfos(AB_IMEXPORTER_CONTEXT *iec,
				       const aqb_Accounts *allAccounts,
				       abt_history *history /* = NULL */)
{
	AB_IMEXPORTER_ACCOUNTINFO *ai;
	const char *logmsg = "PARSER - Acc-Info: ";
	int cnt = 0;
	aqb_AccountInfo *acc;
	abt_history *parseHistory = nullptr;

	ai = AB_ImExporterContext_GetFirstAccountInfo(iec);
	while (ai) {
		QString accnr = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountNumber(ai));
		QString blz = QString::fromUtf8(AB_ImExporterAccountInfo_GetBankCode(ai));
		QString iban = QString::fromUtf8(AB_ImExporterAccountInfo_GetIban(ai));
		QString bic = QString::fromUtf8(AB_ImExporterAccountInfo_GetBic(ai));
		QString owner = QString::fromUtf8(AB_ImExporterAccountInfo_GetOwner(ai));
		QString name = QString::fromUtf8(AB_ImExporterAccountInfo_GetAccountName(ai));
		QString id = QString("%1").arg(AB_ImExporterAccountInfo_GetAccountId(ai));

		//check if we parse data for the "history account"
		if (history && (accnr == "0000000000") && (blz == "00000000") &&
		    (owner == "AB-Transfers")) {
			//we should parse data for the history!
			acc = nullptr;
			parseHistory = history;
		} else {
			parseHistory = nullptr;
			//get the internaly used account object (null if not found)
			acc = allAccounts->getAccount(accnr, blz, owner, name);

			if (!acc) {
				qWarning() << logmsg << "No account found! Import for: "
						     << "AccNr:" << accnr
						     << "BLZ:" << blz
						     << "( IBAN: " << iban
						     << "/ BIC: " << bic << ")"
						     << "Name:" << name
						     << "Owner:" << owner;
				//next account
				ai = AB_ImExporterContext_GetNextAccountInfo(iec);
				continue;
			}
		}

		QString tmp = QString("%1 (%3) [%2] Owner: %4 ID: %5 (IBAN: %6 BIC: %7)").arg(
				      accnr, blz, name, owner, id, iban, bic);
		qDebug() << logmsg << tmp;

		if (parseHistory) {
			//only parse supported account infos
			abt_parser::parse_ctx_ai_datedTransfers(ai, acc, parseHistory, allAccounts);
			abt_parser::parse_ctx_ai_standingOrders(ai, acc, parseHistory, allAccounts);
			abt_parser::parse_ctx_ai_transfers(ai, acc, parseHistory, allAccounts);
		} else {
			//parse all account infos
			abt_parser::parse_ctx_ai_status(ai, acc);
			abt_parser::parse_ctx_ai_datedTransfers(ai, acc);
			abt_parser::parse_ctx_ai_notedTransactions(ai, acc);
			abt_parser::parse_ctx_ai_standingOrders(ai, acc);
			abt_parser::parse_ctx_ai_transactions(ai, acc);
			abt_parser::parse_ctx_ai_transfers(ai, acc);
		}

		//next account
		ai = AB_ImExporterContext_GetNextAccountInfo(iec);
		cnt++;
	}

	return cnt;
}

//private static
/** \brief parses all account status of the supplied account info \a ai.
 *
 * Includes the bank line, noted balance, booked balance, disposable and
 * disposed values.
 *
 * These values are set at the supplied account \a acc.
 *
 * (The account status is not saved at the history, so there is no need to
 *  to add it to the definition)
 *
 * \returns the status counts at the account info.
 */
int abt_parser::parse_ctx_ai_status(AB_IMEXPORTER_ACCOUNTINFO *ai,
				    aqb_AccountInfo *acc)
{
	AB_ACCOUNT_STATUS *as;
	const char *logmsg = "PARSER - AccStats: ";
	int cnt = 0;
	QString tmp;

	as = AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
	while (as) {
		acc->setAccountStatus(as); //save the account state

#if DEBUG_ABTPARSER
		const AB_BALANCE *b;
		const AB_VALUE *v;

		QDate date = abt_conv::GwenTimeToQDate(AB_AccountStatus_GetTime(as));
		tmp = QString("Time:\t\t%1").arg(date.toString(Qt::DefaultLocaleLongDate));
		qDebug() << logmsg << tmp;

		v = AB_AccountStatus_GetBankLine(as);
		if (v) {
			tmp = QString("BankLine:\t%1").arg(
				      AB_Value_GetValueAsDouble(v), 0, 'f', 2);
			qDebug() << logmsg << tmp;
		}

		b = AB_AccountStatus_GetNotedBalance(as);
		if (b) {
			v = AB_Balance_GetValue(b);
			if (v) {
				tmp = QString("NotedBalance:\t%1").arg(
					      AB_Value_GetValueAsDouble(v), 0, 'f', 2);
				qDebug() << logmsg << tmp;
			}
		}

		b = AB_AccountStatus_GetBookedBalance(as);
		if (b) {
			v = AB_Balance_GetValue(b);
			if (v) {
				tmp = QString("BookedBalance:\t%1").arg(
					      AB_Value_GetValueAsDouble(v), 0, 'f', 2);
				qDebug() << logmsg << tmp;
			}
		}

		v = AB_AccountStatus_GetDisposable(as);
		if (v) {
			tmp = QString("Disposable:\t%1").arg(
				      AB_Value_GetValueAsDouble(v), 0, 'f', 2);
			qDebug() << logmsg << tmp;
		}

		v = AB_AccountStatus_GetDisposed(as);
		if (v) {
			tmp = QString("Disposed:\t%1").arg(
				      AB_Value_GetValueAsDouble(v), 0, 'f', 2);
			qDebug() << logmsg << tmp;
		}
#endif

		as = AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
		cnt++;
	}

	tmp = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << tmp;

	return cnt;
}

//private static
/** \brief parses all dated transfers of the supplied account info \a ai.
 *
 * Depending of the dated transfer state, the found dated transfers are added
 * or removed form the supplied account \a acc.
 *
 * If \a history is supplied != NULL, it is assumed that the parsed data should
 * be added to the history (no further checks are done!)
 *
 * \returns the count of dated transfers found at the account info.
 */
int abt_parser::parse_ctx_ai_datedTransfers(AB_IMEXPORTER_ACCOUNTINFO *ai,
					    aqb_AccountInfo *acc,
					    abt_history *history /* = nullptr */,
					    const aqb_Accounts *allAccounts /* = nullptr */)
{
	AB_TRANSACTION *t;
	const char *logmsg = "PARSER - DatedTra: ";
	QString tmp;

	int cnt = AB_ImExporterAccountInfo_GetDatedTransferCount(ai);

	tmp = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << tmp;

	t = AB_ImExporterAccountInfo_GetFirstDatedTransfer(ai);
	while (t) {
		if (history) { //parse history data
			Q_ASSERT(allAccounts);
			abt_parser::addJobInfoToHistory(history, allAccounts, t,
							AB_Job_TypeCreateDatedTransfer,
							AB_Job_StatusFinished);
		} else { //parse received data
			//add or remove the dated transfers to the accuont,
			//depending on its supplied state.
			abt_datedTransferInfo *dt = new abt_datedTransferInfo(t);
			if (dt->getTransaction()->getStatus() == AB_Transaction_StatusRevoked) {
				//accounts only have existing dated transfers
				//(the history possible show other values too)
				acc->removeDatedTransfer(dt);
				delete dt; //no longer used
			} else {
				//the account gets the owner of the dt!
				acc->addDatedTransfer(dt);
			}
		}

#if DEBUG_ABTPARSER
		const GWEN_STRINGLIST *sl;
		QStringList strList;

		sl = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("Purpose:\t\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		const AB_VALUE *v = AB_Transaction_GetValue(t);
		if (v) {
			tmp = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));
		} else {
			tmp = QString("Value:\t\tNOT SET!");
		}
		qDebug() << logmsg << tmp;

		sl = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("RemoteName:\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
		tmp = QString("Status:\t\t%1 (%2)").arg(state).arg(
				AB_Transaction_Status_toString(state));
		qDebug() << logmsg << tmp;
#endif

		t = AB_ImExporterAccountInfo_GetNextDatedTransfer(ai);
	}

	return cnt;
}

//private static
/** \brief parses all noted transactions of the supplied account info \a ai.
 *
 * Only prints the values at the debug output (if enabled). If the transactions
 * should be shown and handled by abtransfers then this must be implemented.
 *
 * (The noted transactions are not saved at the history, so there is no need to
 *  to add it to the definition)
 *
 * \returns the count of noted transactions found at the account info.
 */
int abt_parser::parse_ctx_ai_notedTransactions(AB_IMEXPORTER_ACCOUNTINFO *ai,
					       aqb_AccountInfo* /* acc */)
{
	AB_TRANSACTION *t;
	const char *logmsg = "PARSER - NotedTra: ";
	QString tmp;

	int cnt = AB_ImExporterAccountInfo_GetNotedTransactionCount(ai);

	tmp = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << tmp;

	t = AB_ImExporterAccountInfo_GetFirstNotedTransaction(ai);
	while (t) {

		/** \todo implement the setting at the account if it should
		 *        be supported.
		 */

#if DEBUG_ABTPARSER
		const GWEN_STRINGLIST *sl;
		QStringList strList;

		sl = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("Purpose:\t\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		const AB_VALUE *v = AB_Transaction_GetValue(t);
		tmp = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));
		qDebug() << logmsg << tmp;

		sl = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("RemoteName:\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
		tmp = QString("Status:\t\t%1 (%2)").arg(state).arg(
				AB_Transaction_Status_toString(state));
		qDebug() << logmsg << tmp;
#endif

		t = AB_ImExporterAccountInfo_GetNextNotedTransaction(ai);
	}

	return cnt;
}

//private static
/** \brief parses all standing orders of the supplied account info \a ai.
 *
 * Depending of the standing order state, the found standing orders are added
 * or removed form the supplied account \a acc.
 *
 * If \a history is supplied != NULL, it is assumed that the parsed data should
 * be added to the history (no further checks are done!)
 *
 * \returns the count of standing orders found at the account info.
 */
int abt_parser::parse_ctx_ai_standingOrders(AB_IMEXPORTER_ACCOUNTINFO *ai,
					    aqb_AccountInfo *acc,
					    abt_history *history /* = NULL */,
					    const aqb_Accounts *allAccounts /* = NULL */)
{
	AB_TRANSACTION *t;
	const char *logmsg = "PARSER - Standing: ";
	QString tmp;

	int cnt = AB_ImExporterAccountInfo_GetStandingOrderCount(ai);

	tmp = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << tmp;

	t = AB_ImExporterAccountInfo_GetFirstStandingOrder(ai);
	while (t) {
		if (history) { //parse for the history
			Q_ASSERT(allAccounts);
			abt_parser::addJobInfoToHistory(history, allAccounts, t,
							AB_Job_TypeCreateStandingOrder,
							AB_Job_StatusFinished);
		} else {
			//add or remove the standing order to the accuont, depending
			//on its supplied state.
			abt_standingOrderInfo *so = new abt_standingOrderInfo(t);
			if (so->getTransaction()->getStatus() == AB_Transaction_StatusRevoked) {
				//accounts only have existing standing orders
				//(the history possible show other values too)
				acc->removeStandingOrder(so);
				delete so; //no longer used
			} else {
				//the account gets the owner of the so and deletes it!
				acc->addStandingOrder(so);
			}
		}

#if DEBUG_ABTPARSER
		const GWEN_STRINGLIST *sl;
		QStringList strList;

		sl = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("Purpose:\t\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		const AB_VALUE *v = AB_Transaction_GetValue(t);
		tmp = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));
		qDebug() << logmsg << tmp;

		sl = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("RemoteName:\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
		tmp = QString("Status:\t\t%1 (%2)").arg(state).arg(
				AB_Transaction_Status_toString(state));
		qDebug() << logmsg << tmp;
#endif

		t = AB_ImExporterAccountInfo_GetNextStandingOrder(ai);
	}

	return cnt;
}

//private static
/** \brief parses all transactions of the supplied account info \a ai.
 *
 * Only prints the values at the debug output (if enabled). If the transactions
 * should be shown and handled by abtransfers then this must be implemented.
 *
 * (The transactions are not saved at the history, so there is no need to
 *  to add it to the definition)
 *
 * \returns the count of transactions found at the account info.
 */
int abt_parser::parse_ctx_ai_transactions(AB_IMEXPORTER_ACCOUNTINFO *ai,
					  aqb_AccountInfo* /* acc */)
{
	AB_TRANSACTION *t;
	const char *logmsg = "PARSER - Transact: ";
	QString tmp;

	int cnt = AB_ImExporterAccountInfo_GetTransactionCount(ai);
	tmp = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << tmp;

	t = AB_ImExporterAccountInfo_GetFirstTransaction(ai);
	while (t) {

		/** \todo implement the setting at the account if it should
		 *        be supported.
		 */

#if DEBUG_ABTPARSER
		const GWEN_STRINGLIST *sl;
		QStringList strList;

		sl = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("Purpose:\t\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		const AB_VALUE *v = AB_Transaction_GetValue(t);
		tmp = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));
		qDebug() << logmsg << tmp;

		sl = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("RemoteName:\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
		tmp = QString("Status:\t\t%1 (%2)").arg(state).arg(
				AB_Transaction_Status_toString(state));
		qDebug() << logmsg << tmp;
#endif

		t = AB_ImExporterAccountInfo_GetNextTransaction(ai);
	}

	return cnt;
}

//private static
/** \brief parses all transfers of the supplied account info \a ai.
 *
 * Transfers are only shown at the history!
 *
 * If \a history is supplied != NULL, it is assumed that the parsed data should
 * be added to the history (no further checks are done!)
 *
 * With no \a history supplied the values are only printed as debug output
 * (if enabled). If the transfers should be shown and handled by abtransfers
 * then this must be implemented.
 *
 * \returns the count of transfers found at the account info.
 */
int abt_parser::parse_ctx_ai_transfers(AB_IMEXPORTER_ACCOUNTINFO *ai,
				       aqb_AccountInfo* /* acc */,
				       abt_history *history /* = NULL */,
				       const aqb_Accounts *allAccounts /* = NULL */)
{
	AB_TRANSACTION *t;
	const char *logmsg = "PARSER - Transfer: ";
	QString tmp;

	int cnt = AB_ImExporterAccountInfo_GetTransferCount(ai);

	tmp = QString("Count: %1").arg(cnt);
	qDebug() << logmsg << tmp;

	t = AB_ImExporterAccountInfo_GetFirstTransfer(ai);
	while (t) {
		if (history) { //parse for history
			Q_ASSERT(allAccounts);
			abt_parser::addJobInfoToHistory(history, allAccounts, t,
							AB_Job_TypeTransfer,
							AB_Job_StatusFinished);
		} else {
			/** \todo implement the setting at the account if
			 *        it should be supported.
			 */
		}

#if DEBUG_ABTPARSER
		const GWEN_STRINGLIST *sl;
		QStringList strList;

		sl = AB_Transaction_GetPurpose(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("Purpose:\t\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		const AB_VALUE *v = AB_Transaction_GetValue(t);
		tmp = QString("Value:\t\t%1").arg(AB_Value_GetValueAsDouble(v));
		qDebug() << logmsg << tmp;

		sl = AB_Transaction_GetRemoteName(t);
		strList = abt_conv::GwenStringListToQStringList(sl);
		tmp = QString("RemoteName:\t%1").arg(strList.join(" - "));
		qDebug() << logmsg << tmp;

		AB_TRANSACTION_STATUS state = AB_Transaction_GetStatus(t);
		tmp = QString("Status:\t\t%1 (%2)").arg(state).arg(
				AB_Transaction_Status_toString(state));
		qDebug() << logmsg << tmp;
#endif

		t = AB_ImExporterAccountInfo_GetNextTransfer(ai);
	}

	return cnt;
}

//constructor
/** \brief default constructor, does nothing
 *
 * all needed functions are static and an instantiation of the class is not
 * needed yet.
 */
abt_parser::abt_parser()
{
}


//static
/** The caller is responsible for freeing the returned context (e.g. with
 * AB_ImExporterContext_free()).
 */
AB_IMEXPORTER_CONTEXT *abt_parser::load_local_ctx(const QString &filename,
						  const QString &importerName,
						  const QString &profileName)
{
	AB_IMEXPORTER_CONTEXT *ctx = AB_ImExporterContext_new();

	AB_Banking_ImportFileWithProfile(banking->getAqBanking(),
					 importerName.toUtf8(),
					 ctx,
					 profileName.toUtf8(),
					 nullptr,
					 filename.toUtf8());

	//try to fill gaps (e.g. the account id)
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
		//error occurred!
		qWarning() << Q_FUNC_INFO << "ERROR:" << ret
			   << " -- something went wrong on storing the ctx to file";
	}

}

//static
/**
  * The caller is responsible to free the returned context (e.g. trough
  * AB_ImExporterContext_free() )
  *
  * \returns an AB_IMEXPORTER_CONTEXT with all values from \a allAccounts
  * \returns NULL if \a allAccounts is not valid!
  */
AB_IMEXPORTER_CONTEXT *abt_parser::create_ctx_from(const aqb_Accounts *allAccounts)
{
	if (!allAccounts) return nullptr; //no accounts, cant work

	AB_IMEXPORTER_CONTEXT *ctx = AB_ImExporterContext_new();

	//go trough all known accounts and add the IEC to the returned one
	QHashIterator<int, aqb_AccountInfo*> it(allAccounts->getAccountHash());
	while (it.hasNext()) {
		it.next();
		qDebug() << it.key() << ": " << it.value();
		AB_ImExporterContext_AddContext(ctx, it.value()->getContext());
	}

	return ctx;
}

//static
/**
 * This is the main function that should be used to parse any data of the
 * supplied context \a iec.
 *
 * This context could be returned from the bank or the saved values of the
 * history of abtransfers.
 *
 * If data for the history should be paresed the abt_history object must be
 * supplied as \a history (default is NULL).
 */
void abt_parser::parse_ctx(AB_IMEXPORTER_CONTEXT *iec, const aqb_Accounts *allAccounts,
			   abt_history *history /* = NULL */)
{
	if (!iec) {
		//if no valid context is supplied we have nothing to do
		return;
	}

	if (!allAccounts) {
		//we need the accounts to store the vales (even for the history
		//import)
		return;
	}

	//fill possible gaps (e.g. bankname, account number etc)
	int ret = AB_Banking_FillGapsInImExporterContext(banking->getAqBanking(),
							 iec);
	if (ret) {
		qWarning() << Q_FUNC_INFO << "ERROR =" << ret
			   << " -- something went wrong on filling the gaps"
			   << "(this is not an serious error)";
	}

	//messages and securities are not supported at the history, so there
	//is no need to supply it to them (at the moment)
	abt_parser::parse_ctx_messages(iec);
	abt_parser::parse_ctx_securities(iec);

	//parse all types of transfers for the accounts (or the history)
	abt_parser::parse_ctx_accountInfos(iec, allAccounts, history);
}
