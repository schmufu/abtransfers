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

#include "abt_parser.h"

#ifdef TESTWIDGETACCESS
	 //nur zum Testen!
#include "pages/pagewidgettests.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	this->accounts = new aqb_Accounts(banking->getAqBanking());
	this->history = new abt_history(this);
	this->logw = new page_log();
	this->outbox = new Page_Ausgang();
	this->dock_KnownRecipient = NULL;
	this->dock_KnownStandingOrders = NULL;
	this->dock_KnownDatedTransfers = NULL;
	this->dock_Accounts = NULL;

	//Alle Accounts von AqBanking wurden erstellt (this->accounts), jetzt
	//können die Daten mit dem parser geladen werden
	this->loadAccountData();
	//Auch die History-Daten müssen geladen werden
	this->loadHistoryData();

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

	//default DockOptions setzen
	this->setDockOptions(QMainWindow::AllowNestedDocks |
			     QMainWindow::AllowTabbedDocks);// |
			     //QMainWindow::AnimatedDocks);
	this->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	//Alle DockWidgets erstellen
	this->createDockKnownRecipients();
	this->createDockBankAccountWidget();
	this->createDockStandingOrders();
	this->createDockDatedTransfers();

	this->createJobCtrlAndConnections(); //zwingend nach createDock...

	this->createActions();
	this->createMenus();
	this->createDockToolbar();

	this->createWidgetsInScrollArea();

	//Den PushButtons im Übersichts-Widget die entsprechenden Actions
	//zuordnen
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

	//Immer die Übersicht als Startseite anzeigen, egal was im .ui definiert
	//ist und den default-Entry im ListWidget auf Überweisung setzen
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
	this->ui->stackedWidget->setCurrentIndex(0);


	//Wir starten hier einen Timer, dieser arbeitet zwar schon, der Slot
	//wird aber erst aufgerufen wenn die EventLoop der Anwendung startet.
	//Dies ist erst der Fall wenn app.exec() in main() ausgeführt wird.
	//Somit kann in dem Slot TimerTimeOut() Code direkt nach der
	//Initialisierung der Anwendung und wenn diese auch vollständig gestartet
	//ist ausgeführt werden.
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

	delete this->outbox;	//AusgangsWidget löschen
	delete this->logw;	//LogWidget löschen
	delete this->jobctrl;	//jobControl-Object löschen
	delete this->history;
	delete this->accounts;	//account-Object löschen
	delete ui;

	qDebug() << Q_FUNC_INFO << "deleted";
}

void MainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

//protected
void MainWindow::closeEvent(QCloseEvent *e)
{
	//Überprüfen ob noch Aufträge im Ausgang sind, wenn ja den Benutzer
	//fragen ob wirklich beendet werden soll.
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
			//Es soll noch nicht beendet werden!
			e->ignore();
			return;
		}
	}

	/** \todo Überprüfung ob Speicherung notwendig
	  *
	  * Das speichern sollte in eine separate Funktion und in den
	  * Einstellungen auch angebar sein dass z.B. nach einem erfolgreichen
	  * Aktualisiseren automatisch gespeichert wird.
	  */

	//Alle Daten speichern
	this->actSaveAllData->trigger();

	//jetzt können wir geschlossen werden
	e->accept();
}

//private Slot
void MainWindow::TimerTimeOut()
{
	//Der Timer läuft erst ab wenn die execLoop gestartet ist. Somit wird
	//dieser Code erst ausgeführt wenn das MainWindow angezeigt und die
	//EventLoop der Anwendung läuft. Danach dann nie wieder!

	disconnect(this, SLOT(TimerTimeOut())); //connection entfernen
	delete this->timer; //Der Timer wird nicht länger benötigt
	this->timer = NULL;


	abt_dialog dia(this,
		       tr("eventuelle Kosten"),
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

	//überprüfen ob Aufträge nach dem Start in den Ausgang gestellt werden
	//sollen und ob diese auch gleich ausgeführt werden sollen.
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
	actSaveAllData->setIcon(QIcon::fromTheme("document-save"));
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
	//wir löschen alles in der ScrollArea, und erstellen dann alles neu

	//Vielliecht besitzen wir schon ein Layout
	QVBoxLayout *layoutScrollArea = dynamic_cast<QVBoxLayout*>(this->ui->scrollAreaWidgetContents->layout());
	if (layoutScrollArea) { //layout vorhanden somit sind auch QGroupBoxen
		//vorhanden, alle childs löschen
		QList<QGroupBox*> list = this->ui->scrollAreaWidgetContents->findChildren<QGroupBox*>();
		while(!list.isEmpty()) {
			delete list.takeFirst();
		}
	} else { //neues Layout erstellen
		layoutScrollArea = new QVBoxLayout(this->ui->scrollAreaWidgetContents);
	}

	foreach(const aqb_AccountInfo *acc, this->accounts->getAccountHash().values()) {
		//Bekannte Daueraufträge
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

		//Bekannte Terminüberweisungen
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

/** \brief Erstellt das "Online Konten" QDockWidget
  *
  * Es wird ein neues QDockWidget (Online Konten) erstellt.
  * Diesem wird dann ein neues BankAccountsWidget zugewiesen.
  * Und das neue DockWidget wird dann dem MainWindow zugeteilt.
  *
  * Außerdem wird das entsprechende Signal mit dem Slot zur Anzeige eines
  * CustomContextMenu's verbunden.
  */
//private
void MainWindow::createDockBankAccountWidget()
{
	//DockWidget für Accounts erstellen
	this->dock_Accounts = new QDockWidget(tr("Online Konten"),this);
	this->dock_Accounts->setObjectName("OnlineAccounts");
	qDebug() << "creating bankAccountsWidget";
	BankAccountsWidget *baw = new BankAccountsWidget(this->accounts, this->dock_Accounts);
	this->dock_Accounts->setWidget(baw);
	//this->dock_Accounts->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
	this->dock_Accounts->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_Accounts->setFloating(false);
	this->dock_Accounts->hide();
	this->dock_Accounts->toggleViewAction()->setIcon(QIcon(":/icons/bank-icon"));
	this->addDockWidget(Qt::TopDockWidgetArea, this->dock_Accounts);
	connect(baw, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(onAccountWidgetContextMenuRequest(QPoint)));
}

/** \brief Erstellt das "Bekannte Empfänger" QDockWidget
  *
  * Es wird ein neues QDockWidget (Bekannte Empfänger) erstellt.
  * Diesem wird dann ein neues KnwonEmfpaengerWidget zugewiesen.
  * Und das neue DockWidget wird dann dem MainWindow zugeteilt.
  *
  * Außerdem werden die Signale des KnownEmfpängerWidget mit den entprechenden
  * Slots des MainWindow verbunden.
  */
//private
void MainWindow::createDockKnownRecipients()
{
	//DockWidget für KnownRecipients erstellen
	this->dock_KnownRecipient = new QDockWidget(tr("Bekannte Empfänger"),this);
	this->dock_KnownRecipient->setObjectName("KnownRecipients");
	qDebug() << "creating knownEmpfaengerWidget";
	KnownEmpfaengerWidget *kew = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient);
	//Änderungen der EmpfängerListe dem Widget bekanntgeben
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
	QDockWidget *dock = new QDockWidget("Daueraufträge", this);
	dock->setObjectName("dockStandingOrders");

	QVBoxLayout *layoutDock = new QVBoxLayout();
	QHBoxLayout *layoutAcc = new QHBoxLayout();
	QLabel *accText = new QLabel(tr("Konto"));

	widgetAccountComboBox *accComboBox = new widgetAccountComboBox(NULL, NULL);

	widgetKnownStandingOrders *StandingOrders;
	//Das DockWidget muss mit dem in der comboBox gewählten Account erstellt
	//werden, da wenn die ComboBox mit "NULL" erstellt wurde trotzdem das
	//erste Konto gewählt ist! (somit Erstellung des Dock mit Account != NULL)
	StandingOrders = new widgetKnownStandingOrders();

	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		StandingOrders, SLOT(setAccount(const aqb_AccountInfo*)));
	//damit Änderungen der Auswahl auch in der settings.ini gespeichert werden
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

void MainWindow::dockStandingOrdersSetAccounts()
{
	widgetAccountComboBox *accComboBox = this->dock_KnownStandingOrders->findChild<widgetAccountComboBox*>();
	widgetKnownStandingOrders *standingOrders = this->dock_KnownStandingOrders->findChild<widgetKnownStandingOrders*>();

	if ((accComboBox == NULL) || (standingOrders == NULL)) {
		return; //ein widget fehlt, abbruch
	}

	//den zuletzt gewählten Account herausfinden
	int selAccID = settings->loadSelAccountInWidget("StandingOrders");
	const aqb_AccountInfo *lastAcc = this->accounts->getAccount(selAccID);

	//Alle bekannten Accounts in der ComboBox setzen
	accComboBox->setAllAccounts(this->accounts);
	//und den zuletzt gewählten Anzeigen
	accComboBox->setSelectedAccount(lastAcc);

	//Über das ändern des selectedAccounts in der accComboBox wird automatisch
	//der account in standingOrders geändert!
	//standingOrders->setAccount(lastAcc);
}

//private
void MainWindow::createDockDatedTransfers()
{
	QDockWidget *dock = new QDockWidget("Terminüberweisungen", this);
	dock->setObjectName("dockDatedTransfers");

	QVBoxLayout *layoutDock = new QVBoxLayout();
	QHBoxLayout *layoutAcc = new QHBoxLayout();
	QLabel *accText = new QLabel(tr("Konto"));

	widgetAccountComboBox *accComboBox = new widgetAccountComboBox(NULL, NULL);

	widgetKnownDatedTransfers *DatedTransfers;
	//Das DockWidget muss mit dem in der comboBox gewählten Account erstellt
	//werden, da wenn die ComboBox mit "NULL" erstellt wurde trotzdem das
	//erste Konto gewählt ist! (somit Erstellung des Dock mit Account != NULL)
	DatedTransfers = new widgetKnownDatedTransfers();

	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		DatedTransfers, SLOT(setAccount(const aqb_AccountInfo*)));

	//Änderungen der Account-Wahl in der settings.ini speichern
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
		return; //ein widget fehlt, abbruch
	}

	//den zuletzt gewählten Account herausfinden
	int selAccID = settings->loadSelAccountInWidget("DatedTransfers");
	const aqb_AccountInfo *lastAcc = this->accounts->getAccount(selAccID);

	//Alle bekannten Accounts in der ComboBox setzen
	accComboBox->setAllAccounts(this->accounts);
	//und den zuletzt gewählten Anzeigen
	accComboBox->setSelectedAccount(lastAcc);

	//Über das ändern des selectedAccounts in der accComboBox wird automatisch
	//der account in standingOrders geändert!
	//standingOrders->setAccount(lastAcc);
}


//private
void MainWindow::createJobCtrlAndConnections()
{
	Q_ASSERT(this->history); //Das History Object muss vorhanden sein
	Q_ASSERT(this->accounts); //Accounts müssen vorhanden sein!

	this->jobctrl = new abt_job_ctrl(this->accounts, this->history, this);

	/***** Signals und Slots der Objecte verbinden ******/
	//Nicht mögliche Aufträge in der StatusBar anzeigen
	connect(this->jobctrl, SIGNAL(jobNotAvailable(AB_JOB_TYPE)),
		this, SLOT(DisplayNotAvailableTypeAtStatusBar(AB_JOB_TYPE)));

	//über erfolgreich hinzugefügte jobs wollen wir informiert werden
	connect(this->jobctrl, SIGNAL(jobAdded(const abt_jobInfo*)),
		this, SLOT(onJobAddedToJobCtrlList(const abt_jobInfo*)));

	//Logs von abt_job_ctrl in der Log-Seite anzeigen
	connect(this->jobctrl, SIGNAL(log(QString)),
		this->logw, SLOT(appendLogText(QString)));

	//Bearbeiten von im Ausgang befindlichen Jobs zulassen
	connect(this->outbox, SIGNAL(editJob(int)),
		this, SLOT(onEditJobFromOutbox(int)));

	connect(this->jobctrl, SIGNAL(jobQueueListChanged()),
		this, SLOT(onJobCtrlQueueListChanged()));
	connect(this->outbox, SIGNAL(moveJobInList(int,int)),
		this->jobctrl, SLOT(moveJob(int,int)));
	connect(this->outbox, SIGNAL(executeClicked()),
		this->jobctrl, SLOT(execQueuedTransactions()));
	connect(this->outbox, SIGNAL(removeJob(int)),
		this->jobctrl, SLOT(deleteJob(int)));

	widgetKnownDatedTransfers *datedTransfers = this->dock_KnownDatedTransfers->findChild<widgetKnownDatedTransfers*>();
	connect(datedTransfers, SIGNAL(updateDatedTransfers(const aqb_AccountInfo*)),
		this->jobctrl, SLOT(addGetDatedTransfers(const aqb_AccountInfo*)));

	widgetKnownStandingOrders *standingOrders = this->dock_KnownStandingOrders->findChild<widgetKnownStandingOrders*>();
	connect(standingOrders, SIGNAL(updateStandingOrders(const aqb_AccountInfo*)),
		this->jobctrl, SLOT(addGetStandingOrders(const aqb_AccountInfo*)));

}

/** \brief Lädt alle Account Daten */
//private
void MainWindow::loadAccountData()
{
	Q_ASSERT(this->accounts); //Accounts müssen vorhanden sein!
	AB_IMEXPORTER_CONTEXT *ctx;

	//Account Daten aus der entsprechenden Datei laden
	ctx = abt_parser::load_local_ctx(settings->getAccountDataFilename(),
					 "ctxfile", "default");
	abt_parser::parse_ctx(ctx, this->accounts);
	AB_ImExporterContext_free(ctx); //alle Daten geladen ctx wieder löschen.
}

/** \brief Speichert alle Account Daten */
//private
void MainWindow::saveAccountData()
{
	Q_ASSERT(this->accounts); //Accounts müssen vorhanden sein!
	AB_IMEXPORTER_CONTEXT *ctx = NULL;

	//erstellt einen AB_IMEXPORTER_CONTEXT für ALLE accounts
	ctx = abt_parser::create_ctx_from(this->accounts);

	//wenn kein ctx vorhanden ist müssen wir auch nichts speichern
	if (!ctx) return;

	abt_parser::save_local_ctx(ctx, settings->getAccountDataFilename(),
				   "ctxfile", "default");
	//ctx wieder freigeben!
	AB_ImExporterContext_free(ctx);
}

/** \brief Lädt alle History Daten */
//private
void MainWindow::loadHistoryData()
{
	Q_ASSERT(this->history); //Das History Object muss vorhanden sein
	Q_ASSERT(this->accounts); //Accounts müssen vorhanden sein!
	AB_IMEXPORTER_CONTEXT *ctx;

	//wir laden die History neu, deswegen erstmal alle Einträge löschen
	this->history->clearAll();

	//History-Daten aus der entsprechenden Datei laden
	ctx = abt_parser::load_local_ctx(settings->getHistoryFilename(),
					 "ctxfile", "default");
	abt_parser::parse_ctx(ctx, this->accounts, this->history);
	AB_ImExporterContext_free(ctx); //alle Daten geladen ctx wieder löschen.
}

/** \brief Speichert alle History Daten */
//private
void MainWindow::saveHistoryData()
{
	Q_ASSERT(this->history); //Das History Object muss vorhanden sein
	Q_ASSERT(this->accounts); //Accounts müssen vorhanden sein!
	AB_IMEXPORTER_CONTEXT *ctx = NULL;

	//Die History in einer Separaten Datei speichern
	ctx = this->history->getContext();
	abt_parser::save_local_ctx(ctx, settings->getHistoryFilename(),
				   "ctxfile", "default");
	//ctx wieder freigeben!
	AB_ImExporterContext_free(ctx);
}


void MainWindow::on_actionDebug_Info_triggered()
{
	if (debugDialog->isVisible()) {
		debugDialog->hide();
	} else {
		debugDialog->showNormal();
	}
}

/*!
 * Slot is called when a job is added to the jobctrl
 */
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

/*!
 * Item des Listwidget hat sich geändert, die entsprechende Seite des
 * stackedWidget anzeigen.
 */
void MainWindow::on_listWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
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
	about->setWindowTitle(tr("about %1").arg(qApp->applicationName()));

	//Lizenz-Text auf Anforderung (Click) anzeigen
	QDialog *licenseDialog = new QDialog(about);
	licenseDialog->setWindowTitle(tr("Lizenz"));
	QVBoxLayout *licenseLayout = new QVBoxLayout(licenseDialog);
	QLabel *licenseText = new QLabel(licenseDialog);
	licenseText->setText("Copyright (C) 2011-2012 Patrick Wacker<br /><br />"
			     "Dieses Programm ist freie Software. Sie können es unter den Bedingungen der<br />"
			     "GNU General Public License, wie von der Free Software Foundation veröffentlicht,<br />"
			     "weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder (nach<br />"
			     "Ihrer Option) jeder späteren Version.<br /><br />"
			     "Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, dass es Ihnen von<br />"
			     "Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, sogar ohne die implizite Garantie<br />"
			     "der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK. Details<br />"
			     "finden Sie in der GNU General Public License.<br /><br />"
			     "Sie sollten ein Exemplar der GNU General Public License zusammen mit diesem<br />"
			     "Programm erhalten haben. Falls nicht, schreiben Sie an die<br />"
			     "Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA.<br /><br /><br />"
			     "siehe <a href=\"http://www.gnu.de/documents/gpl-2.0.de.html\">http://www.gnu.de/documents/gpl-2.0.de.html</a>");
	licenseText->setOpenExternalLinks(true);
	licenseText->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	licenseLayout->addWidget(licenseText);
	QPushButton *licenseClose = new QPushButton(tr("Schließen"));
	connect(licenseClose, SIGNAL(clicked()), licenseDialog, SLOT(accept()));
	licenseLayout->addWidget(licenseClose, 0, Qt::AlignRight);

	//Horizontal Layout
	QHBoxLayout *hbox = new QHBoxLayout();
	//Icon-Image als Grafik oben links
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

	QLabel *viewvcURL = new QLabel(tr("viewvc: <a href=\"%1\">%1</a>").arg("http://schmufu.dyndns.org/viewvc/ab_transfers/"));
	viewvcURL->setOpenExternalLinks(true);
	viewvcURL->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	vbox->addWidget(viewvcURL, 0, Qt::AlignLeft);

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

	QLabel *iconsURL = new QLabel(QString("Icons used from: <a href=\"%1\">%1</a>").arg("http://www.oxygen-icons.org"));
	iconsURL->setOpenExternalLinks(true);
	iconsURL->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	vbox->addWidget(iconsURL, 0, Qt::AlignCenter);

	vbox->addSpacing(6);

	about->exec();

	delete about;
}

//private slot
void MainWindow::on_actionHelp_triggered()
{
	//hier eine Hilfe über die Vorgehensweise in AB-Transfers anzeigen
	//sowie oft gestellte Fragen beantworten.

	QDialog *helpDialog = new QDialog(this);
	helpDialog->setWindowTitle(tr("Hilfe / FAQ"));

	//Horizontal Layout
	QVBoxLayout *vbox = new QVBoxLayout(helpDialog);

	//ScrollArea zur Anzeige des Textes
	QScrollArea *scroll = new QScrollArea();
	scroll->setWidgetResizable(true);
	scroll->setMinimumSize(520, 600);
	scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	vbox->addWidget(scroll);

	// Der eigentliche HilfeText ist als ressource eingebunden und stammt
	// aus der Datei helpText.html im src Verzeichnis
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

	vbox->addSpacing(10);

	QPushButton *ok = new QPushButton(tr("OK"));
	connect(ok, SIGNAL(clicked()), helpDialog, SLOT(accept()));
	vbox->addWidget(ok, 0, Qt::AlignHCenter);

	vbox->addSpacing(6);

	helpDialog->exec();

	delete helpDialog;
}

//private SLOT
void MainWindow::on_actionEinstellungen_triggered()
{
	DialogSettings DiaSettings(settings, this);

	DiaSettings.exec();
}

//private SLOT
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
	if (acc->getSelectedAccount() != NULL) {
		//Menü nur anzeigen wenn auch ein Account ausgewählt ist
		this->accountContextMenu->exec(this->dock_Accounts->widget()->mapToGlobal(p));
	}
}

//private slot
void MainWindow::selectedStandingOrdersAccountChanged(const aqb_AccountInfo* acc)
{
	if (acc == NULL) return; //Abbruch wenn kein Account vorhanden
	settings->saveSelAccountInWidget("StandingOrders", acc);
}

//private slot
void MainWindow::selectedDatedTransfersAccountChanged(const aqb_AccountInfo* acc)
{
	if (acc == NULL) return; //Abbruch wenn kein Account vorhanden
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
	if (!acc) return; //Abbruch wenn kein BankAccountsWidget gefunden wurde
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
	if (!acc) return; //Abbruch wenn kein BankAccountsWidget gefunden wurde
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
	if (!acc) return; //Abbruch wenn kein BankAccountsWidget gefunden wurde
	this->jobctrl->addGetBalance(acc->getSelectedAccount());
}

//private slot
void MainWindow::onActionShowAvailableJobsTriggered()
{
	//Alle unterstützen bzw. nicht unterstützen Jobs für das gewählte
	//Konto anzeigen
	BankAccountsWidget *BAW = this->dock_Accounts->findChild<BankAccountsWidget*>();
	aqb_AccountInfo *acc = BAW->getSelectedAccount();
	if (!acc) return; //kein Account gewählt -> abbruch

	QDialog *dialog = new QDialog(this);
	dialog->setWindowTitle(tr("Unterstützte Aufträge"));
	QGridLayout *gl = new QGridLayout(dialog);

	//Text als Einleitung
	QLabel *text = new QLabel(tr("Anzeige der vom Institut unterstützten "
				     "Aufträge für das Konto %1 (%2). "
				     "Nicht alle Aufträge werden auch von "
				     "%3 unterstützt!").arg(
					acc->Number(), acc->Name(),
					qApp->applicationName()));
	text->setWordWrap(true);
	gl->addWidget(text, 0, 0, 1, -1); //use the text as a header
	gl->setRowMinimumHeight(1, 12); //one row as seperator

	//mögliche Icons erstellen
	QIcon *icoSup = new QIcon(QIcon::fromTheme("dialog-ok-apply"));
	QIcon *icoNotSup = new QIcon(QIcon::fromTheme("edit-delete"));
	const QPixmap pixSup = icoSup->pixmap(16, QIcon::Normal); //supported
	const QPixmap pixNotSup = icoNotSup->pixmap(16, QIcon::Normal); //not supported
	const QPixmap pixLatSup = icoSup->pixmap(16, QIcon::Disabled); //later supported

	//Spaltenüberschriften erstellen und anzeigen
	QLabel *text_bank = new QLabel(tr("Institut"));
	QLabel *text_abtransfer = new QLabel(tr("%1").arg(qApp->applicationName()));
	QLabel *text_description = new QLabel(tr("Auftrag"));
	gl->addWidget(text_bank, 2, 0, Qt::AlignCenter);
	gl->addWidget(text_abtransfer, 2, 1, Qt::AlignCenter);
	gl->addWidget(text_description, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
	gl->setRowMinimumHeight(3, 5); //kleine Unterteilung

	gl->setHorizontalSpacing(10); //Abstand zwischen den Feldern

	//Alle Aufträge von AqBanking durchgehen und entsprechend anzeigen
	QHashIterator<AB_JOB_TYPE, bool> it(*acc->availableJobsHash());
	it.toFront();
	int row = gl->rowCount(); //neue Widgets anfügen
	while (it.hasNext()) {
		it.next();
		QLabel *txt = new QLabel(abt_conv::JobTypeToQString(it.key()));
		QLabel *IcoBank = new QLabel();
		QLabel *IcoABT = new QLabel();

		//Icons für vom Institut unterstützte Aufträge
		if (it.value()) {
			IcoBank->setPixmap(pixSup);
		} else {
			IcoBank->setPixmap(pixNotSup);
		}

		//Icons für von AB-Transfers unterstützte Aufträge
		switch (abt_settings::supportedByAbtransfers(it.key())) {
		case 1: IcoABT->setPixmap(pixSup);;
			break;
		case 2: IcoABT->setPixmap(pixLatSup);
			break;
		default:
			IcoABT->setPixmap(pixNotSup);
			break;
		}

		//Icons und Text ausgeben
		gl->addWidget(IcoBank, row, 0, Qt::AlignCenter);
		gl->addWidget(IcoABT, row, 1, Qt::AlignCenter);
		gl->addWidget(txt, row, 2, Qt::AlignLeft | Qt::AlignVCenter);

		row++; //nächste Zeile
	}

	//Symbolbedeutungen erklären
	gl->setRowMinimumHeight(row++, 12); //Abstand zur Liste
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
	dialog->exec();

	delete dialog;
}

#ifdef TESTWIDGETACCESS
//private slot (ONLY FOR TESTING!)
void MainWindow::onActionTestWidgetAccessTriggered()
{
	//Nur zum Testen verwendet!
	QDialog *dialog = new QDialog(this);
	QVBoxLayout *vb = new QVBoxLayout(dialog);
	pageWidgetTests *page = new pageWidgetTests(this->accounts);
	vb->addWidget(page);

	dialog->exec();

	delete dialog;
}
#endif

//private slot
/** Tab soll gelöscht werden
 *
 * Prüft ob nicht gespeicherte Änderungen vorhanden sind und wenn ja erfolgt
 * eine abfrage ob der Tab wirklich geschlossen werden soll.
 * Wenn der Tab entfernt werden soll wird dies über deleteTabWidgetAndTab()
 * erledigt.
 */
void MainWindow::on_tabWidget_UW_tabCloseRequested(int index)
{
	widgetTransfer *transW = dynamic_cast<widgetTransfer*>(this->ui->tabWidget_UW->widget(index));
	if (transW == NULL) {
		//child widget nicht vorhanden!
		return; //nichts machen
	}

	if (transW->hasChanges()) {
		if (QMessageBox::question(this,
					  tr("Änderungen verwerfen?"),
					  tr("Im Tab '%1' wurden "
					     "Änderungenen vorgenommen!\n\n"
					     "Sollen diese Änderungen "
					     "verworfen werden?").arg(
							     this->ui->tabWidget_UW->tabText(index)),
					  QMessageBox::Yes | QMessageBox::No,
					  QMessageBox::Yes) != QMessageBox::Yes) {
			return; //Abbruch, Tab nicht schließen
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
	//Diese funktion sollte nur aufgerufen werden wenn das Tab welches
	//gelöscht werden soll auch gerade das übergebene Widget anzeigt.
	if (this->ui->tabWidget_UW->currentWidget() == w) {
		tabIdx = this->ui->tabWidget_UW->currentIndex();
	}

	if (tabIdx >= 0) {
		this->deleteTabWidgetAndTab(tabIdx);
	}
}

//private
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
		return NULL; //Abbruch, ohne Account können wir nichts machen!
	}

	widgetTransfer *trans = new widgetTransfer(type, acc, this->accounts, this);

	//Den neuen Tab auch gleich als aktuellen Tab setzen
	int tabid = this->ui->tabWidget_UW->addTab(trans, abt_conv::JobTypeToQString(type));
	this->ui->tabWidget_UW->setCurrentIndex(tabid);

	connect(trans, SIGNAL(createTransfer(AB_JOB_TYPE,const widgetTransfer*)),
		this, SLOT(onWidgetTransferCreateTransfer(AB_JOB_TYPE,const widgetTransfer*)));
	connect(trans, SIGNAL(cancelClicked(widgetTransfer*)),
		this, SLOT(onWidgetTransferCancelClicked(widgetTransfer*)));

	//Sicherstellen das die Übersichts-Seite angezeigt wird (damit der neue
	//Tab auch gleich sichtbar ist)
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

	return trans;
}

//private slot
void MainWindow::onWidgetTransferCancelClicked(widgetTransfer *sender)
{
	//sender im TabWidget suchen und entfernen
	int tabIdx = this->ui->tabWidget_UW->indexOf(sender);
	this->on_tabWidget_UW_tabCloseRequested(tabIdx);
}

//private slot
void MainWindow::onWidgetTransferCreateTransfer(AB_JOB_TYPE type, const widgetTransfer *sender)
{
	if (type == AB_Job_TypeCreateStandingOrder ||
	    type == AB_Job_TypeModifyStandingOrder) {
		//Testen ob die Eingaben von recurrence i.O. sind
		QString errMsg;
		bool inputOK = sender->isRecurrenceInputOk(errMsg);
		if (!inputOK) {
			bool accepted = this->correctRecurrenceDates(sender->recurrence);
			if (!accepted) {
				//User möchte die automatische Korrektur nicht
				return;
			}

		}

	}


	//erstmal prüfen ob die Eingaben in dem Widget so OK sind.
	QString errMsg;
	if (! sender->isGeneralInputOk(errMsg)) {
		QMessageBox::critical(this,
				      tr("Fehlerhafte Eingaben"),
				      tr("Folgende Eingaben sind fehlerhaft:<br />"
					 "%1<br />"
					 "Bitte korrigieren Sie diese.").arg(errMsg),
				      QMessageBox::Ok);
		return; //Abbruch, Eingaben sind fehlerhaft.
	}

	//entsprechende Transaction erstellen oder Ändern und dem abt_job_ctrl
	//zur Ausführung übergeben, danach das Widget und den Tab entfernen.
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

	//Der Transfer wurde in den Ausgangskorb übernommen, das Widget und den
	//Tab löschen
	this->deleteTabWidgetAndTab(sender);
}

/**
  * Wenn die Eingaben im widgetRecurrence fehlerhaft sind kann diese Funktion
  * aufgerufen werden um die Daten automatisch zu korrigieren.
  *
  * Vor einer Korrektur wird der Benutzer gefragt ob dies so gemacht werden soll.
  * Wenn er dem zustimmt wird true zurückgegeben und die Daten im
  * widgetRecurrence geändert. Ansonsten false und die Daten so belassen wie
  * sie sind.
  */
//private
bool MainWindow::correctRecurrenceDates(widgetRecurrence *recurrence) const
{
	QDate correctFirstDate, correctLastDate, correctNextDate;

	const QDate firstDate = recurrence->getFirstExecutionDate();
	const QDate lastDate = recurrence->getLastExecutionDate();
	const QDate nextDate = recurrence->getNextExecutionDate();

	//QDate DateEins(2012, 1, 31);
	//DateEins = DateEins.addMonths(2);
	//wo landen wir?
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

	//Wir gehen davon aus das der executionDay, die period und der cycle
	//richtig gewählt wurden und stellen dementsprechend das first-, last-
	//und nextDate ein.

	//Erstmal die gewählten Werte übernehmen (da const)
	correctFirstDate = firstDate;
	correctLastDate = lastDate;
	correctNextDate = nextDate;

	switch(period) {
	/** \todo Die einzelnen case-Abschnitte sollten der Übersichtshalber
		  in einzelne Funktionen ausgelagert werden.
	*/
	case AB_Transaction_PeriodWeekly: {
		//zum firstDate werden solange Tage addiert bis der gewählte
		//Wochentag dem Datum entspricht
		while (correctFirstDate.dayOfWeek() != execDay) {
			correctFirstDate = correctFirstDate.addDays(1);
		}

		//nextDate must be the same as firstDate
		correctNextDate = correctFirstDate;

		//Wenn lastDate ungültig ist soll der Dauerauftrag "bis auf
		//weiteres" durchgeführt werden (Kein Enddatum)
		if (!lastDate.isValid()) break; //keine weitere Bearbeitung

		//Ansonsten stellen wir das Enddatum auf den richtigen Wochentag
		//In diesem Fall der erste Wochentag der VOR dem eingestelltem
		//Datum mit dem gewählten Wochentag übereinstimmt

		int diffTage  = correctFirstDate.daysTo(correctLastDate);
		if ( diffTage < (cycle * 7) ) {
			// - firstDate und lastDate dürfen nicht gleich sein
			// - lastDate darf nicht vor firstDate liegen
			correctLastDate = correctFirstDate.addDays(cycle * 7);
		} else {
			// - Der Zyklus muss stimmen
			//Abweichung vom gewählten Wochentag (could be 0)
			int remTage = diffTage % 7;
			//Abweichung vom gewählten Zyklus (could be 0)
			int diffWochen = ( diffTage - remTage ) % cycle;
			//lastDate auf einen gültigen Tag, vor dem einstellten
			//Tag, setzen.
			correctLastDate = correctLastDate.addDays( -remTage -diffWochen*7 );
		}
	} //case AB_Transaction_PeriodWeekly:
		break;
	case AB_Transaction_PeriodMonthly: {
		//Für 'Erstmalig' setzen wir den erstmöglichen Tag der nach dem
		//eingestellten Datum liegt unter Berücksichtigung des
		//eingestellten Ausführungstags.
		//Für 'Letztmalig' setzen wir, wenn möglich, einen Tag der vor
		//oder gleich dem eingestellten Datum ist.

		switch(execDay) {
		case 99: /* Ultimo   */
		case 98: /* Ultimo-1 */
		case 97: /* Ultimo-2 */ {
			//Die Ausführung soll jeweils am letzten Tag[-1/-2]
			//des Monats erfolgen.

			int daysToEnd = 99 - execDay; //Tage vor dem letzten des Monats
			//bei Ultimo: 0 / Ultimo-1: 1 / Ultimo-2: 2

			if (firstDate.day() > (firstDate.daysInMonth() - daysToEnd)) {
				//erste Ausführung in den nächsten Monat schieben
				correctFirstDate = firstDate.addMonths(1);
			}
			//Den "richtigen" Tag setzen
			correctFirstDate.setDate(correctFirstDate.year(),
						 correctFirstDate.month(),
						 correctFirstDate.daysInMonth() - daysToEnd);

			// --> correctFirstDate ist jetzt richtig!


			//NextDate wird erstmal immer auf dasselbe Datum gesetzt
			//wie firstDate
			correctNextDate = correctFirstDate;


			//Wenn lastDate ungültig ist soll der Dauerauftrag "bis auf
			//weiteres" durchgeführt werden (Kein Enddatum)
			if (!lastDate.isValid()) break; //keine weitere Bearbeitung

			//Ansonsten stellen wir das Enddatum auf den richtigen Tag.
			//Wir wählen ein Datum das möglichst vor dem eingestellten
			//lastDate liegt und berücksichtigen den Ausführungstag
			//sowie den Zyklus!

			int monthDiff = ( ( lastDate.year() - correctFirstDate.year() ) * 12 +
					  ( lastDate.month() - correctFirstDate.month() ) ) % cycle;

			//monthDiff entspricht jetzt den Monaten die das lastDate
			//vorgezogen werden muss (kann auch 0 sein).
			correctLastDate = lastDate.addMonths( -monthDiff );

			//im lastDate den "richtigen" Ausführungstag setzen
			correctLastDate.setDate(correctLastDate.year(),
						correctLastDate.month(),
						correctLastDate.daysInMonth() - daysToEnd);

			// - firstDate und lastDate dürfen keinesfalls gleich sein
			// - lastDate darf nicht vor firstDate liegen
			if (correctFirstDate >= correctLastDate) {
				//in den nächsten Zyklus wechseln
				/** \todo Kann es auch sein das wir 2 Zyklen
					  weiter müssen? Nochmal durchrechnen!
				*/
				correctLastDate = correctFirstDate.addMonths(cycle);
				//correctLastDate wurde geändert, deswegen setzen
				//wir nochmal den "richtigen" Ausführungstag
				correctLastDate.setDate(correctLastDate.year(),
							correctLastDate.month(),
							correctLastDate.daysInMonth() - daysToEnd);
			}

			// --> correctLastDate ist jetzt richtig!
		}
			break;
		default: //"Normaler" Tag
			//"Erstmalig" richtig einstellen

			//worst case:   31        30
			if ( firstDate.day() > execDay ) {
				//erste Ausführung in den nächsten Monat verschieben
				correctFirstDate = firstDate.addMonths(1);
				//worst case: date.day() ist jetzt 30 (oder 28) !
			}

			if ( correctFirstDate.daysInMonth() < execDay ) {
				//In diesem Fall wird der letzte Tag ausgewählt.
				//kommt nur sehr selten vor! Und sollte richtig sein.
				correctFirstDate.setDate(correctFirstDate.year(),
							 correctFirstDate.month(),
							 correctFirstDate.daysInMonth());
			} else {
				correctFirstDate = correctFirstDate.addDays( execDay - correctFirstDate.day() );
			}

			// --> correctFirstDate ist jetzt richtig!


			//NextDate wird erstmal immer auf dasselbe Datum gesetzt
			//wie firstDate
			correctNextDate = correctFirstDate;


			//Wenn lastDate ungültig ist soll der Dauerauftrag "bis auf
			//weiteres" durchgeführt werden (Kein Enddatum)
			if (!lastDate.isValid()) break; //keine weitere Bearbeitung

			//Ansonsten stellen wir das Enddatum auf den richtigen Tag.
			//Wir wählen ein Datum das möglichst vor dem eingestellten
			//lastDate liegt und berücksichtigen den Ausführungstag
			//sowie den Zyklus!

			int monthDiff = ( ( lastDate.year() - correctFirstDate.year() ) * 12 +
					  ( lastDate.month() - correctFirstDate.month() ) ) % cycle;

			//monthDiff entspricht jetzt den Monaten die das lastDate
			//vorgezogen werden muss um zum gewählten Zyklus zu passen.
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

			//im lastDate den "richtigen" Ausführungstag setzen
			if ( correctLastDate.daysInMonth() < execDay ) {
				//In diesem Fall wird der letzte Tag ausgewählt.
				//kommt nur sehr selten vor! Und sollte richtig sein.
				correctLastDate.setDate(correctLastDate.year(),
							correctLastDate.month(),
							correctLastDate.daysInMonth());
			} else {
				correctLastDate.setDate(correctLastDate.year(),
							correctLastDate.month(),
							execDay);
			}

			// - firstDate und lastDate dürfen keinesfalls gleich sein
			// - lastDate darf nicht vor firstDate liegen
			if (correctFirstDate >= correctLastDate) {
				correctLastDate = correctFirstDate.addMonths(cycle);
				//lastDate wurde geändert, deswegen setzten wir
				//nochmal den "richtigen" Ausführungstag
				if ( correctLastDate.daysInMonth() < execDay ) {
					//In diesem Fall wird der letzte Tag ausgewählt.
					//kommt nur sehr selten vor! Und sollte richtig sein.
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
		//sollte nicht vorkommen, wir kennen nur weekly und monthly
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


/** \brief Prüft und gibt eine Warnung aus wenn der Dauerauftrag bereits im
  *        Ausgang ist.
  *
  * \returns true wenn der Dauerauftrag bereits im Ausgang ist
  * \returns false wenn der Dauerauftrag nicht im Ausgang ist
  */
//private
bool MainWindow::isStandingOrderInOutbox(const abt_standingOrderInfo *soi)
{
	if (this->jobctrl->isTransactionInQueue(soi->getTransaction())) {
		QMessageBox::critical(this, tr("Bereits im Ausgang"),
			tr("<b>Der Dauerauftrag befindet sich bereits im Ausgang!</b><br /><br />"
			   "Er wurde entweder schon bearbeitet oder soll gelöscht werden. "
			   "Solange sich zu diesem Dauerauftrag bereits eine Änderung "
			   "im Ausgang befindet kann keine weitere Änderung stattfinden.<br />"
			   "Bitte löschen oder Bearbeiten Sie den entsprechenden "
			   "Auftrag im Ausgang."),
			QMessageBox::Ok, QMessageBox::Ok);
		return true;
	}
	return false;
}

/** \brief Prüft ob die gespeicherten Daten veraltet sind und fragt wie fortgefahren
  *        werden soll.
  *
  * Es wird geprüft ob das Datum der ersten Ausführung in der Zukunft liegt.
  * Wenn dies nicht der Fall ist wird davon ausgegangen das die Daten veraltet
  * sind und der Benutzer gefragt ob eine Aktualisierung stattfinden soll.
  *
  * Es existiert auch die Möglichkeit das der Benutzer diese Warnung ignorieren
  * kann, in diesem Fall gibt diese Funktion denselben Rückgabewert zurück als
  * wäre das Datum i.O.
  *
  * Wenn eine Aktualisierung durchgeführt werden soll wird automatisch der
  * entsprechende Auftrag in den Ausgang gestellt und in den Ausgang gewechselt.
  *
  * \returns false
  *	Wenn der Dauerauftrag bearbeitet werden kann (Daten nicht verhaltet) oder
  *	bearbeitet werden soll (Benutzer will keine Aktualisierung)
  *
  * \returns true
  *	wenn der Dauerauftrag nicht bearbeitet werden soll.
  *
  */
//private
bool MainWindow::isStandingOrderOutdated(const aqb_AccountInfo *acc,
					 const abt_standingOrderInfo *soi)
{
	if (QDate::currentDate() >= soi->getTransaction()->getFirstExecutionDate()) {
		//Der gespeicherte Dauerauftrag, ist veraltet!
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
		case QMessageBox::Ignore: break; //User will trotzdem editieren
		case QMessageBox::Yes:
			//Auftrag in den Ausgang einstellen
			this->jobctrl->addGetStandingOrders(acc);
			//Den Ausgang aktivieren
			this->ui->listWidget->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
			return true; //Edit soll abgebrochen werden
			break;
		default: //wie Abort behandeln
			return true; //Abbrechen
			break;
		}
	}
	return false;
}

//private Slot
void MainWindow::onStandingOrderEditRequest(const aqb_AccountInfo *acc, const abt_standingOrderInfo *da)
{
	//Abbrechen wenn bereits im Ausgang
	if (this->isStandingOrderInOutbox(da)) return;

	//Abbrechen wenn Daten veraltet
	if (this->isStandingOrderOutdated(acc, da)) return;

	widgetTransfer *transW;
	transW = this->createTransferWidgetAndAddTab(AB_Job_TypeModifyStandingOrder,
						     acc);
	transW->setValuesFromTransaction(da->getTransaction());
}

//private Slot
void MainWindow::onStandingOrderDeleteRequest(const aqb_AccountInfo *acc, const abt_standingOrderInfo *da)
{
	//Abbruch wenn bereits im Ausgang
	if (this->isStandingOrderInOutbox(da)) return;

	//Abbrechen wenn Daten veraltet
	if (this->isStandingOrderOutdated(acc, da)) return;

	this->jobctrl->addDeleteStandingOrder(acc, da->getTransaction());
}


/** \brief Prüft und gibt eine Warnung aus wenn die terminierte Überweisung
  *	   bereits im Ausgang ist.
  *
  * \returns true wenn die terminierte Überweisung bereits im Ausgang ist
  * \returns false wenn die terminierte Überweisung nicht im Ausgang ist
  */
//private
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

/** \brief Prüft ob die gespeicherten Daten veraltet sind und fragt wie fortgefahren
  *        werden soll.
  *
  * Es wird geprüft ob das Datum der terminierten Überweisung in der Zukunft liegt.
  * Wenn dies nicht der Fall ist wird davon ausgegangen das die Daten veraltet
  * sind und der Benutzer gefragt ob eine Aktualisierung stattfinden soll.
  *
  * Es existiert auch die Möglichkeit das der Benutzer diese Warnung ignorieren
  * kann, in diesem Fall gibt diese Funktion denselben Rückgabewert zurück als
  * wäre das Datum i.O.
  *
  * Wenn eine Aktualisierung durchgeführt werden soll wird automatisch der
  * entsprechende Auftrag in den Ausgang gestellt und in den Ausgang gewechselt.
  *
  * \returns false
  *	Wenn der Dauerauftrag bearbeitet werden kann (Daten nicht verhaltet) oder
  *	bearbeitet werden soll (Benutzer will keine Aktualisierung)
  *
  * \returns true
  *	wenn der Dauerauftrag nicht bearbeitet werden soll.
  *
  */
//private
bool MainWindow::isDatedTransferOutdated(const aqb_AccountInfo *acc,
					 const abt_datedTransferInfo *dti)
{
	if (QDate::currentDate() >= dti->getTransaction()->getDate()) {
		//Die gespeicherte Terminüberweisung ist veraltet!
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
		case QMessageBox::Ignore: break; //User will trotzdem editieren
		case QMessageBox::Yes:
			//Auftrag in den Ausgang einstellen
			this->jobctrl->addGetDatedTransfers(acc);
			//Den Ausgang aktivieren
			this->ui->listWidget->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
			return true; //Edit soll abgebrochen werden
			break;
		default: //wie Abort behandeln
			return true; //Abbrechen
			break;
		}
	}
	return false;
}

//private Slot
void MainWindow::onDatedTransferEditRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo *di)
{
	//Abbrechen wenn bereits im Ausgang
	if (this->isDatedTransferInOutbox(di)) return;

	//Abbrechen wenn Daten veraltet
	if (this->isDatedTransferOutdated(acc, di)) return;

	widgetTransfer *transW;
	transW = this->createTransferWidgetAndAddTab(AB_Job_TypeModifyDatedTransfer,
						     acc);
	transW->setValuesFromTransaction(di->getTransaction());
}

//private Slot
void MainWindow::onDatedTransferDeleteRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo *di)
{
	//Abbrechen wenn bereits im Ausgang
	if (this->isDatedTransferInOutbox(di)) return;

	//Abbrechen wenn Daten veraltet
	if (this->isDatedTransferOutdated(acc, di)) return;

	this->jobctrl->addDeleteDatedTransfer(acc, di->getTransaction());
}

//private Slot
void MainWindow::onEditJobFromOutbox(int itemNr)
{
	/** \todo Der erstellte widgetTransfer enthält bereits Änderungen,
		  dies sollte in diesem auch gesetzt werden, damit bei Klick
		  auf Abbruch eine Warnung erscheint!
	*/

	widgetTransfer *transW;
	const aqb_AccountInfo *acc = NULL;

	//Die itemNr enthält die Position in der JobQueueList
	const abt_jobInfo *job = this->jobctrl->jobqueueList()->at(itemNr);

	QString jobAccBankcode, jobAccNumber;
	jobAccBankcode = AB_Account_GetBankCode(AB_Job_GetAccount(job->getJob()));
	jobAccNumber =AB_Account_GetAccountNumber(AB_Job_GetAccount(job->getJob()));

	//Wir suchen den Account der zu dem bereits erstellten Job gehört
	QHashIterator<int, aqb_AccountInfo*> i(this->accounts->getAccountHash());
	//Alle Accounts durchgehen
	i.toFront();
	while (i.hasNext()) {
		i.next();
		if ((i.value()->BankCode() == jobAccBankcode) &&
		    (i.value()->Number() == jobAccNumber)) {
			//we found the account
			acc = i.value();
			break; //leave while, we have the account
		}
	}

	if (acc == NULL) {
		//account not found
		qWarning() << Q_FUNC_INFO << "account from the job not found, aborting";
		return;
	}

	transW = this->createTransferWidgetAndAddTab(job->getAbJobType(), acc);

	transW->setValuesFromTransaction(job->getTransaction());

	//den Job aus der JobQueueList entfernen
	this->jobctrl->deleteJob(itemNr, true);
}

//private slot
void MainWindow::onJobCtrlQueueListChanged()
{
	this->outbox->refreshTreeWidget(this->jobctrl);
}

//private slot
void MainWindow::onActionSaveAllDataTriggered()
{
	this->saveAccountData();
	this->saveHistoryData();
}

//private
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
void MainWindow::createAndSendTransfer(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

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
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
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
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
void MainWindow::createAndSendDatedTransfer(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

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
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
void MainWindow::createAndSendStandingOrder(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

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
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
void MainWindow::createAndSendSepaTransfer(const widgetTransfer* /* not used yet: sender */)
{
	qWarning() << "create SEPA Transfer not implemented yet!";
	this->statusBar()->showMessage("create SEPA Transfer not implemented yet!");
	return;

//	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
//	abt_transaction *t = new abt_transaction();
//
//	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());
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
//	this->jobctrl->addNewSepaTransfer(acc, t);
//
//	delete t;
}

//private
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
void MainWindow::createAndSendModifyDatedTransfer(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	const abt_transaction *origT = sender->getOriginalTransaction();

	//kopie der original Transaction erstellen
	abt_transaction *newT = new abt_transaction(*origT);

	//und diese modifizieren
	newT->fillLocalFromAccount(acc->get_AB_ACCOUNT());

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
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
void MainWindow::createAndSendModifyStandingOrder(const widgetTransfer *sender)
{
	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	const abt_transaction *origT = sender->getOriginalTransaction();

	//kopie der original Transaction erstellen
	abt_transaction *newT = new abt_transaction(*origT);

	//und diese modifizieren
	newT->fillLocalFromAccount(acc->get_AB_ACCOUNT());

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
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
void MainWindow::createAndSendDebitNote(const widgetTransfer *sender)
{
	qWarning() << "create Debit Note not implemented yet!";
	this->statusBar()->showMessage("create Debit Note not implemented yet!");
	return;

	const aqb_AccountInfo *acc = sender->localAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

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
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
void MainWindow::createAndSendInternalTransfer(const widgetTransfer *sender)
{
	//Umbuchung zwichen 2 Konten bei derselben Bank
	const aqb_AccountInfo *fromAcc = sender->localAccount->getAccount();
	const aqb_AccountInfo *toAcc = sender->remoteAccount->getAccount();
	abt_transaction *t = new abt_transaction();

	t->fillLocalFromAccount(fromAcc->get_AB_ACCOUNT());

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
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
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

void MainWindow::on_actionAqBankingSetup_triggered()
{
	GWEN_DIALOG *dlg;
	int rv;

	/* Der Setup-Dialog darf nur ausgeführt werden wenn keine Jobs
	 * im Ausgang vorhanden sind, bzw. keine Daten der Accounts
	 * in anderen Objekten verwendet werden die nach dem ausführen
	 * des Setup-Dialogs evt. nicht mehr gültig sind und somit nicht
	 * mehr verwendet werden dürfen! (z.B. Pointer auf aqb_accountInfo
	 * Objecte!)
	 */

	int outboxCnt = this->jobctrl->jobqueueList()->size();
	int editCnt = this->ui->tabWidget_UW->count() - 1; //Übersicht ist immer vorhanden

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
		return; //Abbruch
	}


	dlg = AB_SetupDialog_new(banking->getAqBanking());
	if (!dlg) {
		qWarning() << Q_FUNC_INFO << "could not create AqBanking setup dialog";
		return;
	}

	/* Hier müssen alle accounts gelöscht werden, damit sie, nach dem
	 * AqBanking-Setup Dialog, wieder neu erstellt werden können.
	 * Dabei werden dann einfach alle Daten neu geladen.
	 */

	//Alle Verwendungen von accounts auf NULL setzen und somit in den
	//einzelnen Widgets auf 'ungültig' setzen.
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

	this->saveAccountData(); //Alle Account-Daten sichern

	//Alle Accounts löschen
	delete this->jobctrl; //löscht auch alle Connections
	delete this->accounts;

	//AqBanking Setup Dialog ausführen
	rv = GWEN_Gui_ExecDialog(dlg, 0);
	if (rv == 0) {
		qDebug() << Q_FUNC_INFO << "AqBanking setup dialog aborted by user";
	}

	/* Der AqBanking Setup Dialog wurde beendet, jetzt müssen alle Account
	 * Daten neu geladen werden.
	 * Und auch alle Connections neu aufgebaut werden.
	 */

	//Es könnten sich Accounts geändert haben, deswegen alle neu erstellen
	this->accounts = new aqb_Accounts(banking->getAqBanking());

	//den JobController und dessen Connections wieder erstellen
	this->createJobCtrlAndConnections();

	//Alle Account-Daten laden
	this->loadAccountData();

	//Die Accounts in den Widgets wieder setzen
	this->dockDatedTransfersSetAccounts();
	this->dockStandingOrdersSetAccounts();
	baw->setAccounts(this->accounts); //baw wurde oben zugewiesen!

	//Die Daten in der ScrollArea neu aufbauen
	this->createWidgetsInScrollArea();

	GWEN_Dialog_free(dlg);
}
