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

#ifndef ABT_CONV_H
#define ABT_CONV_H

#include <QString>
#include <QStringList>
#include <QDate>

#include <aqbanking/job.h>
#include <gwenhywfar/stringlist.h>


/*! \brief Konvertierungsfunktionen für GWEN* und AB* Typen zu Qt-Typen
  */
class abt_conv
{
protected:
	/*! speichert alle erstellten GWEN_STRINGLIST listen, damit sie am
	    Ende wieder gelöscht werden können. */
	static QList<GWEN_STRINGLIST*> *gwen_lists;
	/*! speichert alle erstellten GWEN_TIME Objecte, damit sie am
	    Ende wieder gelöscht werden können. */
	static QList<GWEN_TIME*> *gwen_timelist;
	/*! speichert alle erstellten AB_VALUE Objecte, damit sie am
	    Ende wieder gelöscht werden können. */
	static QList<AB_VALUE*> *gwen_abvlist;


public:
	abt_conv();

	static const QString JobTypeToQString(const AB_JOB *j);
	static const QString JobTypeToQString(AB_JOB_TYPE type);
	static const QString JobStatusToQString(const AB_JOB *j);
	static const QString JobStatusToQString(AB_JOB_STATUS status);


	static const QDate GwenTimeToQDate(const GWEN_TIME *gwen_time);
	static GWEN_TIME* QDateToGwenTime(const QDate &date);

	static const QStringList GwenStringListToQStringList(const GWEN_STRINGLIST *gwenList);
	static const GWEN_STRINGLIST* QStringListToGwenStringList(const QStringList &l);

	static const QString ABValueToString(const AB_VALUE *v, bool asDecimal=false);
	static AB_VALUE* ABValueFromString(const QString &str, const QString &currency = "EUR");

	//! Muss am Ende Aufgerufen werden um ALLE Listen wieder zu löschen!
	static void freeAllGwenLists();

};

#endif // ABT_CONV_H
