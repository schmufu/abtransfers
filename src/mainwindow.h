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
#include <QtGui/QDockWidget>
#include <QtGui/QToolBar>

#include "aqb_accounts.h"
#include "abt_job_ctrl.h"
#include "abt_history.h"

#include "pages/page_log.h"
#include "pages/page_ausgang.h"

//uncomment the following if access to pagewidgettests.cpp is wanted
//#define TESTWIDGETACCESS

class widgetTransfer;
class widgetRecurrence;

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
	void closeEvent(QCloseEvent *e);

private:
	Ui::MainWindow *ui;
	aqb_Accounts *accounts;
	abt_job_ctrl *jobctrl;
	abt_history *history;
	page_log *logw;
	Page_Ausgang *outw;

	QAction *actTransferNational;
	QAction *actTransferInternational;
	QAction *actTransferSepa;
	QAction *actTransferInternal;
	QAction *actDatedNew;
	QAction *actDatedUpdate;
	QAction *actStandingNew;
	QAction *actStandingUpdate;
	QAction *actDebitNote;
	QAction *actDebitNoteSepa;
	QAction *actUpdateBalance;
	QAction *actShowAvailableJobs;
//	QAction *act;
//	QAction *act;
//	QAction *act;

#ifdef TESTWIDGETACCESS
	QAction *actTestWidgetAccess;
#endif

	QMenu *accountContextMenu;
	QToolBar *dockToolbar;

	QDockWidget *dock_KnownRecipient;
	QDockWidget *dock_Accounts;
	QDockWidget *dock_KnownStandingOrders;
	QDockWidget *dock_KnownDatedTransfers;

	QTimer *timer;

	void createActions();
	void createMenus();
	void createDockToolbar();
	void createWidgetsInScrollArea();
	void createDockStandingOrders();
	void createDockDatedTransfers();

	widgetTransfer* createTransferWidgetAndAddTab(AB_JOB_TYPE type, const aqb_AccountInfo* account = NULL);
	void deleteTabWidgetAndTab(const widgetTransfer *w);
	void deleteTabWidgetAndTab(int tabIndex);

	void createAndSendTransfer(const widgetTransfer *sender);
	void createAndSendEUTransfer(const widgetTransfer *sender);
	void createAndSendDatedTransfer(const widgetTransfer *sender);
	void createAndSendStandingOrder(const widgetTransfer *sender);
	void createAndSendSepaTransfer(const widgetTransfer *sender);
	void createAndSendModifyDatedTransfer(const widgetTransfer *sender);
	void createAndSendModifyStandingOrder(const widgetTransfer *sender);
	void createAndSendDebitNote(const widgetTransfer *sender);
	void createAndSendInternalTransfer(const widgetTransfer *sender);
	void createAndSendSepaDebitNote(const widgetTransfer *sender);

	/** \brief fügt für alle Konten ein getBalance in den Ausgang ein */
	void appendGetBalanceToOutbox() const;
	/** \brief fügt für alle Konten ein getDatedTransders in den Ausgang ein */
	void appendGetDatedTransfersToOutbox() const;
	/** \brief fügt für alle Konten ein getStandingOrders in den Ausgang ein */
	void appendGetStandingOrdersToOutbox() const;

	/** \brief korrigiert automatisch die Datumseingaben beim Dauerauftrag */
	bool correctRecurrenceDates(widgetRecurrence *recurrence) const;

private slots:
	void on_actionAqBankingSetup_triggered();
	void on_tabWidget_UW_tabCloseRequested(int index);
	void TimerTimeOut();
	void DisplayNotAvailableTypeAtStatusBar(AB_JOB_TYPE type);
	void on_listWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void on_actionAbout_abTransfers_triggered();
	void on_actionAbout_Qt_triggered();
	void on_actionDebug_Info_triggered();
	void on_actionHelp_triggered();
	void on_actionEinstellungen_triggered();

	void onJobAddedToJobCtrlList(const abt_jobInfo* ji) const;
	void onAccountWidgetContextMenuRequest(QPoint p);

	//! wird aufgerufen wenn sich der Account im dockWidget von Daueraufträge ändert
	void selectedStandingOrdersAccountChanged(const aqb_AccountInfo* acc);
	//! wird aufgerufen wenn sich der Account im dockWidget von Terminüberweisungen ändert
	void selectedDatedTransfersAccountChanged(const aqb_AccountInfo* acc);

	//Slots für die verschiedenen Actions
	void onActionTransferNationalTriggered();
	void onActionTransferInternationalTriggered();
	void onActionTransferSepaTriggered();
	void onActionTransferInternalTriggered();
	void onActionDatedNewTriggered();
	void onActionDatedUpdateTriggered();
	void onActionStandingNewTriggered();
	void onActionStandingUpdateTriggered();
	void onActionDebitNoteTriggered();
	void onActionDebitNoteSepaTriggered();
	void onActionUpdateBalanceTriggered();
	void onActionShowAvailableJobsTriggered();

#ifdef TESTWIDGETACCESS
	void onActionTestWidgetAccessTriggered();
#endif

	void onWidgetTransferCreateTransfer(AB_JOB_TYPE type, const widgetTransfer* sender);
	void onWidgetTransferCancelClicked(widgetTransfer* sender);

	void onStandingOrderEditRequest(const aqb_AccountInfo* acc, const abt_standingOrderInfo* da);
	void onStandingOrderDeleteRequest(const aqb_AccountInfo* acc, const abt_standingOrderInfo* da);

	void onDatedTransferEditRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo*di);
	void onDatedTransferDeleteRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo*di);

	void onEditJobFromOutbox(const abt_jobInfo* job);

};

#endif // MAINWINDOW_H
