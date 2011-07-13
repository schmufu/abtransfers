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

#ifndef TRANS_JOB_H
#define TRANS_JOB_H

#include <QString>
#include <QStringList>
#include <QDate>

class trans_job
{
private:
	//Local Account Info
	QString LocalCountry;
	QString LocalBankCode;
	QString LocalBranchId;
	QString LocalAccountNumber;
	QString LocalSuffix;
	QString LocalIban;
	QString LocalName;
	QString LocalBic;
	//Remote Account Info
	QString RemoteCountry;
	QString RemoteBankName;
	QString RemoteBankLocation;
	QString RemoteBankCode;
	QString RemoteBranchId;
	QString RemoteAccountNumber;
	QString RemoteSuffix;
	QString RemoteIban;
	QString RemoteName;
	QString RemoteBic;
	QString UniqueId;
	QString IdForApplication; //!< Unique ID set by the Application, not changed by aqBanking
	QString GroupId;
	//Dates
	QString ValutaDate; //!< Wertstellung (Datum der wirklichen Ausführung)
	QString Date; //!< Buchungsdatum (Datum der Erstellung)
	//Value
	QString Value;
	QString Fees;
	//Info which is Not Supported by all Backends
	QString TextKey; //!< Textschlüssel (Überweisung 51, debit note 04 or 05) [HBCI only]
	QString TextKeyExt; //!< Textschlüsselergänzung (Überweisung 51, debit note 04 or 05) [HBCI only]
	QString TransactionKey; //!< Buchungsschlüssel [HBCI only]
	QString CustomerReference; //!< Kundenreferenz
	QString BankReference; //!< Bankreferenz
	QString EntToEndReference; //!< This is a reference provided by the issuer of a SEPA transfer.
	QString MandateReference;
	QString CreditorIdentifier; //!< used for SEPA transfers
	QString OriginatorIdentifier; //!< used for SEPA transfers
	int TransactionCode; //!< Geschäftsvorfallcode (3 digit numerical transaction code)
	QString TransactionText;
	QString Promanota;
	QString FiId;
	QStringList Purpose; //!< every entry represents a single purpose line
	QStringList Category;
	//Additional Information for Standing Orders (Daueraufträge)
	QString Period; //!< weekly or monthly
	QString Cycle; //!< executet every cycle * period (period=weekly and cycle=2 ==> executed every 2 weeks)
	QString ExecutionDay; /*!< meaning depends on the content of period
				- monthly: day of the month (starting with 1)
				- weekly: day of the week (startin with 1=Monday)
				*/
	QDate FirstExecutionDate;
	QDate LastExecutionDate;
	QDate NextExecutionDate;
	//Additional Information for Transfers (Überweisungen)
	QString Type; //!< transfer, debit note etc.
	QString SubType;
	QString Status; //!< accepted, rejected, pending etc.
	QString Charge;
	//Additional Information for Foreign Transfers
	QString RemoteAddrStreet;
	QString RemoteAddZipcode;
	QString RemoteAddrCity;
	QString RemotePhone;
	//Additional Information for Investment Transfers
	QString UnitId;
	QString InitIdNameSpace;
	QString Units;
	QString UnitPrice;
	QString Commission;

public:
    trans_job();
};

#endif // TRANS_JOB_H
