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
 *	This class is used to manage the executed jobs.
 *	A widget can use the data of this class and display the jobs from
 *	the past executions.
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


/** \brief management of the realized jobs
  *
  * At start the MainWindow creates a history object and the needed data is
  * read by the history-parser from the history-file.
  * All data that the parser found is added to this object.
  *
  * If a new job is executed the result would at first be saved in this object
  * and at destruction of the MainWindow the data is exported to an
  * AB_IMEXPORTER_CONTEXT and saved in the history file.
  */

class abt_history: public QObject {
	Q_OBJECT

public:
	abt_history(/*const aqb_Accounts *allAccounts,*/ QObject *parent = 0);
	~abt_history();

private:
	/** @brief the list in which the job-items are stored */
	QList<abt_jobInfo*> *m_historyList;

	/** @brief Sorts the list of history items depending on their creation */
	void sortListByTimestamp(bool descending = true);

public:
	/** @brief adds the @a job to the history list */
	void add(abt_jobInfo *job);
	/** @brief deletes the @a job from the history list */
	bool remove(abt_jobInfo *job);
	/** @overload */
	bool remove(int pos);
	/** @brief removes all jobs from the history list */
	void clearAll();


	/** @brief creates an AB_IMEXPORTER_CONTEXT that could be saved
	 *  trough aqbanking
	 */
	AB_IMEXPORTER_CONTEXT *getContext() const;

	/** @brief returns the current history list as a const object */
	const QList<abt_jobInfo*> *getHistoryList() const { return this->m_historyList; }

private slots:

public slots:

signals:
	void historyListChanged(const abt_history *sender);

};

#endif // ABT_HISTORY_H
