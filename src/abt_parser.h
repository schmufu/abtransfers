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

#ifndef ABT_PARSER_H
#define ABT_PARSER_H

#include <aqbanking/imexporter.h>

#include <aqbanking/job.h>
#include <aqbanking/transaction.h>

#include <aqbanking/jobsingletransfer.h>
#include <aqbanking/jobsingledebitnote.h>
#include <aqbanking/jobinternaltransfer.h>
#include <aqbanking/jobeutransfer.h>
#include <aqbanking/jobsepatransfer.h>
#include <aqbanking/jobsepadebitnote.h>
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>
#include <aqbanking/jobloadcellphone.h>

#include <aqbanking/jobcreatedatedtransfer.h>
#include <aqbanking/jobmodifydatedtransfer.h>
#include <aqbanking/jobdeletedatedtransfer.h>
#include <aqbanking/jobgetdatedtransfers.h>

#include <aqbanking/jobcreatesto.h>
#include <aqbanking/jobmodifysto.h>
#include <aqbanking/jobdeletesto.h>
#include <aqbanking/jobgetstandingorders.h>

#include "aqb_accounts.h"
#include "abt_history.h"


#ifndef DEBUG_ABTPARSER
/** \brief DEBUG_ABTPARSER should be 1 to enable debug output of the parser!
 *
 * It could also be set with -DDEBUG_ABTPARSER=1 as a compiler parameter to
 * enable it for a test compilation only!
*/
//disable debug messages if not supplied as compiler define
#define DEBUG_ABTPARSER 0
#endif


/** \brief parser for the local saved data and for the data retrieved from
 *         the bank.
 *
 * With \ref load_local_ctx() local saved data can be loaded (returned as
 * AB_IMEXPORTER_CONTEXT).
 *
 * Every AB_IMEXPORTER_CONTEXT [ctx] can be parsed with \ref parse_ctx().
 * \ref parse_ctx() sets all read values from the supplied ctx at the
 * corresponding objects.
 *
 * \ref parse_ctx() could also be used to parse the information retrieved from
 * the bank and update the corresponding objects.
 *
 */
class abt_parser
{
private:
	/** \brief reads the type and status from the category field */
	static void getJobStatesFromTransaction(AB_TRANSACTION *t,
						AB_JOB_TYPE *jobType,
						AB_JOB_STATUS *jobStatus);
	static void addJobInfoToHistory(abt_history *history,
					const aqb_Accounts *allAccounts,
					AB_TRANSACTION *t, AB_JOB_TYPE defType,
					AB_JOB_STATUS defStatus);

	//private functions to parse the different possible contents of a
	//AB_IMEXPORTER_CONTEXT object
	static int parse_ctx_messages(AB_IMEXPORTER_CONTEXT *iec);
	static int parse_ctx_securities(AB_IMEXPORTER_CONTEXT *iec);
	static int parse_ctx_accountInfos(AB_IMEXPORTER_CONTEXT *iec,
					  aqb_Accounts *allAccounts,
					  abt_history *history = NULL);

	static int parse_ctx_ai_status(AB_IMEXPORTER_ACCOUNTINFO *ai,
				       aqb_AccountInfo *acc);
	static int parse_ctx_ai_datedTransfers(AB_IMEXPORTER_ACCOUNTINFO *ai,
					       aqb_AccountInfo *acc,
					       abt_history *history = NULL,
					       const aqb_Accounts *allAccounts = NULL);
	static int parse_ctx_ai_notedTransactions(AB_IMEXPORTER_ACCOUNTINFO *ai,
						  aqb_AccountInfo *acc);
	static int parse_ctx_ai_standingOrders(AB_IMEXPORTER_ACCOUNTINFO *ai,
					       aqb_AccountInfo *acc,
					       abt_history *history = NULL,
					       const aqb_Accounts *allAccounts = NULL);
	static int parse_ctx_ai_transactions(AB_IMEXPORTER_ACCOUNTINFO *ai,
					     aqb_AccountInfo *acc);
	static int parse_ctx_ai_transfers(AB_IMEXPORTER_ACCOUNTINFO *ai,
					  aqb_AccountInfo *acc,
					  abt_history *history = NULL,
					  const aqb_Accounts *allAccounts = NULL);

public:
	abt_parser();

	/** \brief creates an AB_IMEXPORTER_CONTEXT for all aqb_AccountInfo
	 *         objects of the aqb_Accounts object.
	 */
	static AB_IMEXPORTER_CONTEXT *create_ctx_from(const aqb_Accounts *allAccounts);

	/** \brief loads all data from the \a filename with the \a importerName
	 *         and \a profileName to the returned context.
	 */
	static AB_IMEXPORTER_CONTEXT *load_local_ctx(const QString &filename,
						     const QString &importerName,
						     const QString &profileName);

	/** \brief saves all data of the \a ctx to the given \a filename.
	 */
	static void save_local_ctx(AB_IMEXPORTER_CONTEXT *ctx,
				   const QString &filename,
				   const QString &exporterName,
				   const QString &profileName);

	/** \brief parses the supplied context \a iec and stores the relevant
	 *         data to the matching accounts at \a allAccounts
	 */
	static void parse_ctx(AB_IMEXPORTER_CONTEXT *iec,
			      aqb_Accounts *allAccounts,
			      abt_history *history = NULL);

};

#endif // ABT_PARSER_H
