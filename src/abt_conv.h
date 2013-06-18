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

#ifndef ABT_CONV_H
#define ABT_CONV_H

#include <QString>
#include <QStringList>
#include <QDate>

#include <aqbanking/job.h>
#include <gwenhywfar/stringlist.h>

#include "abjobtype.hpp"

/*! \brief conversion functions for GWEN* and AB* types to Qt types
  */
class abt_conv
{
protected:
	/*! stores all created GWEN_STRINGLIST lists, so that they can be
	 *  deleted at the end
	 */
        static QList<GWEN_STRINGLIST*> *m_gwen_strlist;

	/*! stores all created GWEN_TIME objects, so that they can be deleted
	 *  at the end
	 */
        static QList<GWEN_TIME*> *m_gwen_timelist;

	/*! stores all created AB_VALUE objects, so that they can be deleted
	 *  at the end
	 */
        static QList<AB_VALUE*> *m_gwen_abvlist;

public:
	abt_conv();

	static const QString JobTypeToQString(const AB_JOB *j);
        static const QString JobTypeToQString(AB_JOB_TYPE type);
        static const QString JobTypeToQString(AbJobType *abJobType);
	static const QString JobStatusToQString(const AB_JOB *j);
	static const QString JobStatusToQString(AB_JOB_STATUS status);


	static const QDate GwenTimeToQDate(const GWEN_TIME *gwen_time);
	static const GWEN_TIME* QDateToGwenTime(const QDate &date);

	static const QStringList GwenStringListToQStringList(const GWEN_STRINGLIST *gwenList);
	static const GWEN_STRINGLIST* QStringListToGwenStringList(const QStringList &l);

	static const QString ABValueToString(const AB_VALUE *value, bool asDecimal=false);
	static AB_VALUE* ABValueFromString(const QString &str, const QString &currency = "EUR");

	static void freeAllGwenLists();

};

#endif // ABT_CONV_H
