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
#include "pages/page_history.h"

//uncomment the following if access to pagewidgettests.cpp is wanted
//#define TESTWIDGETACCESS

class widgetTransfer;
class widgetRecurrence;

namespace Ui {
	class MainWindow;
}

/** @brief The main Window of AB-Transfers
 *
 * All activities are handled here and delegated to the other objects.
 *
 */

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
        aqb_Accounts *m_accounts;
        abt_job_ctrl *m_jobctrl;
        abt_history *m_history;
        page_log *m_logw;
        Page_Ausgang *m_outbox;
        page_history *m_pageHistory;

        QAction *m_actTransferNational;
        QAction *m_actTransferInternational;
        QAction *m_actTransferSepa;
        QAction *m_actTransferInternal;
        QAction *m_actDatedNew;
        QAction *m_actDatedUpdate;
        QAction *m_actStandingNew;
        QAction *m_actStandingUpdate;
        QAction *m_actDebitNote;
        QAction *m_actDebitNoteSepa;
        QAction *m_actUpdateBalance;
        QAction *m_actShowAvailableJobs;
        QAction *m_actSaveAllData;
//	QAction *m_act;
//	QAction *m_act;

#ifdef TESTWIDGETACCESS
        QAction *m_actTestWidgetAccess;
#endif

        QMenu *m_accountContextMenu;
        QToolBar *m_dockToolbar;

        QDockWidget *m_dock_KnownRecipient;
        QDockWidget *m_dock_Accounts;
        QDockWidget *m_dock_KnownStandingOrders;
        QDockWidget *m_dock_KnownDatedTransfers;

        QTimer *m_timer;

	void createActions();
	void createMenus();
	void createDockToolbar();
	void createWidgetsInScrollArea();
	void createDockBankAccountWidget();
	void createDockKnownRecipients();
	void createDockStandingOrders();
	void createDockDatedTransfers();

        widgetTransfer *createTransferWidgetAndAddTab(AB_JOB_TYPE type, const aqb_AccountInfo* account = NULL);
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

	/** @brief Puts a getBalance job for every account to the outbox */
	void appendGetBalanceToOutbox() const;
	/** @brief Puts a getDatedTransfers job for every account to the outbox */
	void appendGetDatedTransfersToOutbox() const;
	/** @brief Puts a getStandingOrders job for every account to the outbox */
	void appendGetStandingOrdersToOutbox() const;

	void checkReachedDatedTransfers();

	/** @brief Automatically corrects the dates for standingOrders */
	bool correctRecurrenceDates(widgetRecurrence *recurrence) const;

	bool isStandingOrderInOutbox(const abt_standingOrderInfo *soi);
	bool isStandingOrderOutdated(const aqb_AccountInfo *acc, const abt_standingOrderInfo *soi);

	bool isDatedTransferInOutbox(const abt_datedTransferInfo *dti);
	bool isDatedTransferOutdated(const aqb_AccountInfo *acc, const abt_datedTransferInfo *dti);

	void loadAccountData();
	void saveAccountData();
	void loadHistoryData();
	void saveHistoryData();

	void dockStandingOrdersSetAccounts();
	void dockDatedTransfersSetAccounts();

	void createJobCtrlAndConnections();

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

	/** @brief Is called when the account in the dockWiget was changed */
	void selectedStandingOrdersAccountChanged(const aqb_AccountInfo* acc);
	/** @brief Is called when the account in the dockWiget was changed */
	void selectedDatedTransfersAccountChanged(const aqb_AccountInfo* acc);

	//Slots for the different Actions
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
	void onActionSaveAllDataTriggered();

#ifdef TESTWIDGETACCESS
	void onActionTestWidgetAccessTriggered();
#endif

	void onWidgetTransferCreateTransfer(AB_JOB_TYPE type, const widgetTransfer* sender);
	void onWidgetTransferCancelClicked(widgetTransfer* sender);

	void onStandingOrderEditRequest(const aqb_AccountInfo* acc, const abt_standingOrderInfo* da);
	void onStandingOrderDeleteRequest(const aqb_AccountInfo* acc, const abt_standingOrderInfo* da);

	void onDatedTransferEditRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo*di);
	void onDatedTransferDeleteRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo*di);

	void onEditJobFromOutbox(int itemNr);
	void onJobCtrlQueueListChanged();

	void createTransferFromJob(const abt_jobInfo *ji);

	void deleteHistoryItems(QList<abt_jobInfo*> jiList);

};

#endif // MAINWINDOW_H
