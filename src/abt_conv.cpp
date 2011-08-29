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

#include "abt_conv.h"
#include <QDebug>
#include <string>
#include <stdio.h>

//initialize static member variables
QList<GWEN_STRINGLIST*> *abt_conv::gwen_lists = new QList<GWEN_STRINGLIST*>;
QList<GWEN_TIME*> *abt_conv::gwen_timelist = new QList<GWEN_TIME*>;


abt_conv::abt_conv()
{
}

/*****************************************************************************
 * static helper functions for type conversions                              *
 *****************************************************************************/

//static
const QString abt_conv::JobTypeToQString(const AB_JOB *j)
{
	AB_JOB_TYPE type = AB_Job_GetType(j);
	return abt_conv::JobTypeToQString(type);
}

const QString abt_conv::JobTypeToQString(AB_JOB_TYPE type)
{
	switch (type) {
	case AB_Job_TypeCreateDatedTransfer:
		return (QObject::tr("Terminüberweisung anlegen"));
		break;
	case AB_Job_TypeCreateStandingOrder :
		return (QObject::tr("Dauerauftrag anlegen"));
		break;
	case AB_Job_TypeDebitNote :
		return (QObject::tr("Lastschrift anlegen"));
		break;
	case AB_Job_TypeDeleteDatedTransfer :
		return (QObject::tr("Terminüberweisung löschen"));
		break;
	case AB_Job_TypeDeleteStandingOrder :
		return (QObject::tr("Dauerauftrag löschen"));
		break;
	case AB_Job_TypeEuTransfer :
		return (QObject::tr("Internationale Überweisung"));
		break;
	case AB_Job_TypeGetBalance :
		return (QObject::tr("Kontostand abfragen"));
		break;
	case AB_Job_TypeGetDatedTransfers :
		return (QObject::tr("Terminüberweisungen abfragen"));
		break;
	case AB_Job_TypeGetStandingOrders :
		return (QObject::tr("Daueraufträge abfragen"));
		break;
	case AB_Job_TypeGetTransactions :
		return (QObject::tr("Buchungen abfragen"));
		break;
	case AB_Job_TypeInternalTransfer :
		return (QObject::tr("Umbuchung durchführen"));
		break;
	case AB_Job_TypeLoadCellPhone :
		return (QObject::tr("Handy Prepaid-Karte aufladen"));
		break;
	case AB_Job_TypeModifyDatedTransfer :
		return (QObject::tr("Terminüberweisung ändern"));
		break;
	case AB_Job_TypeModifyStandingOrder :
		return (QObject::tr("Dauerauftrag ändern"));
		break;
	case AB_Job_TypeSepaDebitNote :
		return (QObject::tr("SEPA Lastschrift anlegen"));
		break;
	case AB_Job_TypeSepaTransfer :
		return (QObject::tr("SEPA Überweisung"));
		break;
	case AB_Job_TypeTransfer :
		return (QObject::tr("Überweisung durchführen"));
		break;
	case AB_Job_TypeUnknown :
		return (QObject::tr("AqBanking Typ unbekannt"));
		break;
	}

	return QObject::tr("ab_transfers Typ unbekannt");
}

//static
const QString abt_conv::JobStatusToQString(const AB_JOB *j)
{
	AB_JOB_STATUS status = AB_Job_GetStatus(j);
	return abt_conv::JobStatusToQString(status);
}

//static
const QString abt_conv::JobStatusToQString(AB_JOB_STATUS status)
{
	switch (status) {
	case AB_Job_StatusEnqueued:
		return QObject::tr("Enqueued");
		break;
	case AB_Job_StatusError:
		return QObject::tr("Error");
		break;
	case AB_Job_StatusFinished:
		return QObject::tr("Finished");
		break;
	case AB_Job_StatusNew:
		return QObject::tr("Neu");
		break;
	case AB_Job_StatusPending:
		return QObject::tr("Pending");
		break;
	case AB_Job_StatusSent:
		return QObject::tr("Sent");
		break;
	case AB_Job_StatusUnknown:
		return QObject::tr("aqBanking status unbekannt");
		break;
	case AB_Job_StatusUpdated:
		return QObject::tr("Updated");
		break;
	}

	return QObject::tr("ab_transfer status unbekannt");
}

//static
const QDate abt_conv::GwenTimeToQDate(const GWEN_TIME *gwentime)
{
	QDate date;
	if (gwentime) {
		struct tm tmtime;
		tmtime = GWEN_Time_toTm(gwentime);
		date.setDate(tmtime.tm_year+1900, tmtime.tm_mon+1, tmtime.tm_mday);
	} else {
		date.setDate(2011,2,30); //invalid Date wenn gwen_date==NULL
	}

	return date;
}

/**
  * Gibt einen GWEN_TIME Object zurück dessen Datum dem übergebenen entspricht.
  * Die enthaltene Uhrzeit wird auf 10:00:00 gesetzt! (in localTime, nicht UTC!)
  */
//static
GWEN_TIME* abt_conv::QDateToGwenTime(const QDate &date)
{
	GWEN_TIME *gwt;
	gwt = GWEN_Time_new(date.year(), date.month()-1, date.day(), 10, 0, 0, 0);
	abt_conv::gwen_timelist->append(gwt);
	return gwt;
}

//static
const QStringList abt_conv::GwenStringListToQStringList(const GWEN_STRINGLIST *gwenList)
{
	QStringList ret;
	ret.clear();
	for (unsigned int i=0; i<GWEN_StringList_Count(gwenList); ++i) {
		ret.append(QString::fromUtf8(GWEN_StringList_StringAt(gwenList, i)));
	}
	return ret;
}

/**
  * Die hier erstellte GWEN_STRINGLIST wird später durch freeAllGwenStringLists()
  * wieder freigegeben!
  */
//static
const GWEN_STRINGLIST *abt_conv::QStringListToGwenStringList(const QStringList &l)
{
	GWEN_STRINGLIST *gwl = GWEN_StringList_new();
	for (int i=0; i<l.size(); ++i) {
		QString s = l.at(i);
		//wir reservieren Speicher für einen "normalen" C-String, damit
		//GWEN_StringList_free diesen auch wieder freigeben kann.
		char *c = (char*)malloc(sizeof(char)*s.toStdString().length()+1);
		//unseren String in den erstellten Speicherbereich kopieren
		strcpy(c, s.toStdString().c_str());
		//und der GWEN_Liste hinzufügen. Das Löschen der GWEN_Liste
		//gibt später auch den C-String wieder frei.
		GWEN_StringList_AppendString(gwl, c, 1, 0);
	}
	//Die erstellte GWEN_Stringlist in unserer globalen Liste aufbewahren
	Q_ASSERT(abt_conv::gwen_lists != NULL);
	abt_conv::gwen_lists->append(gwl);
	return gwl;
}

/*! wird genutzt um die Werte als String in der ini-Datei zu speichern
 *
 * wenn der String als Decimal-Wert dargestellt werden soll muss \a asDecimal
 * = true übergeben werden.
 */
//static
const QString abt_conv::ABValueToString(const AB_VALUE *v, bool asDecimal)
{
	if (v == NULL) {
		return QString();
	}
	if (asDecimal) {
		return QString("%L1").arg(AB_Value_GetValueAsDouble(v),0,'f',2);
	} else {
		GWEN_BUFFER *buf = GWEN_Buffer_new(NULL, 100, 0, 0);
		AB_Value_toString(v, buf);
		std::string result(GWEN_Buffer_GetStart(buf));
		GWEN_Buffer_free(buf);
		return QString::fromStdString(result);
	}
}

/*! wird genutzt um die als String gespeicherten Werte aus der ini-Datei zu lesen
  *
  * This function reads a AB_VALUE from a string. Strings suitable as arguments
  * are those created by AB_Value_toString or simple floating point string (as
  * in "123.45" or "-123.45").
  *
  * Die Währung kann in \a currency auch mit angegeben werden (default: "EUR")
  */
//static
AB_VALUE *abt_conv::ABValueFromString(const QString &str, const QString &currency)
{
	//! \todo Die hier erstellen AB_VALUE Objekte müssen am Ende wieder freigegeben werden!
	if (str.isEmpty()) {
		return NULL;
	}
	std::string s = str.toStdString();
	AB_VALUE *val;
	val = AB_Value_fromString(s.c_str());
	QString cur = currency.toUtf8();
	std::string c = cur.toStdString();
	AB_Value_SetCurrency(val, c.c_str());
	return val;
}

/*! löscht alle erstelten GWEN_STRINGLISTs und GWEN_TIMEs wieder aus dem speicher
 *
 * Dies darf erst am Ende des Programms erfolgen, ansonsten werden Speicherbereiche
 * gelöscht die noch in verwendung sind!
 */
//static
void abt_conv::freeAllGwenLists()
{
	GWEN_STRINGLIST *list;
	while (!abt_conv::gwen_lists->isEmpty()) {
		list = abt_conv::gwen_lists->takeFirst();
		qDebug() << "freeing GWEN_StringList: " << list;
		GWEN_StringList_free(list);
	}

	GWEN_TIME *gwt;
	while (!abt_conv::gwen_timelist->isEmpty()) {
		gwt = abt_conv::gwen_timelist->takeFirst();
		GWEN_Time_free(gwt);
	}

	//Globale Listen auch löschen
	delete abt_conv::gwen_lists;
	delete abt_conv::gwen_timelist;
}
