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


/** \brief parser zum laden der lokal gespeicherten Daten sowie zum Auswerten
  *        der vom Institut gelieferten Daten.
  *
  * Über \ref load_local_ctx() können die Daten einer lokal gespeicherten Datei
  * in den Context (Rückgabewert) geladen werden.
  *
  * Dieser Context [ctx] kann dann über \ref parse_ctx() ausgewertet werden.
  * \ref parse_ctx() setzt dabei alle Werte die im Context gefunden wurden in
  * den entsprechenden Objekten.
  *
  * Außerdem kann \ref parse_ctx() dazu verwendet werden die gelieferten Daten
  * des Instituts aus zu werten und die entsprechenden Objekte zu aktualisieren.
  *
  */
class abt_parser
{
private:
	//static AB_IMEXPORTER_CONTEXT *ctx;

public:
	abt_parser();

	/** \brief erstellt einen AB_IMEXPORTER_CONTEXT für alle in aqb_Accounts
	  * enhaltenen aqb_AccountInfo Objekte
	  */
	static AB_IMEXPORTER_CONTEXT *create_ctx_from(const aqb_Accounts *allAccounts);


	/** \brief lädt alle Daten der Datei \a filename mit dem Importer
	  * \a importerName und dem Profile \a profileName in den \return ctx
	  */
	static AB_IMEXPORTER_CONTEXT *load_local_ctx(const QString &filename,
						     const QString &importerName,
						     const QString &profileName);

	/** \brief speichert alle Daten des IE-Context \a ctx in der Datei
	  * \a filename mit dem Importer \a importerName und dem Profile
	  * \a profileName
	  */
	static void save_local_ctx(AB_IMEXPORTER_CONTEXT *ctx,
				   const QString &filename,
				   const QString &exporterName,
				   const QString &profileName);


	/** \brief parst den Context \a iec und setzt die entsprechenden Werte
	  * des entsprechenden Accounts
	  */
	static void parse_ctx(AB_IMEXPORTER_CONTEXT *iec, aqb_Accounts *allAccounts);

	/** \overload zur Verwendung wenn die History geladen werden soll
	  */
	static void parse_ctx(AB_IMEXPORTER_CONTEXT *iec,
			      const aqb_Accounts *allAccounts,
			      abt_history *history);

};

#endif // ABT_PARSER_H
