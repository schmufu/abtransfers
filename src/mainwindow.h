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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui/QListWidgetItem>

#include "aqb_accounts.h"
#include "abt_job_ctrl.h"

#include "pages/page_log.h"
#include "pages/page_ausgang.h"
#include "pages/page_da_edit_delete.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();



protected:
	void changeEvent(QEvent *e);

private:
	Ui::MainWindow *ui;
	aqb_Accounts *accounts;
	abt_job_ctrl *jobctrl;
	page_log *logw;
	Page_Ausgang *outw;
	Page_DA_Edit_Delete *daw;


private slots:
	void DisplayNotAvailableTypeAtStatusBar(AB_JOB_TYPE type);
	void on_actionExecQueued_triggered();
	void on_actionAddGetDated_triggered();
	void on_actionAddGetDAs_triggered();
	void on_actionAbout_abTransfers_triggered();
	void on_actionAbout_Qt_triggered();
	void on_listWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void on_actionDebug_Info_triggered();

	void onJobAddedToJobCtrlList(const abt_job_info* ji) const;
};

#endif // MAINWINDOW_H
