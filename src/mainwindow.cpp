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
#include <gwenhywfar4/gwen-gui-qt4/qt4_gui.hpp>
#include <aqbanking/jobgettransactions.h>
#include <aqbanking/value.h>

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
	this->history = new abt_history(this->accounts, this);
	this->jobctrl = new abt_job_ctrl(this->accounts, this->history, this);
	this->logw = new page_log();
	this->outw = new Page_Ausgang(this->jobctrl);
	this->dock_KnownRecipient = NULL;
	this->dock_KnownStandingOrders = NULL;
	this->dock_KnownDatedTransfers = NULL;

	//Alle Accounts von AqBanking wurden erstellt (this->accounts), jetzt
	//können die Daten mit dem parser geladen werden
	AB_IMEXPORTER_CONTEXT *ctx = abt_parser::load_local_ctx(
						settings->getAccountDataFilename(),
						"ctxfile", "default");
	abt_parser::parse_ctx(ctx, this->accounts);
	//alle Daten geladen ctx wieder löschen.
	AB_ImExporterContext_free(ctx);

	//Auch die History-Daten müssen geladen werden
	ctx = abt_parser::load_local_ctx(settings->getHistoryFilename(),
					 "ctxfile", "default");
	abt_parser::parse_ctx(ctx, this->accounts, this->history);
	//alle Daten geladen ctx wieder löschen.
	AB_ImExporterContext_free(ctx);


	QVBoxLayout *logLayout = new QVBoxLayout(ui->Log);
	logLayout->setMargin(0);
	logLayout->setSpacing(2);
	ui->Log->setLayout(logLayout);
	ui->Log->layout()->addWidget(this->logw);

	QVBoxLayout *outLayout = new QVBoxLayout(ui->Ausgang);
	outLayout->setMargin(0);
	outLayout->setSpacing(2);
	ui->Ausgang->setLayout(outLayout);
	ui->Ausgang->layout()->addWidget(this->outw);

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
	connect(this->outw, SIGNAL(edit_Job(const abt_jobInfo*)),
		this, SLOT(onEditJobFromOutbox(const abt_jobInfo*)));

	//Default-Entry Überweisung auswählen
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

	//default DockOptions setzen
	this->setDockOptions(QMainWindow::AllowNestedDocks |
			     QMainWindow::AllowTabbedDocks);// |
			     //QMainWindow::AnimatedDocks);
	this->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

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

	this->createDockStandingOrders();
	this->createDockDatedTransfers();

	this->createActions();
	this->createMenus();
	this->createDockToolbar();

	this->createWidgetsInScrollArea();

	//this->ui->tabWidget_UW->setTabText(0, tr("Überischt"));
	connect(this->ui->pushButton, SIGNAL(clicked()),
		this->actTransferNational, SLOT(trigger()));
	connect(this->ui->pushButton_2, SIGNAL(clicked()),
		this->actTransferInternational, SLOT(trigger()));
	connect(this->ui->pushButton_3, SIGNAL(clicked()),
		this->actTransferSepa, SLOT(trigger()));
	connect(this->ui->pushButton_4, SIGNAL(clicked()),
		this->actTransferInternal, SLOT(trigger()));

	connect(this->ui->pushButton_so_new, SIGNAL(clicked()),
		this->actStandingNew, SLOT(trigger()));
	connect(this->ui->pushButton_so_update, SIGNAL(clicked()),
		this->actStandingUpdate, SLOT(trigger()));

	connect(this->ui->pushButton_dated_new, SIGNAL(clicked()),
		this->actDatedNew, SLOT(trigger()));
	connect(this->ui->pushButton_dated_update, SIGNAL(clicked()),
		this->actDatedUpdate, SLOT(trigger()));

	

	//QGroupBox *grpKSO = new QGroupBox(this->ui->MainTab);
	//widgetKnownStandingOrders *kso = new widgetKnownStandingOrders(this->accounts->getAccountHash().value(5, NULL), grpKSO);


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

	delete this->outw;	//AusgangsWidget löschen
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

	//Bevor wir geschlossen werden noch alle Daten sichern!
	AB_IMEXPORTER_CONTEXT *ctx = NULL;
	//erstellt einen AB_IMEXPORTER_CONTEXT für ALLE accounts
	ctx = abt_parser::create_ctx_from(this->accounts);
	abt_parser::save_local_ctx(ctx, settings->getAccountDataFilename(),
				   "ctxfile", "default");
	//ctx wieder freigeben!
	AB_ImExporterContext_free(ctx);

	//Die History in einer Separaten Datei speichern
	ctx = this->history->getContext();
	abt_parser::save_local_ctx(ctx, settings->getHistoryFilename(),
				   "ctxfile", "default");
	//ctx wieder freigeben!
	AB_ImExporterContext_free(ctx);

	//jetzt können wir geschlossen werden
	e->accept();
}

//private Slot
void MainWindow::TimerTimeOut()
{
	//Der Timer läuft erst ab wenn die execLoop gestartet ist. Somit wird
	//dieser Code erst ausgeführt wenn das MainWindow angezeigt und die
	//EventLoop der Anwendung läuft.

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

	//Der Timer wird nicht länger benötigt
	delete this->timer;
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

	actDatedEdit = new QAction(this);
	actDatedEdit->setText(tr("Bearbeiten"));
	actDatedEdit->setIcon(QIcon(":/icons/bank-icon"));
	connect(actDatedEdit, SIGNAL(triggered()), this, SLOT(onActionDatedEditTriggered()));

	actDatedDel = new QAction(this);
	actDatedDel->setText(tr("Löschen"));
	actDatedDel->setIcon(QIcon(":/icons/bank-icon"));
	connect(actDatedDel, SIGNAL(triggered()), this, SLOT(onActionDatedDelTriggered()));

	actDatedUpdate = new QAction(this);
	actDatedUpdate->setText(tr("Aktualisieren"));
	actDatedUpdate->setIcon(QIcon(":/icons/bank-icon"));
	connect(actDatedUpdate, SIGNAL(triggered()), this, SLOT(onActionDatedUpdateTriggered()));

	actStandingNew = new QAction(this);
	actStandingNew->setText(tr("Anlegen"));
	actStandingNew->setIcon(QIcon(":/icons/bank-icon"));
	connect(actStandingNew, SIGNAL(triggered()), this, SLOT(onActionStandingNewTriggered()));

	actStandingEdit = new QAction(this);
	actStandingEdit->setText(tr("Bearbeiten"));
	actStandingEdit->setIcon(QIcon(":/icons/bank-icon"));
	connect(actStandingEdit, SIGNAL(triggered()), this, SLOT(onActionStandingEditTriggered()));

	actStandingDel = new QAction(this);
	actStandingDel->setText(tr("Löschen"));
	actStandingDel->setIcon(QIcon(":/icons/bank-icon"));
	connect(actStandingDel, SIGNAL(triggered()), this, SLOT(onActionStandingDelTriggered()));

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

#ifdef TESTWIDGETACCESS
	actTestWidgetAccess = new QAction(tr("TestWidget"), this);
	connect(actTestWidgetAccess, SIGNAL(triggered()), this, SLOT(onActionTestWidgetAccessTriggered()));
#endif


}


//private
void MainWindow::createMenus()
{
	this->accountContextMenu = new QMenu(this);
	QMenu *MenuTransfer = new QMenu("Überweisung", this);
	MenuTransfer->addAction(this->actTransferNational);
	MenuTransfer->addAction(this->actTransferInternational);
	MenuTransfer->addAction(this->actTransferInternal);
	MenuTransfer->addAction(this->actTransferSepa);
	QMenu *MenuStanding = new QMenu("Daueraufträge", this);
	MenuStanding->addAction(this->actStandingNew);
	//MenuStanding->addAction(this->actStandingEdit);
	MenuStanding->addAction(this->actStandingUpdate);
	QMenu *MenuDated = new QMenu("Terminüberweisungen", this);
	MenuDated->addAction(this->actDatedNew);
	//MenuDated->addAction(this->actDatedEdit);
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
	QVBoxLayout *layoutScrollArea = new QVBoxLayout(this->ui->scrollAreaWidgetContents);

	foreach(const aqb_AccountInfo *acc, this->accounts->getAccountHash().values()) {
		//Bekannte Daueraufträge
		QGroupBox *grpSO = new QGroupBox(this);
		QVBoxLayout *lSO = new QVBoxLayout(grpSO);
		grpSO->setTitle(tr("Daueraufträge von \"%1\" (%2 - %3)").arg(acc->Name(), acc->Number(), acc->BankCode()));
		widgetKnownStandingOrders *StandingOrders = new widgetKnownStandingOrders(acc, this);

		connect(StandingOrders, SIGNAL(updateStandingOrders(const aqb_AccountInfo*)),
			this->jobctrl, SLOT(addGetStandingOrders(const aqb_AccountInfo*)));

		connect(StandingOrders, SIGNAL(editStandingOrder(const aqb_AccountInfo*,const abt_standingOrderInfo*)),
			this, SLOT(onStandingOrderEditRequest(const aqb_AccountInfo*,const abt_standingOrderInfo*)));
		connect(StandingOrders, SIGNAL(deleteStandingOrder(const aqb_AccountInfo*,const abt_standingOrderInfo*)),
			this, SLOT(onStandingOrderDeleteRequest(const aqb_AccountInfo*,const abt_standingOrderInfo*)));

		lSO->addWidget(StandingOrders);
		layoutScrollArea->addWidget(grpSO);

		//Bekannte Terminüberweisungen
		QGroupBox *grpDT = new QGroupBox(this);
		QVBoxLayout *lDT = new QVBoxLayout(grpDT);
		grpDT->setTitle(tr("Terminierte Überweisungen von \"%1\" (%2 - %3)").arg(acc->Name(), acc->Number(), acc->BankCode()));
		widgetKnownDatedTransfers *DatedTransfers = new widgetKnownDatedTransfers(acc, this);

		connect(DatedTransfers, SIGNAL(updateDatedTransfers(const aqb_AccountInfo*)),
			this->jobctrl, SLOT(addGetDatedTransfers(const aqb_AccountInfo*)));

		connect(DatedTransfers, SIGNAL(editDatedTransfer(const aqb_AccountInfo*, const abt_datedTransferInfo*)),
			this, SLOT(onDatedTransferEditRequest(const aqb_AccountInfo*, const abt_datedTransferInfo*)));
		connect(DatedTransfers, SIGNAL(deleteDatedTransfer(const aqb_AccountInfo*, const abt_datedTransferInfo*)),
			this, SLOT(onDatedTransferDeleteRequest(const aqb_AccountInfo*, const abt_datedTransferInfo*)));

		lDT->addWidget(DatedTransfers);
		layoutScrollArea->addWidget(grpDT);
	}
}

//private
void MainWindow::createDockStandingOrders()
{
	QDockWidget *dock = new QDockWidget("Daueraufträge", this);
	dock->setObjectName("dockStandingOrders");

	QVBoxLayout *layoutDock = new QVBoxLayout();
	QHBoxLayout *layoutAcc = new QHBoxLayout();
	QLabel *accText = new QLabel(tr("Konto"));

	//den zuletzt gewählten Account wieder anzeigen
	int SelAccID = settings->loadSelAccountInWidget("StandingOrders");
	const aqb_AccountInfo *lastAcc = this->accounts->getAccount(SelAccID);

	widgetAccountComboBox *accComboBox = new widgetAccountComboBox(lastAcc,
								       this->accounts);
	widgetKnownStandingOrders *StandingOrders;
	//Das DockWidget muss mit dem in der comboBox gewählten Account erstellt
	//werden, da wenn die ComboBox mit "NULL" erstellt wurde trotzdem das
	//erste Konto gewählt ist! (somit Erstellung des Dock mit Account != NULL)
	StandingOrders = new widgetKnownStandingOrders(accComboBox->getAccount());

	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		StandingOrders, SLOT(setAccount(const aqb_AccountInfo*)));
	//damit Änderungen der Auswahl auch in der settings.ini gespeichert werden
	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		this, SLOT(selectedStandingOrdersAccountChanged(const aqb_AccountInfo*)));

	connect(StandingOrders, SIGNAL(updateStandingOrders(const aqb_AccountInfo*)),
		this->jobctrl, SLOT(addGetStandingOrders(const aqb_AccountInfo*)));

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
}

//private
void MainWindow::createDockDatedTransfers()
{
	QDockWidget *dock = new QDockWidget("Terminüberweisungen", this);
	dock->setObjectName("dockDatedTransfers");

	QVBoxLayout *layoutDock = new QVBoxLayout();
	QHBoxLayout *layoutAcc = new QHBoxLayout();
	QLabel *accText = new QLabel(tr("Konto"));

	//den zuletzt gewählten Account wieder anzeigen
	int SelAccID = settings->loadSelAccountInWidget("DatedTransfers");
	const aqb_AccountInfo *lastAcc = this->accounts->getAccount(SelAccID);

	widgetAccountComboBox *accComboBox = new widgetAccountComboBox(lastAcc,
								       this->accounts);
	widgetKnownDatedTransfers *DatedTransfers;
	//Das DockWidget muss mit dem in der comboBox gewählten Account erstellt
	//werden, da wenn die ComboBox mit "NULL" erstellt wurde trotzdem das
	//erste Konto gewählt ist! (somit Erstellung des Dock mit Account != NULL)
	DatedTransfers = new widgetKnownDatedTransfers(accComboBox->getAccount());

	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		DatedTransfers, SLOT(setAccount(const aqb_AccountInfo*)));

	//Änderungen der Account-Wahl in der settings.ini speichern
	connect(accComboBox, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		this, SLOT(selectedDatedTransfersAccountChanged(const aqb_AccountInfo*)));

	connect(DatedTransfers, SIGNAL(updateDatedTransfers(const aqb_AccountInfo*)),
		this->jobctrl, SLOT(addGetDatedTransfers(const aqb_AccountInfo*)));

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
	ui->statusBar->showMessage(msg);
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
	settings->saveSelAccountInWidget("StandingOrders", acc);
}

//private slot
void MainWindow::selectedDatedTransfersAccountChanged(const aqb_AccountInfo* acc)
{
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
void MainWindow::onActionDatedEditTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeModifyDatedTransfer);
	//Sicherstellen das "Übersicht" im listWidget ausgewählt ist.
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
}

//private slot
void MainWindow::onActionDatedDelTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeDeleteDatedTransfer);
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
void MainWindow::onActionStandingEditTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeModifyStandingOrder);
	//Sicherstellen das "Übersicht" im listWidget ausgewählt ist.
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
}

//private slot
void MainWindow::onActionStandingDelTriggered()
{
	this->createTransferWidgetAndAddTab(AB_Job_TypeDeleteStandingOrder);
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

	widgetTransfer *trans = new widgetTransfer(type, acc, this->accounts, this);

	int tabid = this->ui->tabWidget_UW->addTab(trans, abt_conv::JobTypeToQString(type));
	this->ui->tabWidget_UW->setCurrentIndex(tabid);

	connect(trans, SIGNAL(createTransfer(AB_JOB_TYPE,const widgetTransfer*)),
		this, SLOT(onWidgetTransferCreateTransfer(AB_JOB_TYPE,const widgetTransfer*)));
	connect(trans, SIGNAL(cancelClicked(widgetTransfer*)),
		this, SLOT(onWidgetTransferCancelClicked(widgetTransfer*)));
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





//private Slot
void MainWindow::onStandingOrderEditRequest(const aqb_AccountInfo *acc, const abt_standingOrderInfo *da)
{
	if (this->jobctrl->isTransactionInQueue(da->getTransaction())) {
		QMessageBox::critical(this, tr("Bereits im Ausgang"),
			tr("<b>Der Dauerauftrag befindet sich bereits im Ausgang!</b><br /><br />"
			   "Er wurde entweder schon bearbeitet oder soll gelöscht werden. "
			   "Solange sich zu diesem Dauerauftrag bereits eine Änderung "
			   "im Ausgang befindet kann keine weitere Änderung stattfinden.<br />"
			   "Bitte löschen Sie zuerst den entsprechenden Auftrag im Ausgang."),
			QMessageBox::Ok, QMessageBox::Ok);
		return; //Abbruch
	}

	widgetTransfer *transW;
	transW = this->createTransferWidgetAndAddTab(AB_Job_TypeModifyStandingOrder,
						     acc);
	transW->setValuesFromTransaction(da->getTransaction());
	//Sicherstellen das "Übersicht" im listWidget gewählt ist
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
}

//private Slot
void MainWindow::onStandingOrderDeleteRequest(const aqb_AccountInfo *acc, const abt_standingOrderInfo *da)
{
	if (this->jobctrl->isTransactionInQueue(da->getTransaction())) {
		QMessageBox::critical(this, tr("Bereits im Ausgang"),
			tr("<b>Der Dauerauftrag befindet sich bereits im Ausgang!</b><br /><br />"
			   "Er wurde entweder schon bearbeitet oder soll gelöscht werden. "
			   "Solange sich zu diesem Dauerauftrag bereits eine Änderung "
			   "im Ausgang befindet kann keine weitere Änderung stattfinden.<br />"
			   "Bitte löschen Sie zuerst den entsprechenden Auftrag im Ausgang."),
			QMessageBox::Ok, QMessageBox::Ok);
		return; //Abbruch
	}

	this->jobctrl->addDeleteStandingOrder(acc, da->getTransaction());
}


//private Slot
void MainWindow::onDatedTransferEditRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo *di)
{
	if (this->jobctrl->isTransactionInQueue(di->getTransaction())) {
		QMessageBox::critical(this, tr("Bereits im Ausgang"),
			tr("<b>Die terminierte Überweisung befindet sich bereits im Ausgang!</b><br /><br />"
			   "Sie wurde entweder schon bearbeitet oder soll gelöscht werden. "
			   "Solange sich zu dieser terminierten Überweisung bereits eine Änderung "
			   "im Ausgang befindet kann keine weitere Änderung stattfinden.<br />"
			   "Bitte löschen Sie zuerst den entsprechenden Auftrag im Ausgang."),
			QMessageBox::Ok, QMessageBox::Ok);
		return; //Abbruch
	}

	widgetTransfer *transW;
	transW = this->createTransferWidgetAndAddTab(AB_Job_TypeModifyDatedTransfer,
						     acc);
	transW->setValuesFromTransaction(di->getTransaction());
	//Sicherstellen das "Übersicht" im listWidget gewählt ist
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
}

//private Slot
void MainWindow::onDatedTransferDeleteRequest(const aqb_AccountInfo *acc, const abt_datedTransferInfo *di)
{
	if (this->jobctrl->isTransactionInQueue(di->getTransaction())) {
		QMessageBox::critical(this, tr("Bereits im Ausgang"),
			tr("<b>Die terminierte Überweisung befindet sich bereits im Ausgang!</b><br /><br />"
			   "Sie wurde entweder schon bearbeitet oder soll gelöscht werden. "
			   "Solange sich zu dieser terminierten Überweisung bereits eine Änderung "
			   "im Ausgang befindet kann keine weitere Änderung stattfinden.<br />"
			   "Bitte löschen Sie zuerst den entsprechenden Auftrag im Ausgang."),
			QMessageBox::Ok, QMessageBox::Ok);
		return; //Abbruch
	}

	this->jobctrl->addDeleteDatedTransfer(acc, di->getTransaction());
}

//private Slot
void MainWindow::onEditJobFromOutbox(const abt_jobInfo *job)
{
	/** \todo Der erstellte widgetTransfer enthält bereits Änderungen,
		  dies sollte in diesem auch gesetzt werden, damit bei Klick
		  auf Abbruch eine Warnung erscheint!
	*/

	widgetTransfer *transW;
	const aqb_AccountInfo *acc = NULL;

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

	//den neuen tab gleich darstellen
	this->ui->listWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

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


