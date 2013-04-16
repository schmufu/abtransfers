/******************************************************************************
 * Copyright (C) 2011-2013 Patrick Wacker
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
QList<GWEN_STRINGLIST*> *abt_conv::gwen_strlist = new QList<GWEN_STRINGLIST*>;
QList<GWEN_TIME*> *abt_conv::gwen_timelist = new QList<GWEN_TIME*>;
QList<AB_VALUE*> *abt_conv::gwen_abvlist = new QList<AB_VALUE*>;

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

	//default if no type match
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
/** @brief converts a GWEN_TIME to QDate
 *
 * Converts the time supplied at @a gwentime into a QDate.
 *
 * If the @a gwentime is NULL, the returned QDate is set to an invalid date.
 */
const QDate abt_conv::GwenTimeToQDate(const GWEN_TIME *gwentime)
{
	QDate date;
	if (gwentime) {
		GWEN_BUFFER *gbuf = GWEN_Buffer_new(NULL, 100, 0, 0);

		GWEN_Time_toUtcString(gwentime, "YYYYMMDD", gbuf);
		std::string stdDatetime(GWEN_Buffer_GetStart(gbuf));
		QString strDate = QString::fromStdString(stdDatetime);
		date = QDate::fromString(strDate, "yyyyMMdd");
		GWEN_Buffer_free(gbuf);

	} else {
		date.setDate(2011,2,30); //invalid date if gwentime == NULL
	}

	return date;
}

//static
/** @brief converts a QDate to a GWEN_TIME
 *
 * Returns a pointer to a GWEN_TIME object or NULL if the supplied @a date is
 * invalid.
 *
 * The time of the returned GWEN_TIME is set to 12:00:00 (UTC!).
 *
 * The returned pointer must not be freed! It is freed automatically at the
 * termination of the program by calling @ref freeAllGwenLists()
 */
const GWEN_TIME* abt_conv::QDateToGwenTime(const QDate &date)
{
	GWEN_TIME *gwt;
	QString datestr;

	if (!date.isValid()) {
		return NULL;
	}

	datestr = QString("%1%2%3")
		  .arg(date.year(), 4, 10, QLatin1Char('0'))
		  .arg(date.month(), 2, 10, QLatin1Char('0'))
		  .arg(date.day(), 2, 10, QLatin1Char('0'));

	datestr.append("-12:00");

	gwt = GWEN_Time_fromUtcString(datestr.toStdString().c_str(), "YYYYMMDD-hh:mm");
	//gwt = GWEN_Time_new(date.year(), date.month()-1, date.day(), 12, 0, 0, 1);

	Q_ASSERT(abt_conv::gwen_timelist);
	abt_conv::gwen_timelist->append(gwt);

	return gwt;
}

//static
const QStringList abt_conv::GwenStringListToQStringList(const GWEN_STRINGLIST *gwenList)
{
	Q_ASSERT(gwenList);
	QStringList ret;

	for (unsigned int i=0; i<GWEN_StringList_Count(gwenList); ++i) {
		ret.append(QString::fromUtf8(GWEN_StringList_StringAt(gwenList, i)));
	}

	return ret;
}

//static
/** @brief converts a QStringList to a GWEN_STRINGLIST
 *
 * The returned pointer must not be freed! It is freed automatically at the
 * termination of the program by calling @ref freeAllGwenLists()
  */
const GWEN_STRINGLIST* abt_conv::QStringListToGwenStringList(const QStringList &l)
{
	Q_ASSERT(abt_conv::gwen_strlist);

	GWEN_STRINGLIST *gwl = GWEN_StringList_new();
	for (int i=0; i<l.size(); ++i) {
		QString s = l.at(i);
		//we allocate memory for a normal c string, so that
		//GWEN_StringList_free() can free it
		char *c = (char*)malloc(sizeof(char)*s.toStdString().length()+1);
		strcpy(c, s.toStdString().c_str());
		//GWEN_StringList_free() will also free the allocated c string
		GWEN_StringList_AppendString(gwl, c, 1, 0);
	}
	//remember the GWEN_STRINGLIST, so that freeAllGwenLists() can delete it
	abt_conv::gwen_strlist->append(gwl);
	return gwl;
}

//static
/** @brief Converts an AB_VALUE to a QString
 *
 * If @a asDecimal is true the returned string is formated as "xxx.yy",
 * otherwise the string has the format "xxxxx/100".
 *
 * If the supplied @a value is NULL an empty QString is returned.
 */
const QString abt_conv::ABValueToString(const AB_VALUE *value, bool asDecimal)
{
	if (!value) {
		return QString();
	}

	if (asDecimal) {
		return QString("%L1").arg(AB_Value_GetValueAsDouble(value),0,'f',2);
	} else {
		GWEN_BUFFER *buf = GWEN_Buffer_new(NULL, 100, 0, 0);
		AB_Value_toString(value, buf);
		std::string result(GWEN_Buffer_GetStart(buf));
		GWEN_Buffer_free(buf);
		return QString::fromStdString(result);
	}
}

//static
/** @brief converts a QString to an AB_VALUE
  *
  * This function reads a AB_VALUE from a string. Strings suitable as arguments
  * are those created by AB_Value_toString or simple floating point string (as
  * in "123.45" or "-123.45").
  *
  * Returns NULL if the supplied @a str is empty!
  */
AB_VALUE* abt_conv::ABValueFromString(const QString &str, const QString &currency)
{
	Q_ASSERT(abt_conv::gwen_abvlist);

	if (str.isEmpty()) {
		return NULL;
	}

	std::string s = str.toStdString();
	AB_VALUE *val;
	val = AB_Value_fromString(s.c_str());
	QString cur = currency.toUtf8();
	std::string c = cur.toStdString();
	AB_Value_SetCurrency(val, c.c_str());
	//remember the AB_VALUE, so that freeAllGwenLists() can delete it
	abt_conv::gwen_abvlist->append(val);
	return val;
}

/** @brief must be called at the termination of the program so that ALL created
 *  list are deleted.
 *
 * This function must only be called at the termination of the program,
 * otherwise references are deleted that still in use.
 */
//static
void abt_conv::freeAllGwenLists()
{
	qDebug() << Q_FUNC_INFO << "freeing GWEN_STRINGLIST list";
	GWEN_STRINGLIST *list;
	while (!abt_conv::gwen_strlist->isEmpty()) {
		list = abt_conv::gwen_strlist->takeFirst();
		GWEN_StringList_free(list);
	}

	qDebug() << Q_FUNC_INFO << "freeing GWEN_TIME list";
	GWEN_TIME *gwt;
	while (!abt_conv::gwen_timelist->isEmpty()) {
		gwt = abt_conv::gwen_timelist->takeFirst();
		GWEN_Time_free(gwt);
	}

	qDebug() << Q_FUNC_INFO << "freeing AB_VALUE list";
	AB_VALUE *v;
	while (!abt_conv::gwen_abvlist->isEmpty()) {
		v = abt_conv::gwen_abvlist->takeFirst();
		AB_Value_free(v);
	}

	//free the 'global' lists too
	delete abt_conv::gwen_strlist;
	delete abt_conv::gwen_timelist;
	delete abt_conv::gwen_abvlist;
}
