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
 *
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#ifndef PAGE_HISTORY_H
#define PAGE_HISTORY_H

#include <QFrame>
#include "../abt_jobinfo.h"

class abt_history;

namespace Ui {
	class page_history;
}

/** \brief Anzeige aller bereits durchgeführten und in der abt_history
 *         gespeicherten Transaktionen
 *
 */

class page_history : public QFrame
{
	Q_OBJECT
public:
	explicit page_history(const abt_history *history, QWidget *parent = 0);
	~page_history();
	
protected:
	void changeEvent(QEvent *e);
	void resizeEvent(QResizeEvent *event);

private:
	Ui::page_history *ui;
	const abt_history *history;

	QAction *actGenerateNewTransaction;
	QAction *actDeleteSelected;
	QAction *actExportSelected;

	void setTreeWidgetColWidths();
	void setDefaultTreeWidgetHeader();
	void createActions();

signals:
	void createNewFromHistory(const abt_jobInfo *jobInfo);
	void deleteFromHistory(const QList<int> historyIndexes);
	void exportFromHistory(const QList<int> historyIndexes);

private slots:
	void onActGenerateNewTransaction();
	void onActDeleteSelected();
	void onActExportSelected();

	void on_treeWidget_itemSelectionChanged();

public slots:
	void refreshTreeWidget(const abt_history *hist);

};

#endif // PAGE_HISTORY_H
