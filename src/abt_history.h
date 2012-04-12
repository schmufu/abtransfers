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
 *	Diese Klasse wird genutzt um die durchgeführten Aufträge zu verwalten.
 *	Ein Widget kann diese Daten dann nutzen um die durchgeführten Aufträge
 *	anzuzeigen.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#ifndef ABT_HISTORY_H
#define ABT_HISTORY_H

#include <QtCore/QObject>
#include <QList>

#include "aqb_accounts.h"
#include "abt_jobinfo.h"

class abt_history: public QObject {
	Q_OBJECT

//	//der parser darf unsere protected funktionen nutzen um der Liste
//	//neue Objecte hinzu zu fügen. Ansonsten darf niemand diese Werte ändern.
//	friend class abt_parser;

public:
	abt_history(const aqb_Accounts *allAccounts, QObject *parent = 0);
	~abt_history();

private:
	const aqb_Accounts *m_allAccounts;
	QList<abt_jobInfo*> *m_historyList;

public:
	void add(abt_jobInfo *job);
	bool remove(abt_jobInfo *job);
	bool remove(int pos);

	AB_IMEXPORTER_CONTEXT *getContext() const;



private slots:


public slots:



signals:
	void historyListChanged(const abt_history *sender);

};

#endif // ABT_HISTORY_H
