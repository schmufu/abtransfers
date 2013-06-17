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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QString>
#include <QStringList>
#include <QDebug>

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QAction>
#include <QIcon>
#include <QTimer>

#if defined(USE_QT_WEBKIT)
	#include <QWebView>
#endif

#include <aqbanking/banking.h>
#include <aqbanking/account.h>
#include <aqbanking/jobgettransactions.h>
#include <aqbanking/value.h>
#include <aqbanking/dlg_setup.h>
#include <aqbanking/dlg_importer.h>

#include "globalvars.h"
#include "abt_conv.h"
#include "abt_transactionlimits.h"
#include "widgets/widgettransfer.h"
#include "widgets/bankaccountswidget.h"
#include "widgets/knownempfaengerwidget.h"
#include "widgets/widgetknownstandingorders.h"
#include "widgets/widgetknowndatedtransfers.h"
#include "widgets/widgetaccountcombobox.h"

#include "dialogs/dialogsettings.h"
#include "dialogs/abt_dialog.h"
#include "translationchooser.h"

#include "abt_parser.h"

#ifdef TESTWIDGETACCESS
	 //only for test purposes
#include "pages/pagewidgettests.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	//at first set the wanted language, so that all later created objects
	//consider the translations.
	QString lang = settings->language();
	if (lang.isEmpty()) {
		//try to use the current system locale
		this->translations = new TranslationChooser(QLocale::system(), this);
	} else {
		this->translations = new TranslationChooser(lang, this);
	}
	//keep the settings updated by each language change
	connect(this->translations, SIGNAL(languageChanged(QString)),
		settings, SLOT(setLanguage(QString)));

	QAction *langMenu = this->ui->menuEinstellungen->addMenu(
					this->translations->languageMenu());
	langMenu->setText(tr("Sprache"));


	this->accounts = new aqb_Accounts(banking->getAqBanking());
	this->history = new abt_history(this);
	this->logw = new page_log();
	this->outbox = new Page_Ausgang(settings);
	this->dock_KnownRecipient = NULL;
	this->dock_KnownStandingOrders = NULL;
	this->dock_KnownDatedTransfers = NULL;
	this->dock_Accounts = NULL;

	//All accounts from AqBanking were created (this->accounts).
	this->loadAccountData(); //Now the account data can be loaded
	this->loadHistoryData(); //and also the history data

	this->pageHistory = new page_history(this->history);

	QVBoxLayout *logLayout = new QVBoxLayout(ui->Log);
	logLayout->setMargin(0);
	logLayout->setSpacing(2);
	ui->Log->setLayout(logLayout);
	ui->Log->layout()->addWidget(this->logw);

	QVBoxLayout *outLayout = new QVBoxLayout(ui->Ausgang);
	outLayout->setMargin(0);
	outLayout->setSpacing(2);
	ui->Ausgang->setLayout(outLayout);
	ui->Ausgang->layout()->addWidget(this->outbox);

	QVBoxLayout *historyLayout = new QVBoxLayout(ui->history);
	historyLayout->setMargin(0);
	historyLayout->setSpacing(2);
	ui->history->setLayout(historyLayout);
	ui->history->layout()->addWidget(this->pageHistory);

	//set default DockOptions
	this->setDockOptions(QMainWindow::AllowNestedDocks |
			     QMainWindow::AllowTabbedDocks);// |
			     //QMainWindow::AnimatedDocks);
	this->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	//create all DockWidgets
	this->createDockKnownRecipients();
	this->createDockBankAccountWidget();
	this->createDockStandingOrders();
	this->createDockDatedTransfers();

	this->createJobCtrlAndConnections(); //must be called after createDock...

	this->createActions();
	this->createMenus();
	this->createDockToolbar();

	this->createWidgetsInScrollArea();

	//connect the signals and slots for the PushButtons of the summary widget
	connect(this->ui->pushButton_transferNational, SIGNAL(clicked()),
		this->actTransferNational, SLOT(trigger()));
	connect(this->ui->pushButton_transferInternational, SIGNAL(clicked()),
		this->actTransferInternational, SLOT(trigger()));
	connect(this->ui->pushButton_transferInternal, SIGNAL(clicked()),
		this->actTransferInternal, SLOT(trigger()));
	connect(this->ui->pushButton_transferSepa, SIGNAL(clicked()),
		this->actTransferSepa, SLOT(trigger()));
	connect(this->ui->pushButton_standingNew, SIGNAL(clicked()),
		this->actStandingNew, SLOT(trigger()));
	connect(this->ui->pushButton_standingUpdate, SIGNAL(clicked()),
		this->actStandingUpdate, SLOT(trigger()));
	connect(this->ui->pushButton_datedNew, SIGNAL(clicked()),
		this->actDatedNew, SLOT(trigger()));
	connect(this->ui->pushButton_datedUpdate, SIGNAL(clicked()),
		this->actDatedUpdate, SLOT(trigger()));

	connect(this->pageHistory, SIGNAL(createNewFromHistory(const abt_jobInfo*)),
		this, SLOT(createTransferFromJob(const abt_jobInfo*)));
	connect(this->pageHistory, SIGNAL(deleteFromHistory(QList<abt_jobInfo*>)),
		this, SLOT(deleteHistoryItems(QList<abt_jobInfo*>)));
	connect(this->pageHistory, SIGNAL(showSettingsForImExpFavorite()),
		this, SLOT(on_actionEinstellungen_triggered()));


	//Always show the summary as start page, regardless of the .ui setting
	//and also set the summary selected in listWidget
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
	this->ui->stackedWidget->setCurrentIndex(0);


	//we start a timer, this timer starts working, but the connected slot
	//is not called until the eventLoop of the application is running.
	//This is the case when app.exec() in main() is executed.
	//So the code in the slot TimerTimeOut() is executed when the
	//application is completely initialised and running.
	this->timer = new QTimer(this);
	this->timer->setSingleShot(true);
	connect(this->timer, SIGNAL(timeout()), this, SLOT(TimerTimeOut()));
	this->timer->start(10);

#ifdef TESTWIDGETACCESS
	this->ui->menuBar->addAction(this->actTestWidgetAccess);
#endif

}

MainWindow::~MainWindow()
{
	disconnect(this->jobctrl, SIGNAL(jobNotAvailable(AB_JOB_TYPE)),
		   this, SLOT(DisplayNotAvailableTypeAtStatusBar(AB_JOB_TYPE)));

	//cleanup all created widgets
	delete this->outbox;
	delete this->logw;
	delete this->pageHistory;
	delete this->jobctrl;
	delete this->history;
	delete this->accounts;
	delete ui;

	qDebug() << Q_FUNC_INFO << "deleted";
}

void MainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		this->retranslateCppCode();
		break;
	default:
		break;
	}
}

//protected
void MainWindow::closeEvent(QCloseEvent *e)
{
	//check if jobs are in the outbox. If there are some ask the user
	//if the application should realy quit.
	if (this->jobctrl->jobqueueList()->size() != 0) {
		int ret;
		ret = QMessageBox::warning(this,
					   tr("Aufträge im Ausgang"),
					   tr("Es befinden sich noch Aufträge im Ausgang "
					      "die noch nicht gesendet wurden!<br />"
					      "Beim Beenden des Programms werden die "
					      "Aufträge im Ausgang gelöscht!<br /><br />"
					      "Soll das Programm wirklich beendet werden?"),
					   QMessageBox::Yes | QMessageBox::No,
					   QMessageBox::No);
		if (ret == QMessageBox::No) {
			e->ignore(); //do not quit the application
			return;
		}
	}

	if (debugDialog->isVisible()) {
		//the application can not quit when the debugDialog
		//is still visible
		debugDialog->hide();
	}

	this->actSaveAllData->trigger(); //save all data before quit
	e->accept(); //now we can get closed
}

//protected
/** \brief retranslates all strings that are created during runtime.
 *
 * This function is also called when a QEvent::LanguageChange event occurs.
 *
 * Every string that is created at code level and not in .ui file and that is
 * visible at the time of a language change should be added here.
 *
 * Strings that are newly created each time must not be added.
 */
void MainWindow::retranslateCppCode()
{
	this->dock_Accounts->setWindowTitle(tr("Online Konten"));
	this->dock_KnownDatedTransfers->setWindowTitle(tr("Terminüberweisungen"));
	this->dock_KnownRecipient->setWindowTitle(tr("Bekannte Empfänger"));
	this->dock_KnownStandingOrders->setWindowTitle(tr("Daueraufträge"));
}

//private Slot
/** @brief Execution after the Eventloop is running
 *
 * This slot is called wenn the event loop starts and the application is
 * running. Therefore this code is executed when the MainWindow is displayed
 * and then never again.
 */
void MainWindow::TimerTimeOut()
{
	disconnect(this, SLOT(TimerTimeOut())); //remove connection
	delete this->timer; //the timer is no longer needed
	this->timer = NULL;

	abt_dialog dia(this,
		       tr("Eventuelle Kosten"),
		       tr("<h4>Aufträge können gebührenpflichtig sein</h4>"
			  ""
			  "Bei einigen Kreditinstituten/Banken können Gebühren für "
			  "bestimmte Aufträge (Einrichtung von Daueraufträgen, "
			  "Sammelüberweisungen, etc.) anfallen.<br />"
			  "Bitte informieren Sie sich vorab bei Ihrem Institut / "
			  "Ihrer Bank welche Kosten für welche Aufträge anfallen!<br />"
			  "<br />"
			  "<b>Ich übernehme keine Haftung für eventuell entstehende "
			  "Kosten!</b>"),
		       QDialogButtonBox::Ok,
		       QDialogButtonBox::Ok,
		       QMessageBox::Information,
		       "WarnCosts");
	dia.exec();

	//check if jobs should be put in the outbox and if the should be
	//executed at start.
	if (settings->appendJobToOutbox("getBalance")) {
		this->appendGetBalanceToOutbox();
	}

	if (settings->appendJobToOutbox("getDatedTransfers")) {
		this->appendGetDatedTransfersToOutbox();
	}

	if (settings->appendJobToOutbox("getStandingOrders")) {
		this->appendGetStandingOrdersToOutbox();
	}

	if (settings->appendJobToOutbox("executeAtStart")) {
		this->jobctrl->execQueuedTransactions();
	}

	//check if DatedTransfers exists which reached the date of execution.
	this->checkReachedDatedTransfers();
}

//private
void MainWindow::createActions()
{
	actTransferNational = new QAction(this);
	actTransferNational->setText(tr("National"));
	actTransferNational->setIcon(QIcon(":/icons/bank-icon"));
	connect(actTransferNational, SIGNAL(triggered()), this, SLOT(onActionTransferNationalTriggered()));

	actTransferInternational = new QAction(this);
	actTransferInternational->setText(tr("International"));
	actTransferInternational->setIcon(QIcon(":/icons/bank-icon"));
	connect(actTransferInternational, SIGNAL(triggered()), this, SLOT(onActionTransferInternationalTriggered()));

	actTransferSepa = new QAction(this);
	actTransferSepa->setText(tr("SEPA (EU weit)"));
	actTransferSepa->setIcon(QIcon(":/icons/bank-icon"));
	connect(actTransferSepa, SIGNAL(triggered()), this, SLOT(onActionTransferSepaTriggered()));

	actTransferInternal = new QAction(this);
	actTransferInternal->setText(tr("Umbuchung"));
	actTransferInternal->setIcon(QIcon(":/icons/bank-icon"));
	connect(actTransferInternal, SIGNAL(triggered()), this, SLOT(onActionTransferInternalTriggered()));

	actDatedNew = new QAction(this);
	actDatedNew->setText(tr("Anlegen"));
	actDatedNew->setIcon(QIcon(":/icons/bank-icon"));
	connect(actDatedNew, SIGNAL(triggered()), this, SLOT(onActionDatedNewTriggered()));

	actDatedUpdate = new QAction(this);
	actDatedUpdate->setText(tr("Aktualisieren"));
	actDatedUpdate->setIcon(QIcon(":/icons/bank-icon"));
	connect(actDatedUpdate, SIGNAL(triggered()), this, SLOT(onActionDatedUpdateTriggered()));

	actStandingNew = new QAction(this);
	actStandingNew->setText(tr("Anlegen"));
	actStandingNew->setIcon(QIcon(":/icons/bank-icon"));
	connect(actStandingNew, SIGNAL(triggered()), this, SLOT(onActionStandingNewTriggered()));

	actStandingUpdate = new QAction(this);
	actStandingUpdate->setText(tr("Aktualisieren"));
	actStandingUpdate->setIcon(QIcon(":/icons/bank-icon"));
	connect(actStandingUpdate, SIGNAL(triggered()), this, SLOT(onActionStandingUpdateTriggered()));

	actDebitNote = new QAction(this);
	actDebitNote->setText(tr("Lastschrift"));
	actDebitNote->setIcon(QIcon(":/icons/bank-icon"));
	connect(actDebitNote, SIGNAL(triggered()), this, SLOT(onActionDebitNoteTriggered()));

	actDebitNoteSepa = new QAction(this);
	actDebitNoteSepa->setText(tr("SEPA-Lastschrift (EU weit)"));
	actDebitNoteSepa->setIcon(QIcon(":/icons/bank-icon"));
	connect(actDebitNoteSepa, SIGNAL(triggered()), this, SLOT(onActionDebitNoteSepaTriggered()));

	actUpdateBalance = new QAction(this);
	actUpdateBalance->setText(tr("Kontostand aktualisieren"));
	actUpdateBalance->setIcon(QIcon(":/icons/bank-icon"));
	connect(actUpdateBalance, SIGNAL(triggered()), this, SLOT(onActionUpdateBalanceTriggered()));

	actShowAvailableJobs = new QAction(this);
	actShowAvailableJobs->setText(tr("Unterstütze Aufträge"));
	actShowAvailableJobs->setIcon(QIcon(":/icons/bank-icon"));
	connect(actShowAvailableJobs, SIGNAL(triggered()), this, SLOT(onActionShowAvailableJobsTriggered()));

	actSaveAllData = new QAction(this);
	actSaveAllData->setText(tr("Speichern"));
	actSaveAllData->setIcon(QIcon::fromTheme("document-save", QIcon(":/icons/document-save")));
	connect(actSaveAllData, SIGNAL(triggered()), this, SLOT(onActionSaveAllDataTriggered()));


#ifdef TESTWIDGETACCESS
	actTestWidgetAccess = new QAction(tr("TestWidget"), this);
	connect(actTestWidgetAccess, SIGNAL(triggered()), this, SLOT(onActionTestWidgetAccessTriggered()));
#endif


}


//private
void MainWindow::createMenus()
{
	this->accountContextMenu = new QMenu(this);
	QMenu *MenuTransfer = new QMenu(tr("Überweisung"), this);
	MenuTransfer->addAction(this->actTransferNational);
	MenuTransfer->addAction(this->actTransferInternational);
	MenuTransfer->addAction(this->actTransferInternal);
	MenuTransfer->addAction(this->actTransferSepa);
	QMenu *MenuStanding = new QMenu(tr("Daueraufträge"), this);
	MenuStanding->addAction(this->actStandingNew);
	MenuStanding->addAction(this->actStandingUpdate);
	QMenu *MenuDated = new QMenu(tr("Terminüberweisungen"), this);
	MenuDated->addAction(this->actDatedNew);
	MenuDated->addAction(this->actDatedUpdate);
	this->accountContextMenu->addMenu(MenuTransfer);
	this->accountContextMenu->addMenu(MenuStanding);
	this->accountContextMenu->addMenu(MenuDated);
	this->accountContextMenu->addAction(this->actDebitNote);
	this->accountContextMenu->addAction(this->actDebitNoteSepa);
	this->accountContextMenu->addSeparator();
	this->accountContextMenu->addAction(this->actUpdateBalance);
	this->accountContextMenu->addAction(this->actShowAvailableJobs);
}

//private
void MainWindow::createDockToolbar()
{
	this->dockToolbar = new QToolBar(tr("Dock Toolbar"),this);
	this->dockToolbar->setObjectName("dockToolbar");
	this->dockToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	this->dockToolbar->addAction(this->dock_Accounts->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownRecipient->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownStandingOrders->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownDatedTransfers->toggleViewAction());

	this->addToolBar(Qt::TopToolBarArea, this->dockToolbar);
}

//private
void MainWindow::createWidgetsInScrollArea()
{
	//delete everthing in the ScrollArea and recreate all.

	//perhaps we already have a layout
	QVBoxLayout *layoutScrollArea = dynamic_cast<QVBoxLayout*>(this->ui->scrollAreaWidgetContents->layout());
	if (layoutScrollArea) {
		//layout exists, so the QGroupBoxes. Remove all childs.
		QList<QGroupBox*> list = this->ui->scrollAreaWidgetContents->findChildren<QGroupBox*>();
		while(!list.isEmpty()) {
			delete list.takeFirst();
		}
	} else { //create new layout
		layoutScrollArea = new QVBoxLayout(this->ui->scrollAreaWidgetContents);
	}

	foreach(const aqb_AccountInfo *acc, this->accounts->getAccountHash().values()) {
		//known Standing Orders
		QGroupBox *grpSO = new QGroupBox(this);
		QVBoxLayout *lSO = new QVBoxLayout(grpSO);
		grpSO->setTitle(tr("Daueraufträge von \"%1\" (%2 - %3)").arg(acc->Name(), acc->Number(), acc->BankCode()));
		widgetKnownStandingOrders *standingOrders = new widgetKnownStandingOrders(this->ui->scrollAreaWidgetContents);
		standingOrders->setAccount(acc);;

		connect(standingOrders, SIGNAL(updateStandingOrders(const aqb_AccountInfo*)),
			this->jobctrl, SLOT(addGetStandingOrders(const aqb_AccountInfo*)));

		connect(standingOrders, SIGNAL(editStandingOrder(const aqb_AccountInfo*,const abt_standingOrderInfo*)),
			this, SLOT(onStandingOrderEditRequest(const aqb_AccountInfo*,const abt_standingOrderInfo*)));
		connect(standingOrders, SIGNAL(deleteStandingOrder(const aqb_AccountInfo*,const abt_standingOrderInfo*)),
			this, SLOT(onStandingOrderDeleteRequest(const aqb_AccountInfo*,const abt_standingOrderInfo*)));

		lSO->addWidget(standingOrders);
		layoutScrollArea->addWidget(grpSO);

		//known Dated Transfers
		QGroupBox *grpDT = new QGroupBox(this);
		QVBoxLayout *lDT = new QVBoxLayout(grpDT);
		grpDT->setTitle(tr("Terminierte Überweisungen von \"%1\" (%2 - %3)").arg(acc->Name(), acc->Number(), acc->BankCode()));
		widgetKnownDatedTransfers *datedTransfers = new widgetKnownDatedTransfers(this->ui->scrollAreaWidgetContents);
		datedTransfers->setAccount(acc);

		connect(datedTransfers, SIGNAL(updateDatedTransfers(const aqb_AccountInfo*)),
			this->jobctrl, SLOT(addGetDatedTransfers(const aqb_AccountInfo*)));

		connect(datedTransfers, SIGNAL(editDatedTransfer(const aqb_AccountInfo*, const abt_datedTransferInfo*)),
			this, SLOT(onDatedTransferEditRequest(const aqb_AccountInfo*, const abt_datedTransferInfo*)));
		connect(datedTransfers, SIGNAL(deleteDatedTransfer(const aqb_AccountInfo*, const abt_datedTransferInfo*)),
			this, SLOT(onDatedTransferDeleteRequest(const aqb_AccountInfo*, const abt_datedTransferInfo*)));

		lDT->addWidget(datedTransfers);
		layoutScrollArea->addWidget(grpDT);
	}
}

//private
/** \brief Creates the "Online Konten" QDockWidget */
void MainWindow::createDockBankAccountWidget()
{
	/** create a new QDockWidget ("Online Konten"). */
	this->dock_Accounts = new QDockWidget(tr("Online Konten"),this);
	this->dock_Accounts->setObjectName("OnlineAccounts");
	qDebug() << "creating bankAccountsWidget";
	/** A new BankAccountsWidget is set as the new widget in der QDockWidget. */
	BankAccountsWidget *baw = new BankAccountsWidget(this->accounts, this->dock_Accounts);
	this->dock_Accounts->setWidget(baw);
	//this->dock_Accounts->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
	this->dock_Accounts->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_Accounts->setFloating(false);
	this->dock_Accounts->hide();
	this->dock_Accounts->toggleViewAction()->setIcon(QIcon(":/icons/bank-icon"));
	/** Add the DockWidget topDockWidgetArea of the MainWindow. */
	this->addDockWidget(Qt::TopDockWidgetArea, this->dock_Accounts);
	/** Connection for the ContextMenu is established. */
	connect(baw, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(onAccountWidgetContextMenuRequest(QPoint)));
}

//private
/** \brief Creates the "Bekannte Empfänger" QDockWidget
 *
 * A new QDockWidget ("Bekannte Empfänger") is created and a new
 * KnownEmpfaengerWidget is set as the new Widget of the QDockWidget.
 *
 * Also all needed connections to the KnownEmpfaengerWidget are established.
 */
void MainWindow::createDockKnownRecipients()
{
	this->dock_KnownRecipient = new QDockWidget(tr("Bekannte Empfänger"),this);
	this->dock_KnownRecipient->setObjectName("KnownRecipients");
	qDebug() << "creating knownEmpfaengerWidget";
	KnownEmpfaengerWidget *kew = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient);
	//Changes of the known recipients must be send to the widget
	connect(settings, SIGNAL(recipientsListChanged()),
		kew, SLOT(onEmpfaengerListChanged()));
	connect(kew, SIGNAL(replaceKnownEmpfaenger(int,abt_EmpfaengerInfo*)),
		settings, SLOT(onReplaceKnownRecipient(int,abt_EmpfaengerInfo*)));
	connect(kew, SIGNAL(addNewKnownEmpfaenger(abt_EmpfaengerInfo*)),
		settings, SLOT(addKnownRecipient(abt_EmpfaengerInfo*)));
	connect(kew, SIGNAL(deleteKnownEmpfaenger(abt_EmpfaengerInfo*)),
		settings, SLOT(deleteKnownRecipient(abt_EmpfaengerInfo*)));
	this->dock_KnownRecipient->setWidget(kew);
	//this->dock_KnownRecipient->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	this->dock_KnownRecipient->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient->setFloating(false);
	this->dock_KnownRecipient->hide();
	this->dock_KnownRecipient->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient);
}

//private
void MainWindow::createDockStandingOrders()
{
	QDockWidget *dock = new QDockWidget(tr("Daueraufträge"), this);
	dock->setObjectName("dockStandingOrders");

	QVBoxLayout *layoutDock = new QVBoxLayout();
	QHBoxLayout *layoutAcc = new QHBoxLayout();
	QLabel *accText = new QLabel(tr("Konto"));

	widgetAccountComboBox *accComboBox = new widgetAccountComboBox(NULL, NULL);

	widgetKnownStandingOrders *StandingOrders;
	//The widgetKnownStandingOrders must be created with the selected
	//account in the widgetAccountComboBox. (If the widgetAccountComboBox
	//is created with NULL the first account is selected).
	//This is done after all creations and connections by
	//dockStandingOrdersSetAccounts();
	StandingOrders = new widgetKnownStandingOrders();

	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		StandingOrders, SLOT(setAccount(const aqb_AccountInfo*)));
	//Changes at the selection must also be saved in the settings.ini
	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		this, SLOT(selectedStandingOrdersAccountChanged(const aqb_AccountInfo*)));

	connect(StandingOrders, SIGNAL(editStandingOrder(const aqb_AccountInfo*,const abt_standingOrderInfo*)),
		this, SLOT(onStandingOrderEditRequest(const aqb_AccountInfo*,const abt_standingOrderInfo*)));
	connect(StandingOrders, SIGNAL(deleteStandingOrder(const aqb_AccountInfo*,const abt_standingOrderInfo*)),
		this, SLOT(onStandingOrderDeleteRequest(const aqb_AccountInfo*,const abt_standingOrderInfo*)));

	layoutAcc->addWidget(accText,1, Qt::AlignRight);
	layoutAcc->addWidget(accComboBox, 5, Qt::AlignLeft);

	layoutDock->addLayout(layoutAcc);
	layoutDock->addWidget(StandingOrders);
	QWidget *wid = new QWidget();
	wid->setLayout(layoutDock);

	dock->setWidget(wid);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dock->setFloating(false);
	dock->hide();
	dock->toggleViewAction()->setIcon(QIcon(":/icons/dauerauftrag"));
	this->addDockWidget(Qt::RightDockWidgetArea, dock);

	this->dock_KnownStandingOrders = dock;

	this->dockStandingOrdersSetAccounts();
}

//private
void MainWindow::dockStandingOrdersSetAccounts()
{
	widgetAccountComboBox *accComboBox = this->dock_KnownStandingOrders->findChild<widgetAccountComboBox*>();
	widgetKnownStandingOrders *standingOrders = this->dock_KnownStandingOrders->findChild<widgetKnownStandingOrders*>();

	if ((accComboBox == NULL) || (standingOrders == NULL)) {
		return; //one widget missing, cancel
	}

	//get the last selected account (from settings)
	int selAccID = settings->loadSelAccountInWidget("StandingOrders");
	const aqb_AccountInfo *lastAcc = this->accounts->getAccount(selAccID);

	//set all known accounts in the ComboBox
	accComboBox->setAllAccounts(this->accounts);
	//and select the last time selected one
	accComboBox->setSelectedAccount(lastAcc);

	//by setting the selected account in the accComboBox, the account for
	//the standingOrders Widget is changed automatically.
}

//private
void MainWindow::createDockDatedTransfers()
{
	QDockWidget *dock = new QDockWidget(tr("Terminüberweisungen"), this);
	dock->setObjectName("dockDatedTransfers");

	QVBoxLayout *layoutDock = new QVBoxLayout();
	QHBoxLayout *layoutAcc = new QHBoxLayout();
	QLabel *accText = new QLabel(tr("Konto"));

	widgetAccountComboBox *accComboBox = new widgetAccountComboBox(NULL, NULL);

	widgetKnownDatedTransfers *DatedTransfers;
	//The widgetKnownDatedTransfers must be created with the selected
	//account in the widgetAccountComboBox. (If the widgetAccountComboBox
	//is created with NULL the first account is selected).
	//This is done after all creations and connections by
	//dockDatedTransfersSetAccounts();
	DatedTransfers = new widgetKnownDatedTransfers();

	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		DatedTransfers, SLOT(setAccount(const aqb_AccountInfo*)));

	//Changes at the selection must also be saved in the settings.ini
	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		this, SLOT(selectedDatedTransfersAccountChanged(const aqb_AccountInfo*)));

	connect(DatedTransfers, SIGNAL(editDatedTransfer(const aqb_AccountInfo*,const abt_datedTransferInfo*)),
		this, SLOT(onDatedTransferEditRequest(const aqb_AccountInfo*,const abt_datedTransferInfo*)));
	connect(DatedTransfers, SIGNAL(deleteDatedTransfer(const aqb_AccountInfo*,const abt_datedTransferInfo*)),
		this, SLOT(onDatedTransferDeleteRequest(const aqb_AccountInfo*,const abt_datedTransferInfo*)));

	layoutAcc->addWidget(accText,1, Qt::AlignRight);
	layoutAcc->addWidget(accComboBox, 5, Qt::AlignLeft);

	layoutDock->addLayout(layoutAcc);
	layoutDock->addWidget(DatedTransfers);
	QWidget *wid = new QWidget();
	wid->setLayout(layoutDock);

	dock->setWidget(wid);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dock->setFloating(false);
	dock->hide();
	dock->toggleViewAction()->setIcon(QIcon(":/icons/dauerauftrag"));
	this->addDockWidget(Qt::RightDockWidgetArea, dock);

	this->dock_KnownDatedTransfers = dock;

	this->dockDatedTransfersSetAccounts();
}

//private
void MainWindow::dockDatedTransfersSetAccounts()
{
	widgetAccountComboBox *accComboBox = this->dock_KnownDatedTransfers->findChild<widgetAccountComboBox*>();
	widgetKnownDatedTransfers *datedTransfers = this->dock_KnownDatedTransfers->findChild<widgetKnownDatedTransfers*>();

	if ((accComboBox == NULL) || (datedTransfers == NULL)) {
		return; //one widget missing, cancel
	}

	//get the last selected account (from settings)
	int selAccID = settings->loadSelAccountInWidget("DatedTransfers");
	const aqb_AccountInfo *lastAcc = this->accounts->getAccount(selAccID);

	//set all known accounts in the ComboBox
	accComboBox->setAllAccounts(this->accounts);
	//and select the last time selected one
	accComboBox->setSelectedAccount(lastAcc);

	//by setting the selected account in the accComboBox, the account for
	//the datedTransfers Widget is changed automatically.
}


//private
void MainWindow::createJobCtrlAndConnections()
{
	Q_ASSERT(this->history); //the history and
	Q_ASSERT(this->accounts); //accounts must exist

	this->jobctrl = new abt_job_ctrl(this->accounts, this->history, this);

	/***** Signals and Slots ******/
	//Display not possible jobs at the status bar
	connect(this->jobctrl, SIGNAL(jobNotAvailable(AB_JOB_TYPE)),
		this, SLOT(DisplayNotAvailableTypeAtStatusBar(AB_JOB_TYPE)));

	//Connection for successfully added jobs
	connect(this->jobctrl, SIGNAL(jobAdded(const abt_jobInfo*)),
		this, SLOT(onJobAddedToJobCtrlList(const abt_jobInfo*)));

	//log messges ob abt_job_ctrl should go to the log wiget
	connect(this->jobctrl, SIGNAL(log(QString)),
		this->logw, SLOT(appendLogText(QString)));

	//Allow ediging of jobs in the outbox
	connect(this->outbox, SIGNAL(editJob(int)),
		this, SLOT(onEditJobFromOutbox(int)));

	connect(this->jobctrl, SIGNAL(jobQueueListChanged()),
		this, SLOT(onJobCtrlQueueListChanged()));
	connect(this->outbox, SIGNAL(moveJobInList(int,int)),
		this->jobctrl, SLOT(moveJob(int,int)));
	connect(this->outbox, SIGNAL(executeClicked()),
		this->jobctrl, SLOT(execQueuedTransactions()));
	connect(this->outbox, SIGNAL(removeJob(abt_jobInfo *)),
		this->jobctrl, SLOT(deleteJob(abt_jobInfo *)));


	widgetKnownDatedTransfers *datedTransfers = this->dock_KnownDatedTransfers->findChild<widgetKnownDatedTransfers*>();
	connect(datedTransfers, SIGNAL(updateDatedTransfers(const aqb_AccountInfo*)),
		this->jobctrl, SLOT(addGetDatedTransfers(const aqb_AccountInfo*)));

	widgetKnownStandingOrders *standingOrders = this->dock_KnownStandingOrders->findChild<widgetKnownStandingOrders*>();
	connect(standingOrders, SIGNAL(updateStandingOrders(const aqb_AccountInfo*)),
		this->jobctrl, SLOT(addGetStandingOrders(const aqb_AccountInfo*)));

}

//private
/** @brief Loads all data for all accounts */
void MainWindow::loadAccountData()
{
	Q_ASSERT(this->accounts); //accounts must exist
	AB_IMEXPORTER_CONTEXT *ctx;

	//get all account data from the relevant file
	ctx = abt_parser::load_local_ctx(settings->getAccountDataFilename(),
					 "ctxfile", "default");
	abt_parser::parse_ctx(ctx, this->accounts);
	AB_ImExporterContext_free(ctx); //ctx no longer needed, all loaded
}

//private
/** @brief Saves all data from all accounts */
void MainWindow::saveAccountData()
{
	Q_ASSERT(this->accounts); //accounts must exist
	AB_IMEXPORTER_CONTEXT *ctx = NULL;

	//create one AB_IMEXPORTER_CONTEXT for all accounts
	ctx = abt_parser::create_ctx_from(this->accounts);

	if (!ctx) return; //if no ctx created, we have nothing to save

	abt_parser::save_local_ctx(ctx, settings->getAccountDataFilename(),
				   "ctxfile", "default");
	AB_ImExporterContext_free(ctx);
}

//private
/** \brief loads all history data */
void MainWindow::loadHistoryData()
{
	Q_ASSERT(this->history); //the history and
	Q_ASSERT(this->accounts); //accounts must exist
	AB_IMEXPORTER_CONTEXT *ctx;

	//clear all loaded history items, we relaod them all
	this->history->clearAll();

	//load all history items from the relevant file
	ctx = abt_parser::load_local_ctx(settings->getHistoryFilename(),
					 "ctxfile", "default");
	abt_parser::parse_ctx(ctx, this->accounts, this->history);
	AB_ImExporterContext_free(ctx);
}

//private
/** \brief saves all history data */
void MainWindow::saveHistoryData()
{
	Q_ASSERT(this->history); //the history and
	Q_ASSERT(this->accounts); //accounts must exist
	AB_IMEXPORTER_CONTEXT *ctx = NULL;

	//get the AB_IMEXPORTER_CONTEXT for the history and save it to file
	ctx = this->history->getContext();
	abt_parser::save_local_ctx(ctx, settings->getHistoryFilename(),
				   "ctxfile", "default");
	AB_ImExporterContext_free(ctx);
}

//private
void MainWindow::on_actionDebug_Info_triggered()
{
	if (debugDialog->isVisible()) {
		debugDialog->hide();
	} else {
		debugDialog->showNormal();
	}
}

//private
/** @brief Slot is called when a job is added to the jobctrl */
void MainWindow::onJobAddedToJobCtrlList(const abt_jobInfo* ji) const
{
	this->ui->statusBar->showMessage(tr("Auftrag \"%1\" zum Ausgang "
					    "hinzugefügt").arg(ji->getType()),
					 6000);

	abt_dialog dia(NULL,
		       tr("Auftrag zum Ausgang hinzugefügt"),
		       tr("Der Auftrag \"%1\" wurde erfolgreich zum "
			  "Ausgangskorb hinzugefügt").arg(ji->getType()),
		       QDialogButtonBox::Ok, QDialogButtonBox::Ok,
		       QMessageBox::Information, "JobAddOutput");
	dia.exec();
}

//private
/** @brief item in ListWidget changed
 *
 * shows the corresponding page in the stackedWidget.
 */
void MainWindow::on_listWidget_currentItemChanged(QListWidgetItem* current,
						  QListWidgetItem* previous)
{
	if (!current) {
		current = previous;
	}

	this->ui->stackedWidget->setCurrentIndex(this->ui->listWidget->row(current));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
	qApp->aboutQt();
}

void MainWindow::on_actionAbout_abTransfers_triggered()
{
	QDialog *about = new QDialog(this);
	about->setWindowTitle(tr("Über %1").arg(qApp->applicationName()));

	//show the license text on pushButton click
	QDialog *licenseDialog = new QDialog(about);
	licenseDialog->setWindowTitle(tr("Lizenz"));
	QVBoxLayout *licenseLayout = new QVBoxLayout(licenseDialog);
	QLabel *licenseText = new QLabel(licenseDialog);
	QString lt = "Copyright (C) 2011-2013 Patrick Wacker<br /><br />"
		     "This program is free software; you can redistribute it and/or<br />"
		     "modify it under the terms of the GNU General Public License<br />"
		     "as published by the Free Software Foundation; either version 2<br />"
		     "of the License, or (at your option) any later version.<br /><br />"
		     "This program is distributed in the hope that it will be useful,<br />"
		     "but WITHOUT ANY WARRANTY; without even the implied warranty of<br />"
		     "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br />"
		     "GNU General Public License for more details.<br /><br />"
		     "You should have received a copy of the GNU General Public License<br />"
		     "along with this program; if not, write to the Free Software<br />"
		     "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.<br /><br />";
	lt.append(tr("siehe auch <a href=\"http://www.gnu.de/documents/gpl-2.0.de.html\">http://www.gnu.de/documents/gpl-2.0.de.html</a>"));
	licenseText->setText(lt);
	licenseText->setOpenExternalLinks(true);
	licenseText->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	licenseLayout->addWidget(licenseText);
	QPushButton *licenseClose = new QPushButton(tr("Schließen"));
	connect(licenseClose, SIGNAL(clicked()), licenseDialog, SLOT(accept()));
	licenseLayout->addWidget(licenseClose, 0, Qt::AlignRight);

	//Horizontal Layout
	QHBoxLayout *hbox = new QHBoxLayout();
	//Icon-Image as graphic top left
	QLabel *img = new QLabel(about);
	QPixmap *iconpic = new QPixmap(":/icons/bank-icon");
	img->setPixmap(*iconpic);
	img->setScaledContents(true);
	img->setMaximumSize(100, 100);
	hbox->addWidget(img);

	QLabel *text1 = new QLabel(QString::fromUtf8("<b>AB-Transfers</b><br><br>"
			     "Dieses Programm nutzt die Bibliothek AqBanking um Online-Banking-<br>"
			     "Transaktionen durchzuführen.<br><br>"
			     "Es sind alle wesentlichen Vorgänge von AqBanking implementiert,<br>"
			     "u.a. auch Überweisungen, Lastschriften, Daueraufträge usw.,<br>"
			     "sowie eine selbstimplementierte Verwaltung von Daueraufträgen<br>"
			     "und terminierten Überweisungen."));
	hbox->addWidget(text1, 0, Qt::AlignLeft);

	QVBoxLayout *vbox = new QVBoxLayout(about);
	vbox->addLayout(hbox);

	vbox->addSpacing(16);

	QLabel *author = new QLabel(QString("Author: Patrick Wacker"));
	vbox->addWidget(author, 0, Qt::AlignCenter);
	QLabel *version = new QLabel(QString("Version: %1").arg(qApp->applicationVersion()));
	vbox->addWidget(version, 0, Qt::AlignCenter);
#ifdef ABTRANSFER_VERSION_EXTRA
	QLabel *versionExtra = new QLabel(QString("<b>%1</b>").arg(ABTRANSFER_VERSION_EXTRA));
	vbox->addWidget(versionExtra, 0, Qt::AlignCenter);
#endif
	QLabel *versionSVN = new QLabel(QString("svn revision: %1").arg(ABTRANSFER_SVN_REVISION));
	vbox->addWidget(versionSVN, 0, Qt::AlignCenter);

	vbox->addSpacing(10);


	QLabel *webURL = new QLabel(tr("Website: <a href=\"%1\">%1</a>").arg("http://schmufu.dyndns.org/dokuwiki/ab_transfer:start"));
	webURL->setOpenExternalLinks(true);
	webURL->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	vbox->addWidget(webURL, 0, Qt::AlignLeft);

	QLabel *svnURL = new QLabel(tr("svn repo: <a href=\"%1\">%1</a>").arg("http://schmufu.dyndns.org/svn/ab_transfers/"));
	svnURL->setOpenExternalLinks(true);
	svnURL->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	vbox->addWidget(svnURL, 0, Qt::AlignLeft);

	QLabel *tracUrl = new QLabel(tr("trac: <a href=\"%1\">%1</a>").arg("http://schmufu.dyndns.org/trac/abtransfers/"));
	tracUrl->setOpenExternalLinks(true);
	tracUrl->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	vbox->addWidget(tracUrl, 0, Qt::AlignLeft);

	vbox->addSpacing(10);

	QHBoxLayout *hbox2 = new QHBoxLayout();
	QPushButton *ok = new QPushButton(tr("OK"));
	QPushButton *licenseBtn = new QPushButton(tr("Lizenz"));
	connect(licenseBtn, SIGNAL(clicked()), licenseDialog, SLOT(exec()));
	connect(ok, SIGNAL(clicked()), about, SLOT(accept()));
	hbox2->addWidget(licenseBtn, 0, Qt::AlignLeft);
	QSpacerItem *hbox2spacer = new QSpacerItem(10, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hbox2->addSpacerItem(hbox2spacer);
	hbox2->addWidget(ok, 0, Qt::AlignRight);
	vbox->addLayout(hbox2, 0);

	QLabel *usedImages = new QLabel(tr("<b>genutzte Grafiken:</b>"));
	vbox->addWidget(usedImages, 0, Qt::AlignLeft);

	QLabel *iconsUrl = new QLabel(QString("Icons used from Oxygen <a href=\"%1\">%1</a><br />"
					      "and from 'Ecommerce Business icon pack'<br />"
					      "<a href=\"%2\">%2</a>")
					      .arg("http://www.oxygen-icons.org")
					      .arg("http://www.iconspedia.com/pack/ecommerce-business-icons-4074"));
	iconsUrl->setOpenExternalLinks(true);
	iconsUrl->setAlignment(Qt::AlignHCenter);
	iconsUrl->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	vbox->addWidget(iconsUrl, 0, Qt::AlignCenter);

	vbox->addSpacing(6);

	about->exec();

	delete about;
}

//private slot
/** @brief Shows a help/FAQ dialog
 *
 * the underlying text is included as a resource and is read from
 * helpText.html in the src directory.
 */
void MainWindow::on_actionHelp_triggered()
{
	QDialog *helpDialog = new QDialog(this);
	helpDialog->setWindowTitle(tr("Hilfe / FAQ"));

	QVBoxLayout *vbox = new QVBoxLayout(helpDialog);

#if !defined(USE_QT_WEBKIT)
	//QtWebKit not available, we use a QLabel for the Display
	//ScrollArea for text display
	QScrollArea *scroll = new QScrollArea();
	scroll->setWidgetResizable(true);
	scroll->setMinimumSize(520, 600);
	scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	vbox->addWidget(scroll);

	//text from resource (helpText.html)
	QLabel *text1 = new QLabel();
	text1->setWordWrap(true);
	text1->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	text1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	text1->setTextFormat(Qt::RichText);

	QFile helpText(":/text/help");
	if (helpText.open(QFile::ReadOnly)) {
		QTextStream stream(&helpText);
		text1->setText(stream.readAll());
		scroll->setWidget(text1);
	}
#else //QtWebKit is available, we use a QWebView. (supporting 'local links')
	QWebView *view = new QWebView();
	view->settings()->setDefaultTextEncoding("utf-8");
	view->setMinimumSize(520, 600);

	QFile helpText(":/text/help");
	if (helpText.open(QFile::ReadOnly)) {
		QTextStream stream(&helpText);
		view->setHtml(stream.readAll());
	}

	vbox->addWidget(view);
#endif

	vbox->addSpacing(10);

	QPushButton *ok = new QPushButton(tr("OK"));
	connect(ok, SIGNAL(clicked()), helpDialog, SLOT(accept()));
	vbox->addWidget(ok, 0, Qt::AlignHCenter);

	vbox->addSpacing(6);

	helpDialog->exec();

	delete helpDialog;
}

//private slot
void MainWindow::on_actionEinstellungen_triggered()
{
	DialogSettings DiaSettings(settings, banking->getAqBanking(), this);

	QString classname = QObject::sender()->metaObject()->className();
	if (classname == "page_history") {
		//we were called from the history page
		DiaSettings.setActiveTab(1); //set im-/export as active tab
	}

	DiaSettings.exec();
}

//private slot
void MainWindow::DisplayNotAvailableTypeAtStatusBar(AB_JOB_TYPE type)
{
	QString msg;
	msg.append(abt_conv::JobTypeToQString(type));
	msg.append(tr(" - Auftrag wird von der Bank nicht unterstützt!"));
	this->ui->statusBar->showMessage(msg, 8000);
}

//private slot
void MainWindow::onAccountWidgetContextMenuRequest(QPoint p)
{
	BankAccountsWidget *acc = this->dock_Accounts->findChild<BankAccountsWidget*>();
	//only show the menu when a account is selected
	if (acc->getSelectedAccount() != NULL) {
		this->accountContextMenu->exec(this->dock_Accounts->widget()->mapToGlobal(p));
	}
}

//private slot
void MainWindow::selectedStandingOrdersAccountChanged(const aqb_AccountInfo* acc)
{
	if (acc == NULL) return; //cancel if no account is supplied
	settings->saveSelAccountInWidget("StandingOrders", acc);
}

//private slot
void MainWindow::selectedDatedTransfersAccountChanged(const aqb_AccountInfo* acc)
{
	if (acc == NULL) return; //cancel if no account is supplied
	settings->saveSelAccountInWidget("DatedTransfers", acc);
}

//private slot
void MainWindow::onActionTransferNationalTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeTransfer);
}

//private slot
void MainWindow::onActionTransferInternationalTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeEuTransfer);
}

//private slot
void MainWindow::onActionTransferSepaTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeSepaTransfer);
}

//private slot
void MainWindow::onActionTransferInternalTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeInternalTransfer);
}

//private slot
void MainWindow::onActionDatedNewTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeCreateDatedTransfer);
}

//private slot
void MainWindow::onActionDatedUpdateTriggered()
{
	BankAccountsWidget *acc = this->dock_Accounts->findChild<BankAccountsWidget*>();
	if (!acc) return; //cancel if no BankAccountsWidget was found
	this->jobctrl->addGetDatedTransfers(acc->getSelectedAccount());
}

//private slot
void MainWindow::onActionStandingNewTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeCreateStandingOrder);
}

//private slot
void MainWindow::onActionStandingUpdateTriggered()
{
	BankAccountsWidget *acc = this->dock_Accounts->findChild<BankAccountsWidget*>();
	if (!acc) return; //cancel if no BankAccountsWidget was found
	this->jobctrl->addGetStandingOrders(acc->getSelectedAccount());
}

//private slot
void MainWindow::onActionDebitNoteTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeDebitNote);
}

//private slot
void MainWindow::onActionDebitNoteSepaTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeSepaDebitNote);
}

//private slot
void MainWindow::onActionUpdateBalanceTriggered()
{
	BankAccountsWidget *acc = this->dock_Accounts->findChild<BankAccountsWidget*>();
	if (!acc) return; //cancel if no BankAccountsWidget was found
	this->jobctrl->addGetBalance(acc->getSelectedAccount());
}

//private slot
/** @brief shows a list of all supported and available jobs */
void MainWindow::onActionShowAvailableJobsTriggered()
{
	//show all supported and not supported jobs for the selected account
	BankAccountsWidget *BAW = this->dock_Accounts->findChild<BankAccountsWidget*>();
	aqb_AccountInfo *acc = BAW->getSelectedAccount();
	if (!acc) return; //no account selected, cancel

	QDialog *dialog = new QDialog(this);
	dialog->setWindowTitle(tr("Unterstützte Aufträge"));
	QGridLayout *gl = new QGridLayout(dialog);

	QLabel *text = new QLabel(tr("Anzeige der vom Institut unterstützten "
				     "Aufträge für das Konto %1 (%2). "
				     "Nicht alle Aufträge werden auch von "
				     "%3 unterstützt!").arg(
					acc->Number(), acc->Name(),
					qApp->applicationName()));
	text->setWordWrap(true);
	gl->addWidget(text, 0, 0, 1, -1); //use the text as a header
	gl->setRowMinimumHeight(1, 12); //one row as seperator

	//prepare used icons
	QIcon icoSup = QIcon::fromTheme("dialog-ok-apply", QIcon(":/icons/ok"));
	QIcon icoNotSup = QIcon::fromTheme("edit-delete", QIcon(":/icons/delete"));
	const QPixmap pixSup = icoSup.pixmap(16, QIcon::Normal); //supported
	const QPixmap pixNotSup = icoNotSup.pixmap(16, QIcon::Normal); //not supported
	const QPixmap pixLatSup = icoSup.pixmap(16, QIcon::Disabled); //later supported

	//create and display column header
	QLabel *text_bank = new QLabel(tr("Institut"));
	QLabel *text_abtransfer = new QLabel(tr("%1").arg(qApp->applicationName()));
	QLabel *text_description = new QLabel(tr("Auftrag"));
	gl->addWidget(text_bank, 2, 0, Qt::AlignCenter);
	gl->addWidget(text_abtransfer, 2, 1, Qt::AlignCenter);
	gl->addWidget(text_description, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
	gl->setRowMinimumHeight(3, 5); //small space

	gl->setHorizontalSpacing(10);

	//go through all jobs from AqBanking and display them
	QHashIterator<AB_JOB_TYPE, bool> it(*acc->availableJobsHash());
	it.toFront();
	int row = gl->rowCount(); //append new widgets
	while (it.hasNext()) {
		it.next();
		QLabel *txt = new QLabel(abt_conv::JobTypeToQString(it.key()));
		QLabel *IcoBank = new QLabel();
		QLabel *IcoABT = new QLabel();

		//Icons for jobs supported by bank
		if (it.value()) {
			IcoBank->setPixmap(pixSup);
		} else {
			IcoBank->setPixmap(pixNotSup);
		}

		//Icons for jobs supported by AB-Transfers
		switch (abt_settings::supportedByAbtransfers(it.key())) {
		case 1: IcoABT->setPixmap(pixSup);;
			break;
		case 2: IcoABT->setPixmap(pixLatSup);
			break;
		default:
			IcoABT->setPixmap(pixNotSup);
			break;
		}

		//display icons and text
		gl->addWidget(IcoBank, row, 0, Qt::AlignCenter);
		gl->addWidget(IcoABT, row, 1, Qt::AlignCenter);
		gl->addWidget(txt, row, 2, Qt::AlignLeft | Qt::AlignVCenter);

		row++; //next row
	}

	//descriptions for the meanings of symbols
	gl->setRowMinimumHeight(row++, 12); //space to list
	QLabel *labelTextDescription = new QLabel(tr("Symbolbedeutungen"));
	gl->addWidget(labelTextDescription, row++, 0, 1, -1, Qt::AlignLeft | Qt::AlignBottom);

	QLabel *labelTextSup = new QLabel(tr("unterstützt"));
	QLabel *labelTextNotSup = new QLabel(tr("nicht unterstützt"));
	QLabel *labelTextLatSup = new QLabel(tr("in zukünftiger Version geplant"));

	QLabel *labelIcoSup = new QLabel();
	QLabel *labelIcoNotSup = new QLabel();
	QLabel *labelIcoLatSup = new QLabel();
	labelIcoSup->setPixmap(pixSup);
	labelIcoNotSup->setPixmap(pixNotSup);
	labelIcoLatSup->setPixmap(pixLatSup);

	gl->addWidget(labelIcoSup, row, 0, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
	gl->addWidget(labelTextSup, row++, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

	gl->addWidget(labelIcoNotSup, row, 0, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
	gl->addWidget(labelTextNotSup, row++, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

	gl->addWidget(labelIcoLatSup, row, 0, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
	gl->addWidget(labelTextLatSup, row++, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

	gl->setColumnStretch(2, 50); //only the description should expand

	//actual size of the GridLayout is the minimum
	dialog->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	dialog->setMinimumSize(gl->sizeHint());
	dialog->setFixedSize(dialog->width(), dialog->height());
	dialog->exec();

	delete dialog;
}

#ifdef TESTWIDGETACCESS
//private slot (ONLY FOR TESTING!)
void MainWindow::onActionTestWidgetAccessTriggered()
{
	//Only used for test purposes!
	QDialog *dialog = new QDialog(this);
	QVBoxLayout *vb = new QVBoxLayout(dialog);
	pageWidgetTests *page = new pageWidgetTests(this->accounts);
	vb->addWidget(page);

	dialog->exec();

	delete dialog;
}
#endif

//private slot
/** @brief Tab should be closed
 *
 * checks if a not saved change exist and asks the user if the tab should
 * realy be closed or not.
 *
 * If the tab should realy be closed, this is delegated to
 * deleteTabWidgetAndTab()
 */
void MainWindow::on_tabWidget_UW_tabCloseRequested(int index)
{
	widgetTransfer *transW = dynamic_cast<widgetTransfer*>(this->ui->tabWidget_UW->widget(index));
	if (transW == NULL) {
		return; //child does not exist, do nothing
	}

	if (transW->hasChanges()) {
		int msgret = QMessageBox::question(
				     this, tr("Änderungen verwerfen?"),
				     tr("Im Tab '%1' wurden Änderungen "
					"vorgenommen!\n\n"
					"Sollen diese Änderungen verworfen werden?")
				     .arg(this->ui->tabWidget_UW->tabText(index)),
				     QMessageBox::Yes | QMessageBox::No,
				     QMessageBox::Yes);

		if (msgret != QMessageBox::Yes) {
			return; //user decided to not close the tab
		}
	}

	this->deleteTabWidgetAndTab(index);
}

//private
void MainWindow::deleteTabWidgetAndTab(int tabIndex)
{
	widgetTransfer *transW = dynamic_cast<widgetTransfer*>(this->ui->tabWidget_UW->widget(tabIndex));

	this->ui->tabWidget_UW->removeTab(tabIndex);
	delete transW;
}

//private
void MainWindow::deleteTabWidgetAndTab(const widgetTransfer *w)
{
	int tabIdx = -2;
	//This function should only be called when the supplied widgetTransfer
	//is currently displayed in the current tab.
	if (this->ui->tabWidget_UW->currentWidget() == w) {
		tabIdx = this->ui->tabWidget_UW->currentIndex();
	}

	if (tabIdx >= 0) {
		this->deleteTabWidgetAndTab(tabIdx);
	}
}

//private
/** @brief creates a new widgetTransfer and displays it in a new tab
 *
 * A new widgetTransfer for the supplied @a type and @a account is created.
 * If no @a account is supplied, the account selected in the BankAccountsWidget
 * is used.
 */
widgetTransfer* MainWindow::createTransferWidgetAndAddTab(AB_JOB_TYPE type,
							  const aqb_AccountInfo *account)
{
	BankAccountsWidget *bankAccW;
	const aqb_AccountInfo *acc;
	if (account == NULL) {
		bankAccW = this->dock_Accounts->findChild<BankAccountsWidget*>();
		acc = bankAccW->getSelectedAccount();
	} else {
		acc = account;
	}

	if (acc == NULL) {
		this->ui->statusBar->showMessage(tr("Kein Konto vorhanden! "
						    " -- Ist ein Konto in \"Online "
						    "Konten\" gewählt?"), 8000);
		return NULL; //without a account we cant do anything
	}

	widgetTransfer *trans = new widgetTransfer(type, acc, this->accounts, this);

	//add the new tab and set it as current
	int tabid = this->ui->tabWidget_UW->addTab(trans, abt_conv::JobTypeToQString(type));
	this->ui->tabWidget_UW->setCurrentIndex(tabid);

	connect(trans, SIGNAL(createTransfer(AB_JOB_TYPE,const widgetTransfer*)),
		this, SLOT(onWidgetTransferCreateTransfer(AB_JOB_TYPE,const widgetTransfer*)));
	connect(trans, SIGNAL(cancelClicked(widgetTransfer*)),
		this, SLOT(onWidgetTransferCancelClicked(widgetTransfer*)));

	//make sure that the summary page is displayed, so that the new tab
	//is visible
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

	return trans;
}

//private slot
void MainWindow::onWidgetTransferCancelClicked(widgetTransfer *sender)
{
	//find the widgetTransfer (*sender) at the TabWidget and remove it
	int tabIdx = this->ui->tabWidget_UW->indexOf(sender);
	this->on_tabWidget_UW_tabCloseRequested(tabIdx);
}

//private slot
/** @brief is called when a Transfer should be put to the outbox */
void MainWindow::onWidgetTransferCreateTransfer(AB_JOB_TYPE type, const widgetTransfer *sender)
{
	if (type == AB_Job_TypeCreateStandingOrder ||
	    type == AB_Job_TypeModifyStandingOrder) {
		//check if the inputs to recurrence are ok
		QString errMsg;
		bool inputOK = sender->isRecurrenceInputOk(errMsg);
		if (!inputOK) {
			bool accepted = this->correctRecurrenceDates(sender->recurrence);
			if (!accepted) {
				return; //user dont want the correction
			}
		}
	}


	//first we control the input at the Widget
	QString errMsg;
	if (! sender->isGeneralInputOk(errMsg)) {
		QMessageBox::critical(this,
				      tr("Fehlerhafte Eingaben"),
				      tr("Folgende Eingaben sind fehlerhaft:<br />"
					 "%1<br />"
					 "Bitte korrigieren Sie diese.").arg(errMsg),
				      QMessageBox::Ok);
		return; //cancel, inputs are not ok
	}

	//everthing seems fine. Create the corresponding transaction and
	//delegate it to the abt_job_ctrl for execution.
	switch (type) {
	case AB_Job_TypeTransfer:
		this->createAndSendTransfer(sender);
		break;
	case AB_Job_TypeCreateDatedTransfer :
		this->createAndSendDatedTransfer(sender);
		break;
	case AB_Job_TypeCreateStandingOrder :
		this->createAndSendStandingOrder(sender);
		break;
	case AB_Job_TypeModifyDatedTransfer :
		this->createAndSendModifyDatedTransfer(sender);
		break;
	case AB_Job_TypeModifyStandingOrder :
		this->createAndSendModifyStandingOrder(sender);
		break;
	case AB_Job_TypeDebitNote :
		this->createAndSendDebitNote(sender);
		break;
	case AB_Job_TypeEuTransfer :
		this->createAndSendEUTransfer(sender);
		break;
	case AB_Job_TypeSepaTransfer :
		this->createAndSendSepaTransfer(sender);
		break;
	case AB_Job_TypeInternalTransfer :
		this->createAndSendInternalTransfer(sender);
		break;
	case AB_Job_TypeSepaDebitNote :
		this->createAndSendSepaDebitNote(sender);
		break;
	case AB_Job_TypeGetBalance :
	case AB_Job_TypeDeleteDatedTransfer :
	case AB_Job_TypeDeleteStandingOrder :
	case AB_Job_TypeGetDatedTransfers :
	case AB_Job_TypeGetStandingOrders :
	case AB_Job_TypeGetTransactions :
	case AB_Job_TypeLoadCellPhone :
	case AB_Job_TypeUnknown :
	default:
		qWarning() << Q_FUNC_INFO << "type: " << type
			   << " - not supported! No Job added! ABBRUCH!";
		return;
		break;
	}

	//the transaction was put to the outbox, remove the widget and the tab
	this->deleteTabWidgetAndTab(sender);
}

//private
/**
 * When the input in widgetRecurrence are incorrect this function can be
 * called to correct the dates automatically.
 *
 * Before a correction the user is asked if he wants this. If he answered
 * with 'Yes' the function returns true and changes the dates in the
 * widgetRecurrence, otherwise it returns false and leaves the dates as is.
 */
bool MainWindow::correctRecurrenceDates(widgetRecurrence *recurrence) const
{
	QDate correctFirstDate, correctLastDate, correctNextDate;

	const QDate firstDate = recurrence->getFirstExecutionDate();
	const QDate lastDate = recurrence->getLastExecutionDate();
	const QDate nextDate = recurrence->getNextExecutionDate();

	//QDate DateEins(2012, 1, 31);
	//DateEins = DateEins.addMonths(2);
	//were are we?
	//31.01.2012 + 1 Monat  = 29.02.2012
	//29.02.2012 + 1 Monat  = 29.03.2012
	//31.01.2012 + 2 Monate = 31.03.2012
	//qDebug() << DateEins;

	//weekly or monthly
	AB_TRANSACTION_PERIOD period = recurrence->getPeriod();
	//execute every 'cycle' weeks/months
	int cycle = recurrence->getCycle();
	//depends on cylce (Weekday Mo(1),Di(2),Mi(2) or Day 1,2...31,[97],[98],[99])
	//99 (Ultimo) / 98 (Ultimo-1) / 97 (Ultimo-2)
	int execDay = recurrence->getExecutionDay();

	//we assume that the executionDay, period and cycle are selected
	//correct and calculate the first-, last- and nextDate.

	//copy the selected dates (const)
	correctFirstDate = firstDate;
	correctLastDate = lastDate;
	correctNextDate = nextDate;

	switch(period) {
	/** @todo place the different cases into functions for clarity */
	case AB_Transaction_PeriodWeekly: {
		//add days to firstDate until the weekday matches the selected
		while (correctFirstDate.dayOfWeek() != execDay) {
			correctFirstDate = correctFirstDate.addDays(1);
		}

		//nextDate must be the same as firstDate
		correctNextDate = correctFirstDate;

		//when lastDate is invalid, the standing order should
		//execute until it is deleted or edited (no end date)
		if (!lastDate.isValid()) break; //no further correction needed

		//Otherwise we adjust the end date to the right weekday.
		//In this case, the first weekday that matches with the selected
		//one before the selected end date.
		int diffTage  = correctFirstDate.daysTo(correctLastDate);
		if ( diffTage < (cycle * 7) ) {
			// - firstDate and lastDate must not be equal
			// - lastDate must not before firstDate
			correctLastDate = correctFirstDate.addDays(cycle * 7);
		} else {
			// - the cycle must match
			//deviation from selected weekday (could be 0)
			int remTage = diffTage % 7;
			//difference from selected cycle (could be 0)
			int diffWochen = ( diffTage - remTage ) % cycle;
			//set lastDate to a valid day
			correctLastDate = correctLastDate.addDays( -remTage -diffWochen*7 );
		}
	} //case AB_Transaction_PeriodWeekly:
		break;
	case AB_Transaction_PeriodMonthly: {
		//For firstDate we set the first day possible after the selected
		//day, in consideration to the selected executionDay.
		//For lastDate we set, if possible, a day before or equal to
		//the selected date.

		switch(execDay) {
		case 99: /* Ultimo   */
		case 98: /* Ultimo-1 */
		case 97: /* Ultimo-2 */ {
			//The execution should be on the last day [-1/-2] of
			//the month.

			int daysToEnd = 99 - execDay; //days to last day of month
			//at Ultimo: 0 / Ultimo-1: 1 / Ultimo-2: 2

			if (firstDate.day() > (firstDate.daysInMonth() - daysToEnd)) {
				//move first execution to next month
				correctFirstDate = firstDate.addMonths(1);
			}
			//set the "right" day
			correctFirstDate.setDate(correctFirstDate.year(),
						 correctFirstDate.month(),
						 correctFirstDate.daysInMonth() - daysToEnd);

			// --> correctFirstDate now valid

			correctNextDate = correctFirstDate;

			//when lastDate is invalid, the standing order should
			//execute until it is deleted or edited (no end date)
			if (!lastDate.isValid()) break; //no further correction

			//Otherwise we adjust the lastDate to a valid day.
			//We select a date which is before the selected lastDate
			//if possible. In consideration of the executionDay and
			//the cycle.

			int monthDiff = ( ( lastDate.year() - correctFirstDate.year() ) * 12 +
					  ( lastDate.month() - correctFirstDate.month() ) ) % cycle;

			//monthDiff is now equal to the months which the
			//lastDate must be bring forward (could be 0).
			correctLastDate = lastDate.addMonths( -monthDiff );

			//set a valid executionDay in lastDate
			correctLastDate.setDate(correctLastDate.year(),
						correctLastDate.month(),
						correctLastDate.daysInMonth() - daysToEnd);

			// - firstDate and lastDate must not be equal
			// - lastDate must not before firstDate
			if (correctFirstDate >= correctLastDate) {
				//move to the next cycle
				/** @todo Could it happen that we must go ahead
				 *	  2 cycles at once? Recalculate this!
				 */
				correctLastDate = correctFirstDate.addMonths(cycle);
				//correctLastDate was changed, therfore we set
				//the executionDay another time.
				correctLastDate.setDate(correctLastDate.year(),
							correctLastDate.month(),
							correctLastDate.daysInMonth() - daysToEnd);
			}

			// --> correctLastDate is now valid
		}
			break;
		default: //"Normal" Day
			//set "firstDay" correct

			//worst case:   31        30
			if ( firstDate.day() > execDay ) {
				//move execution to the next month
				correctFirstDate = firstDate.addMonths(1);
				//worst case: date.day() is now 30 (or 28) !
			}

			if ( correctFirstDate.daysInMonth() < execDay ) {
				//In this case we set the last day.
				//occurs very seldom, and should be right.
				correctFirstDate.setDate(correctFirstDate.year(),
							 correctFirstDate.month(),
							 correctFirstDate.daysInMonth());
			} else {
				correctFirstDate = correctFirstDate.addDays( execDay - correctFirstDate.day() );
			}

			// --> correctFirstDate is now valid

			correctNextDate = correctFirstDate;

			//when lastDate is invalid, the standing order should
			//execute until it is deleted or edited (no end date)
			if (!lastDate.isValid()) break; //no further correction

			//Otherwise we adjust the lastDate to a valid day.
			//We select a date which is before the selected lastDate
			//if possible. In consideration of the executionDay and
			//the cycle.

			int monthDiff = ( ( lastDate.year() - correctFirstDate.year() ) * 12 +
					  ( lastDate.month() - correctFirstDate.month() ) ) % cycle;

			//monthDiff is now equal to the months which the
			//lastDate must be bring forward (could be 0).
			correctLastDate = lastDate.addMonths( -monthDiff );

			//wenn der Ausführungstag nach dem gewählten Enddatum
			//liegt, in den vorherigen Zyklus wechseln.
			//Dies aber nur wenn lastDate <= correctLastDate
			//z.B. Ausführungstag = 27 / lastDate = 25.05.
			//     Zyklus = jeden Monat --> monthDiff = 0 !
			//dann ist lastDate == correctLastDate aber Ausführungstag
			//würde nach dem vorgegebenen lastDate liegen, somit
			//wählen wir 1 Monat früher aus.
			if ((lastDate <= correctLastDate) &&
			    (correctLastDate.day() < execDay)) {
				correctLastDate = correctLastDate.addMonths(-cycle);
			}

			//set a valid exection day in lastDate
			if ( correctLastDate.daysInMonth() < execDay ) {
				//In this case we set the last day.
				//occurs very seldom, and should be right.
				correctLastDate.setDate(correctLastDate.year(),
							correctLastDate.month(),
							correctLastDate.daysInMonth());
			} else {
				correctLastDate.setDate(correctLastDate.year(),
							correctLastDate.month(),
							execDay);
			}

			// - firstDate and lastDate must not be equal
			// - lastDate must not before firstDate
			if (correctFirstDate >= correctLastDate) {
				correctLastDate = correctFirstDate.addMonths(cycle);
				//correctLastDate was changed, therfore we set
				//the executionDay another time.
				if ( correctLastDate.daysInMonth() < execDay ) {
					//In this case we set the last day.
					//occurs very seldom, and should be right.
					correctLastDate.setDate(correctLastDate.year(),
								correctLastDate.month(),
								correctLastDate.daysInMonth());
				} else {
					correctLastDate.setDate(correctLastDate.year(),
								correctLastDate.month(),
								execDay);
				}
			}

			break;

		} //switch(execDay)

	} //case AB_Transaction_PeriodMonthly:
		break;
	default:
		//should never happen, we only know weekly or monthly.
		qWarning() << Q_FUNC_INFO << "neither weekly nor monthly! Dates not handled!";
		break;
	} //switch(period)

	QString strFirstDate = correctFirstDate.toString("ddd dd.MM.yyyy");
	QString strLastDate = correctLastDate.isValid() ? correctLastDate.toString("ddd dd.MM.yyyy") : tr("Bis auf weiteres");
	QString strNextDate = correctNextDate.toString("ddd dd.MM.yyyy");

	int ret;
	ret = QMessageBox::question(
		NULL,
		tr("Daten geändert"),
		tr("Die Daten für den Dauerauftrag sind in sich "
		   "nicht konsistent und würden auf die folgenden "
		   "Werte geändert werden:<br />"
		   "<table>"
		   "<tr><td>Erstmalig:</td><td>%1</td></tr>"
		   "<tr><td>Letztmalig:</td><td>%2</td></tr>"
		   "<tr><td>Nächste Ausf.:</td><td>%3</td></tr>"
		   "</table>"
		   "<br /><br />"
		   "Sollen die Daten auf diese Werte geändert "
		   "werden?").arg(strFirstDate, strLastDate, strNextDate),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::Yes);

	if (ret == QMessageBox::Yes) {
		recurrence->setFirstExecutionDay(correctFirstDate);
		recurrence->setLastExecutionDay(correctLastDate);
		recurrence->setNextExecutionDay(correctNextDate);
		return true;
	}

	return false;
}


//private
/** @brief checks if an standing order is already in the outbox and shows
 *	   a warning if.
 *
 * @returns true if the standing order is alreade in the outbox
 * @returns false if the standing order is not in the outbox
 */
bool MainWindow::isStandingOrderInOutbox(const abt_standingOrderInfo *soi)
{
	if (this->jobctrl->isTransactionInQueue(soi->getTransaction())) {
		QMessageBox::critical(this, tr("Bereits im Ausgang"),
			tr("<b>Der Dauerauftrag befindet sich bereits im "
			   "Ausgang!</b><br /><br />"
			   "Er wurde entweder schon bearbeitet oder soll "
			   "gelöscht werden. Solange sich zu diesem "
			   "Dauerauftrag bereits eine Änderung im Ausgang "
			   "befindet kann keine weitere Änderung "
			   "stattfinden.<br />"
			   "Bitte löschen oder Bearbeiten Sie den "
			   "entsprechenden Auftrag im Ausgang."),
			QMessageBox::Ok, QMessageBox::Ok);
		return true;
	}
	return false;
}

//private
/** @brief checks if the saved data are out-of-date and asks for further action
 *
 * It is checked if the first execution date is in the future. If this is not
 * the case, then it is assumed that the data is out-of-date and the user
 * is asked whether he will update the standing orders or not.
 *
 * The user could also ignore the warning, in this case this function would
 * return the same value as when the standing order data is not out-of-date.
 *
 * If an update should be done, the corresponding job is placed in the outbox
 * and it will be switched the outbox automatically.
 *
 * @returns false
 *	If the standing order could be edited (not out-of-date) or the user
 *	wants to edit it (ignores the update question).
 *
 * @returns true
 *	If the standing order should not be edited.
 */
bool MainWindow::isStandingOrderOutdated(const aqb_AccountInfo *acc,
					 const abt_standingOrderInfo *soi)
{
	if (QDate::currentDate() >= soi->getTransaction()->getFirstExecutionDate()) {
		//the saved standing order is out-of-date!
		int rv =
		QMessageBox::warning(this, tr("Daten veraltet"),
			tr("<b>Der gespeicherte Dauerauftrag ist veraltet!</b><br /><br />"
			   "Um sicher zu stellen das eine gültige Version des "
			   "Dauerauftrages gelöscht oder geändert wird, sollten "
			   "die Daueraufträge von diesem Konto aktualisiert "
			   "werden.<br /><br />"
			   "Soll eine Aktualisierung durchgeführt werden?"),
			QMessageBox::Yes | QMessageBox::Abort | QMessageBox::Ignore,
			QMessageBox::Yes);

		switch(rv) {
		case QMessageBox::Ignore: break; //User want to edit
		case QMessageBox::Yes:
			//update job to outbox
			this->jobctrl->addGetStandingOrders(acc);
			//activate the outbox
			this->ui->listWidget->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
			return true; //the standing order should not be edited
			break;
		default: //handled like abort
			return true; //the standing order should not be edited
			break;
		}
	}
	return false;
}

//private Slot
void MainWindow::onStandingOrderEditRequest(const aqb_AccountInfo *acc, const abt_standingOrderInfo *da)
{
	//cancel if standing order is already in outbox or is out-of-date
	if (this->isStandingOrderInOutbox(da)) return;
	if (this->isStandingOrderOutdated(acc, da)) return;

	widgetTransfer *transW;
	transW = this->createTransferWidgetAndAddTab(AB_Job_TypeModifyStandingOrder,
						     acc);
	transW->setValuesFromTransaction(da->getTransaction());
}

//private Slot
void MainWindow::onStandingOrderDeleteRequest(const aqb_AccountInfo *acc, const abt_standingOrderInfo *da)
{
	//cancel if standing order is already in outbox or is out-of-date
	if (this->isStandingOrderInOutbox(da)) return;
	if (this->isStandingOrderOutdated(acc, da)) return;

	this->jobctrl->addDeleteStandingOrder(acc, da->getTransaction());
}


//private
/** @brief checks and displays a warning if the dated transfer is already in
 *	   the outbox
 *
 * @returns true if the dated transfer is already in the outbox
 * @returns false if the dated transfer is not in the outbox
 */
bool MainWindow::isDatedTransferInOutbox(const abt_datedTransferInfo *dti)
{
	if (this->jobctrl->isTransactionInQueue(dti->getTransaction())) {
	      QMessageBox::critical(this, tr("Bereits im Ausgang"),
		      tr("<b>Die terminierte Überweisung befindet sich bereits im Ausgang!</b><br /><br />"
			 "Sie wurde entweder schon bearbeitet oder soll gelöscht werden. "
			 "Solange sich zu dieser terminierten Überweisung bereits eine Änderung "
			 "im Ausgang befindet kann keine weitere Änderung stattfinden.<br />"
			 "Bitte löschen oder bearbeiten Sie den entsprechenden "
			 "Auftrag im Ausgang."),
		      QMessageBox::Ok, QMessageBox::Ok);
		return true;
	}
	return false;
}

//private
/** @brief checks if the saved data are out-of-date and asks for further action
 *
 * It is checked if the date of the dated transfer is in the future. If this is
 * not the case, then it is assumed that the data is out-of-date and the user
 * is asked whether he will update the dated transfers or not.
 *
 * The user could also ignore the warning, in this case this function would
 * return the same value as when the dated transfer data is not out-of-date.
 *
 * If an update should be done, the corresponding job is placed in the outbox
 * and it will be switched the outbox automatically.
 *
 * @returns false
 *	If the dated transfer could be edited (not out-of-date) or the user
 *	wants to edit it (ignores the update question).
 *
 * @returns true
 *	If the dated transfer should not be edited.
 */
bool MainWindow::isDatedTransferOutdated(const aqb_AccountInfo *acc,
					 const abt_datedTransferInfo *dti)
{
	if (QDate::currentDate() >= dti->getTransaction()->getDate()) {
		//the saved dated transfer is out-of-date!
		int rv =
		QMessageBox::warning(this, tr("Daten veraltet"),
			tr("<b>Die gespeicherte terminierte Überweisung ist veraltet!</b><br /><br />"
			   "Das Auführungsdatum der terminierten Überweisung ist "
			   "bereits erreicht oder überschritten. Um sicher zu "
			   "stellen das die terminierte Überweisung noch nicht "
			   "ausgeführt wurde sollten diese aktualisiert "
			   "werden.<br />"
			   "<i>(Ein löschen oder bearbeiten könnte Fehler verursachen)</i>"
			   "<br /><br />"
			   "Soll eine Aktualisierung durchgeführt werden?"),
			QMessageBox::Yes | QMessageBox::Abort | QMessageBox::Ignore,
			QMessageBox::Yes);

		switch(rv) {
		case QMessageBox::Ignore: break; //User want to edit
		case QMessageBox::Yes:
			//update job to outbox
			this->jobctrl->addGetDatedTransfers(acc);
			//activate the outbox
			this->ui->listWidget->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
			return true; //the dated transfer should not be edited
			break;
		default: //handled like abort
			return true; //the dated transfer should not be edited
			break;
		}
	}
	return false;
}

//private Slot
void MainWindow::onDatedTransferEditRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo *di)
{
	//cancel if dated transfer is already in outbox or is out-of-date
	if (this->isDatedTransferInOutbox(di)) return;
	if (this->isDatedTransferOutdated(acc, di)) return;

	widgetTransfer *transW;
	transW = this->createTransferWidgetAndAddTab(AB_Job_TypeModifyDatedTransfer,
						     acc);
	transW->setValuesFromTransaction(di->getTransaction());
}

//private Slot
void MainWindow::onDatedTransferDeleteRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo *di)
{
	//cancel if dated transfer is already in outbox or is out-of-date
	if (this->isDatedTransferInOutbox(di)) return;
	if (this->isDatedTransferOutdated(acc, di)) return;

	this->jobctrl->addDeleteDatedTransfer(acc, di->getTransaction());
}

//private Slot
void MainWindow::onEditJobFromOutbox(int itemNr)
{
	/** @todo the newly created widgetTransfer already contains changes.
	 *	  This should be set in the widget, so that a warning is
	 *	  displayed when the user canceled the editing!
	 */

	widgetTransfer *transW;
	const aqb_AccountInfo *acc = NULL;

	//the itemNr is the position at the JobQueueList
	abt_jobInfo *job = this->jobctrl->jobqueueList()->at(itemNr);

	QString jobAccBankcode, jobAccNumber;
	jobAccBankcode = AB_Account_GetBankCode(AB_Job_GetAccount(job->getJob()));
	jobAccNumber =AB_Account_GetAccountNumber(AB_Job_GetAccount(job->getJob()));

	//get the account that matches the local transaction data
	acc = this->accounts->getAccount(jobAccNumber, jobAccBankcode);

	if (acc == NULL) {
		//account not found
		qWarning() << Q_FUNC_INFO << "account from the job not found, aborting";
		return;
	}

	transW = this->createTransferWidgetAndAddTab(job->getAbJobType(), acc);

	transW->setValuesFromTransaction(job->getTransaction());

	//remove the job from the JobQueueList
	this->jobctrl->deleteJob(job);
}

//private slot
void MainWindow::onJobCtrlQueueListChanged()
{
	this->outbox->refreshTreeWidget(this->jobctrl);
}

//private slot
void MainWindow::createTransferFromJob(const abt_jobInfo *ji)
{
	widgetTransfer *transW;
	const aqb_AccountInfo *acc = NULL;
	QString jobAccBankcode, jobAccNumber;

	jobAccBankcode = ji->getTransaction()->getLocalBankCode();
	jobAccNumber = ji->getTransaction()->getLocalAccountNumber();

	//get the account that matches the local transaction data
	acc = this->accounts->getAccount(jobAccNumber, jobAccBankcode);

	if (acc == NULL) { //no account found
		QString msg;
		msg.append(tr("Kein Account gefunden [%1/%2] - Erstellen nicht möglich")
			   .arg(jobAccNumber, jobAccBankcode));
		this->ui->statusBar->showMessage(msg, 8000);
		qWarning() << Q_FUNC_INFO << "account from the job not found, aborting";
		return;
	}

	transW = this->createTransferWidgetAndAddTab(ji->getAbJobType(), acc);

	transW->setValuesFromTransaction(ji->getTransaction());
}

//private slot
void MainWindow::deleteHistoryItems(QList<abt_jobInfo *> jiList)
{
	foreach(abt_jobInfo* ji, jiList) {
		this->history->remove(ji);
	}
}

//private slot
void MainWindow::onActionSaveAllDataTriggered()
{
	this->saveAccountData();
	this->saveHistoryData();
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendTransfer(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

	//We use the unix timestamp as our ID, so we can display the
	//date and time of the creation of the transaction ;)
	t->setIdForApplication(QDateTime::currentDateTime().toTime_t());

	t->setRemoteAccountNumber(sender->remoteAccount->getAccountNumber());
	t->setRemoteName(QStringList(sender->remoteAccount->getName()));
	t->setRemoteBankCode(sender->remoteAccount->getBankCode());
	t->setRemoteBankName(sender->remoteAccount->getBankName());

	t->setValue(sender->value->getValueABV());

	t->setPurpose(sender->purpose->getPurpose());

	t->setTextKey(sender->textKey->getTextKey());

	this->jobctrl->addNewSingleTransfer(acc, t);

	delete t;
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendEUTransfer(const widgetTransfer* /* not used yet: sender */)
{
	qWarning() << "create EU Transfer not implemented yet!";
	this->statusBar()->showMessage("create EU Transfer not implemented yet!");
	return;

//	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
//	abt_transaction *t = new abt_transaction();
//
//	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());
//
//	//We use the unix timestamp as our ID, so we can display the
//	//date and time of the creation of the transaction ;)
//	t->setIdForApplication(QDateTime::currentDateTime().toTime_t());
//
//	t->setRemoteAccountNumber(sender->remoteAccount->getAccountNumber());
//	t->setRemoteName(QStringList(sender->remoteAccount->getName()));
//	t->setRemoteBankCode(sender->remoteAccount->getBankCode());
//	t->setRemoteBankName(sender->remoteAccount->getBankName());
//
//	t->setValue(sender->value->getValueABV());
//
//	t->setPurpose(sender->purpose->getPurpose());
//
//	t->setTextKey(sender->textKey->getTextKey());
//
//	this->jobctrl->addNewEuTransfer(acc, t);
//
//	delete t;
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendDatedTransfer(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

	//We use the unix timestamp as our ID, so we can display the
	//date and time of the creation of the transaction ;)
	t->setIdForApplication(QDateTime::currentDateTime().toTime_t());

	t->setRemoteAccountNumber(sender->remoteAccount->getAccountNumber());
	t->setRemoteName(QStringList(sender->remoteAccount->getName()));
	t->setRemoteBankCode(sender->remoteAccount->getBankCode());
	t->setRemoteBankName(sender->remoteAccount->getBankName());

	t->setValue(sender->value->getValueABV());

	t->setPurpose(sender->purpose->getPurpose());

	t->setTextKey(sender->textKey->getTextKey());

	t->setDate(sender->datedDate->getDate());

	this->jobctrl->addCreateDatedTransfer(acc, t);

	delete t;
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendStandingOrder(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

	//We use the unix timestamp as our ID, so we can display the
	//date and time of the creation of the transaction ;)
	t->setIdForApplication(QDateTime::currentDateTime().toTime_t());

	t->setRemoteAccountNumber(sender->remoteAccount->getAccountNumber());
	t->setRemoteName(QStringList(sender->remoteAccount->getName()));
	t->setRemoteBankCode(sender->remoteAccount->getBankCode());
	t->setRemoteBankName(sender->remoteAccount->getBankName());

	t->setValue(sender->value->getValueABV());

	t->setPurpose(sender->purpose->getPurpose());

	t->setTextKey(sender->textKey->getTextKey());

	t->setCycle(sender->recurrence->getCycle());
	t->setPeriod(sender->recurrence->getPeriod());
	t->setExecutionDay(sender->recurrence->getExecutionDay());
	//t->setDate(sender->recurrence->getFirstExecutionDate());
	//t->setValutaDate(sender->recurrence->getFirstExecutionDate());
	t->setFirstExecutionDate(sender->recurrence->getFirstExecutionDate());
	t->setLastExecutionDate(sender->recurrence->getLastExecutionDate());
	t->setNextExecutionDate(sender->recurrence->getNextExecutionDate());

	this->jobctrl->addCreateStandingOrder(acc, t);

	delete t;
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendSepaTransfer(const widgetTransfer* sender)
{
	qWarning() << "create SEPA Transfer implemented, but not well tested!";

	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

	//We use the unix timestamp as our ID, so we can display the
	//date and time of the creation of the transaction ;)
	t->setIdForApplication(QDateTime::currentDateTime().toTime_t());

	t->setRemoteIban(sender->remoteAccount->getIBAN());
	t->setRemoteName(QStringList(sender->remoteAccount->getName()));
	t->setRemoteBic(sender->remoteAccount->getBIC());
	t->setRemoteBankName(sender->remoteAccount->getBankName());

	t->setValue(sender->value->getValueABV());

	t->setPurpose(sender->purpose->getPurpose());

	/** @todo could the textkey be set for sepa transfers? */
	//t->setTextKey(sender->textKey->getTextKey());

	this->jobctrl->addNewSepaTransfer(acc, t);

	delete t;
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendModifyDatedTransfer(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	const abt_transaction *origT = sender->getOriginalTransaction();

	//copy the original transaction (because of const)
	abt_transaction *newT = new abt_transaction(*origT);
	//and modify the copy instead
	newT->fillLocalFromAccount(acc->get_AB_ACCOUNT());

	//We use the unix timestamp as our ID, so we can display the
	//date and time of the creation of the transaction ;)
	newT->setIdForApplication(QDateTime::currentDateTime().toTime_t());

	newT->setRemoteAccountNumber(sender->remoteAccount->getAccountNumber());
	newT->setRemoteName(QStringList(sender->remoteAccount->getName()));
	newT->setRemoteBankCode(sender->remoteAccount->getBankCode());
	newT->setRemoteBankName(sender->remoteAccount->getBankName());

	newT->setValue(sender->value->getValueABV());

	newT->setPurpose(sender->purpose->getPurpose());

	newT->setTextKey(sender->textKey->getTextKey());

	newT->setDate(sender->datedDate->getDate());

	this->jobctrl->addModifyDatedTransfer(acc, newT);

	delete newT;
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendModifyStandingOrder(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	const abt_transaction *origT = sender->getOriginalTransaction();

	//copy the original transaction (because of const)
	abt_transaction *newT = new abt_transaction(*origT);
	//and modify the copy instead
	newT->fillLocalFromAccount(acc->get_AB_ACCOUNT());

	//We use the unix timestamp as our ID, so we can display the
	//date and time of the creation of the transaction ;)
	newT->setIdForApplication(QDateTime::currentDateTime().toTime_t());

	newT->setRemoteAccountNumber(sender->remoteAccount->getAccountNumber());
	newT->setRemoteName(QStringList(sender->remoteAccount->getName()));
	newT->setRemoteBankCode(sender->remoteAccount->getBankCode());
	newT->setRemoteBankName(sender->remoteAccount->getBankName());

	newT->setValue(sender->value->getValueABV());

	newT->setPurpose(sender->purpose->getPurpose());

	newT->setTextKey(sender->textKey->getTextKey());

	newT->setCycle(sender->recurrence->getCycle());
	newT->setPeriod(sender->recurrence->getPeriod());
	newT->setExecutionDay(sender->recurrence->getExecutionDay());
	newT->setFirstExecutionDate(sender->recurrence->getFirstExecutionDate());
	newT->setLastExecutionDate(sender->recurrence->getLastExecutionDate());
	newT->setNextExecutionDate(sender->recurrence->getNextExecutionDate());

	this->jobctrl->addModifyStandingOrder(acc, newT);

	delete newT;
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendDebitNote(const widgetTransfer *sender)
{
	qWarning() << "create Debit Note not implemented yet!";
	this->statusBar()->showMessage("create Debit Note not implemented yet!");
	return;

	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

	//We use the unix timestamp as our ID, so we can display the
	//date and time of the creation of the transaction ;)
	t->setIdForApplication(QDateTime::currentDateTime().toTime_t());

	t->setRemoteAccountNumber(sender->remoteAccount->getAccountNumber());
	t->setRemoteName(QStringList(sender->remoteAccount->getName()));
	t->setRemoteBankCode(sender->remoteAccount->getBankCode());
	t->setRemoteBankName(sender->remoteAccount->getBankName());

	t->setValue(sender->value->getValueABV());

	t->setPurpose(sender->purpose->getPurpose());

	t->setTextKey(sender->textKey->getTextKey());

	this->jobctrl->addNewSingleDebitNote(acc, t);

	delete t;
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendInternalTransfer(const widgetTransfer *sender)
{
	//internal transfer between 2 accounts at the same bank
	const aqb_AccountInfo *fromAcc = sender->localAccount->getAccount();
	const aqb_AccountInfo *toAcc = sender->remoteAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(fromAcc->get_AB_ACCOUNT());

	//We use the unix timestamp as our ID, so we can display the
	//date and time of the creation of the transaction ;)
	t->setIdForApplication(QDateTime::currentDateTime().toTime_t());

	t->setRemoteAccountNumber(toAcc->Number());
	t->setRemoteName(QStringList(toAcc->OwnerName()));
	t->setRemoteBankCode(toAcc->BankCode());
	t->setRemoteBankName(toAcc->BankName());

	t->setValue(sender->value->getValueABV());

	t->setPurpose(sender->purpose->getPurpose());

	t->setTextKey(sender->textKey->getTextKey());

	this->jobctrl->addNewInternalTransfer(fromAcc, t);

	delete t;
}

//private
/** must only be called if all inputs are valid */
void MainWindow::createAndSendSepaDebitNote(const widgetTransfer* /* not used yet: sender */)
{
	qWarning() << "create SEPA Debit Note not implemented yet!";
	this->statusBar()->showMessage("create SEPA Debit Note not implemented yet!");
	return;

//	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
//	abt_transaction *t = new abt_transaction();
//
//	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());
//
//	//We use the unix timestamp as our ID, so we can display the
//	//date and time of the creation of the transaction ;)
//	t->setIdForApplication(QDateTime::currentDateTime().toTime_t());
//
//	t->setRemoteAccountNumber(sender->remoteAccount->getAccountNumber());
//	t->setRemoteName(QStringList(sender->remoteAccount->getName()));
//	t->setRemoteBankCode(sender->remoteAccount->getBankCode());
//	t->setRemoteBankName(sender->remoteAccount->getBankName());
//
//	t->setValue(sender->value->getValueABV());
//
//	t->setPurpose(sender->purpose->getPurpose());
//
//	t->setTextKey(sender->textKey->getTextKey());
//
//	this->jobctrl->addNewSingleTransfer(acc, t);
//
//	delete t;
}


//private
void MainWindow::appendGetBalanceToOutbox() const
{
	foreach(aqb_AccountInfo *acc, this->accounts->getAccountHash().values()) {
		this->jobctrl->addGetBalance(acc, true);
	}
}

//private
void MainWindow::appendGetDatedTransfersToOutbox() const
{
	foreach(aqb_AccountInfo *acc, this->accounts->getAccountHash().values()) {
		this->jobctrl->addGetDatedTransfers(acc, true);
	}
}
//private
void MainWindow::appendGetStandingOrdersToOutbox() const
{
	foreach(aqb_AccountInfo *acc, this->accounts->getAccountHash().values()) {
		this->jobctrl->addGetStandingOrders(acc, true);
	}
}

/** \brief Checks for DatedTransfers which execution date is reached
 *
 * If a DatedTransfer is found that has an execution date which is older or
 * equal to the current date, then a message is displayed that the known
 * DatedTransfers should be updated.
 *
 */
//private
void MainWindow::checkReachedDatedTransfers()
{
	QString msgText = "";
	foreach(aqb_AccountInfo *acc, this->accounts->getAccountHash().values()) {
		//next account if this account does not have a datedTransfers-List
		if (acc->getKnownDatedTransfers() == NULL) continue;
		for(int i=0; i < acc->getKnownDatedTransfers()->size(); ++i) {
			abt_datedTransferInfo *dt = acc->getKnownDatedTransfers()->at(i);
			if (dt->getTransaction()->getDate() <= QDate::currentDate()) {
				msgText.append("<table>");
				msgText.append(tr("<tr><td>Begünstigter:</td><td>%1</td></tr>").arg(dt->getTransaction()->getRemoteName().at(0)));
				msgText.append(tr("<tr><td>Betrag:</td><td>%1</td></tr>").arg(abt_conv::ABValueToString(dt->getTransaction()->getValue(), true)));
				msgText.append(tr("<tr><td><b>Ausführen am:</b></td><td>%1</td></tr>").arg(dt->getTransaction()->getDate().toString(Qt::SystemLocaleShortDate)));
				msgText.append("</table>");
			}
		}
	}

	if (!msgText.isEmpty()) {
		msgText.prepend(tr("Folgende terminierte Überweisungen haben "
				   "den Ausführungstag erreicht oder "
				   "überschritten:<br />"));
		msgText.append(tr("<br /><br />"
				  "Es wird empfohlen die terminierten "
				  "Überweisungen zu aktualisieren.<br />"
				  "<br />"
				  "Soll bei allen Konten, die terminierte "
				  "Überweisungen unterstützen, eine "
				  "Aktualisierung durchgeführt werden?"));

		QMessageBox::StandardButton sel;
		sel = QMessageBox::information(this, tr("Terminüberweisung"),
					       msgText, QMessageBox::Yes,
					       QMessageBox::No);
		if (sel == QMessageBox::Yes) {
			this->appendGetDatedTransfersToOutbox();
			this->jobctrl->execQueuedTransactions();
		}
	}

}

void MainWindow::on_actionAqBankingSetup_triggered()
{
	GWEN_DIALOG *dlg;
	int rv;

	/* The Setup dialog of AqBanking can only be executed if no jobs are
	 * at the outbox, or rather no data from the accounts is used in other
	 * objects that would be invalid after the execution of the setup
	 * dialog (e.g. pointer to aqb_accountInfo objects!).
	 */

	int outboxCnt = this->jobctrl->jobqueueList()->size();
	int editCnt = this->ui->tabWidget_UW->count() - 1; //summary is always present

	if ((outboxCnt != 0) || (editCnt != 0)) {
		QMessageBox::information(this,
					 tr("AqBanking einrichten ..."),
					 tr("\"AqBanking einrichten ...\" kann nur "
					    "aufgerufen werden wenn keine Aufträge "
					    "im Ausgang sind und auch keine Aufträge "
					    "in Bearbeitung sind.<br />"
					    "Wenn ein Auftrag im Ausgang ist und "
					    "das entsprechende Konto gelöscht "
					    "werden würde, würde das Ausführen "
					    "zu einem Absturz führen!<br />"
					    "<br />"
					    "Bitte schließen Sie alle Bearbeitungen "
					    "vollständig ab und rufen erst dann "
					    "\"AqBanking einrichten ...\" auf."),
					 QMessageBox::Ok, QMessageBox::Ok);
		return; //cancel
	}


	dlg = AB_SetupDialog_new(banking->getAqBanking());
	if (!dlg) {
		qWarning() << Q_FUNC_INFO << "could not create AqBanking setup dialog";
		return;
	}

	/* here we must delete all accounts. After the AqBanking-Setup dialog
	 * was executed all data is read again and the objects are recreated
	 * with possible new values.
	 */

	//all references to accounts are set to NULL, therefore they are
	//invalid in the corresponding widgets!
	BankAccountsWidget *baw = this->dock_Accounts->findChild<BankAccountsWidget*>();
	baw->setAccounts(NULL);

	widgetAccountComboBox *accBoxDated = this->dock_KnownDatedTransfers->findChild<widgetAccountComboBox*>();
	accBoxDated->setAllAccounts(NULL);
	widgetKnownDatedTransfers *datedTransfers = this->dock_KnownDatedTransfers->findChild<widgetKnownDatedTransfers*>();
	datedTransfers->setAccount(NULL);

	widgetAccountComboBox *accBoxStanding = this->dock_KnownStandingOrders->findChild<widgetAccountComboBox*>();
	accBoxStanding->setAllAccounts(NULL);
	widgetKnownStandingOrders *standingOrders = this->dock_KnownStandingOrders->findChild<widgetKnownStandingOrders*>();
	standingOrders->setAccount(NULL);

	this->saveAccountData(); //save all account data

	//delete all accounts
	delete this->jobctrl; //also removes the connections!
	delete this->accounts;

	//execute the AqBanking-Setup dialog
	rv = GWEN_Gui_ExecDialog(dlg, 0);
	if (rv == 0) {
		qDebug() << Q_FUNC_INFO << "AqBanking setup dialog aborted by user";
	}

	/* The AqBanking-Setup dialog was executed, now we must recreate all
	 * account objects and reload the data.
	 * Also the all connections must be reestablished.
	 */

	//recreate all accounts
	this->accounts = new aqb_Accounts(banking->getAqBanking());

	//recreate the JobController and the corresponding connections
	this->createJobCtrlAndConnections();

	//reload all account data
	this->loadAccountData();

	//set the new accounts in the widgets
	this->dockDatedTransfersSetAccounts();
	this->dockStandingOrdersSetAccounts();
	baw->setAccounts(this->accounts); //baw was assigned above

	//recreate the data at the ScrollArea
	this->createWidgetsInScrollArea();

	GWEN_Dialog_free(dlg);
}
