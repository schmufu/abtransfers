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

#ifndef PAGE_AUSGANG_H
#define PAGE_AUSGANG_H

#include <QFrame>
#include <QAction>
#include "../abt_job_ctrl.h" //for abt_jobinfo and AB_xxx types

class abt_settings;

namespace Ui {
	class Page_Ausgang;
}

/** \brief Anzeige aller im abt_job_ctrl vorhandenen Aufträge für die Bank
 *
 */

class Page_Ausgang : public QFrame {
	Q_OBJECT
private:
	int selectedItem; //!< temporaly var for the currently selected item

	QAction *actEdit;
	QAction *actDelete;
	QAction *actUp;
	QAction *actDown;

	abt_settings *settings;

	void setDefaultTreeWidgetHeader();
	void setTreeWidgetColWidths();
	void createAllActions();

	//! Gibt true zurück wenn der übergebene \a type bearbeitbar ist
	static bool isJobTypeEditable(const AB_JOB_TYPE type);

public:
	Page_Ausgang(abt_settings *settings, QWidget *parent = 0);
	~Page_Ausgang();

protected:
	void changeEvent(QEvent *e);
	void resizeEvent(QResizeEvent *event);

private:
	Ui::Page_Ausgang *ui;

signals:
	void executeClicked();
	void editJob(int itemNr);
	void moveJobInList(int itemNr, int updown);
	void removeJob(abt_jobInfo *jobinfo);

public slots:
	void refreshTreeWidget(const abt_job_ctrl *jobctrl);


private slots:
	void onActionDeleteTriggered();
	void onActionEditTriggered();
	void onActionUpTriggered();
	void onActionDownTriggered();

	void on_pushButton_exec_clicked();
	void on_treeWidget_customContextMenuRequested(QPoint pos);
	void on_treeWidget_itemSelectionChanged();
};

#endif // PAGE_AUSGANG_H
