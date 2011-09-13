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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	this->accounts = new aqb_Accounts(banking->getAqBanking());
	this->jobctrl = new abt_job_ctrl(this);
	this->logw = new page_log();
	this->outw = new Page_Ausgang(this->jobctrl);
	this->da_edit_del = new Page_DA_Edit_Delete(banking,
						    this->accounts,
						    ui->tabWidget_DA);
	this->da_new = new Page_DA_New(banking,
				       this->accounts,
				       ui->tabWidget_DA);
	this->page_transfer_new = new Page_Ueberweisung_New(banking,
							    this->jobctrl,
							    this->accounts,
							    ui->tabWidget_UW);
	this->page_internaltransfer_new = new Page_InternalTransfer_New(
				banking, this->accounts, ui->tabWidget_UW);
	this->dock_KnownRecipient = NULL;

	//ui->tabWidget_DA->clear();
	ui->tabWidget_DA->addTab(this->da_new, tr("Neu"));
	ui->tabWidget_DA->addTab(this->da_edit_del, tr("Bearbeiten"));

	ui->tabWidget_UW->insertTab(0, this->page_transfer_new, tr("National"));
	ui->tabWidget_UW->insertTab(1, this->page_internaltransfer_new, tr("Umbuchung"));

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
	connect(this->jobctrl, SIGNAL(jobAdded(const abt_job_info*)),
		this, SLOT(onJobAddedToJobCtrlList(const abt_job_info*)));

	//Logs von abt_job_ctrl in der Log-Seite anzeigen
	connect(this->jobctrl, SIGNAL(log(QString)),
		this->logw, SLOT(appendLogText(QString)));

	//Wenn ein DA gelöscht werden soll diesen in abt_job_ctrl einfügen
	connect(this->da_edit_del, SIGNAL(deleteDA(aqb_AccountInfo*,const abt_transaction*)),
		this->jobctrl, SLOT(addDeleteStandingOrder(aqb_AccountInfo*,const abt_transaction*)));

	//Aktualisieren eines DAs
	connect(this->da_edit_del, SIGNAL(getAllDAs(aqb_AccountInfo*)),
		this->jobctrl, SLOT(addGetStandingOrders(aqb_AccountInfo*)));

	//Ändern eines DAs
	connect(this->da_edit_del, SIGNAL(modifyDA(aqb_AccountInfo*,const abt_transaction*)),
		this->jobctrl, SLOT(addModifyStandingOrder(aqb_AccountInfo*,const abt_transaction*)));

	//Neuen DA erstellen
	connect(this->da_new, SIGNAL(createDA(aqb_AccountInfo*,const abt_transaction*)),
		this->jobctrl, SLOT(addCreateStandingOrder(aqb_AccountInfo*,const abt_transaction*)));

	//Neue "Nationale Überweisung" anlegen
	connect(this->page_transfer_new, SIGNAL(createTransfer(aqb_AccountInfo*,const abt_transaction*)),
		this->jobctrl, SLOT(addNewSingleTransfer(aqb_AccountInfo*,const abt_transaction*)));

	//Neue "Umbuchung" anlegen
	connect(this->page_internaltransfer_new, SIGNAL(createInternalTransfer(aqb_AccountInfo*,const abt_transaction*)),
		this->jobctrl, SLOT(addNewInternalTransfer(aqb_AccountInfo*,const abt_transaction*)));

	//Jede Änderung des Jobqueue dem Ausgang mitteilen
// Jetzt im Page_Ausgang Constructor
//	connect(this->jobctrl, SIGNAL(jobQueueListChanged()),
//		this->outw, SLOT(refreshTreeWidget()));

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
	qDebug() << "creating knownEmpfaengerWidget";
	KnownEmpfaengerWidget *kew = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient);
	this->dock_KnownRecipient->setWidget(kew);
	//this->dock_KnownRecipient->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	this->dock_KnownRecipient->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient->setFloating(false);
	this->dock_KnownRecipient->hide();
	this->dock_KnownRecipient->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient);

	//DockWidget für Accounts erstellen
	this->dock_Accounts = new QDockWidget(tr("Unterstüzte Online Konten"),this);
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



	this->dock_KnownRecipient2 = new QDockWidget(tr("Empf2"),this);
	KnownEmpfaengerWidget *kew2 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient2);
	this->dock_KnownRecipient2->setWidget(kew2);
	this->dock_KnownRecipient2->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient2->setFloating(false);
	this->dock_KnownRecipient2->hide();
	this->dock_KnownRecipient2->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient2);

	this->dock_KnownRecipient3 = new QDockWidget(tr("Empf3"),this);
	KnownEmpfaengerWidget *kew3 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient3);
	this->dock_KnownRecipient3->setWidget(kew3);
	this->dock_KnownRecipient3->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient3->setFloating(false);
	this->dock_KnownRecipient3->hide();
	this->dock_KnownRecipient3->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient3);

	this->dock_KnownRecipient4 = new QDockWidget(tr("Empf4"),this);
	KnownEmpfaengerWidget *kew4 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient4);
	this->dock_KnownRecipient4->setWidget(kew4);
	this->dock_KnownRecipient4->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient4->setFloating(false);
	this->dock_KnownRecipient4->hide();
	this->dock_KnownRecipient4->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient4, Qt::Vertical);

	this->dock_KnownRecipient5 = new QDockWidget(tr("Empf5"),this);
	KnownEmpfaengerWidget *kew5 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient5);
	this->dock_KnownRecipient5->setWidget(kew5);
	this->dock_KnownRecipient5->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient5->setFloating(false);
	this->dock_KnownRecipient5->hide();
	this->dock_KnownRecipient5->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient5, Qt::Vertical);

	this->dock_KnownRecipient6 = new QDockWidget(tr("Empf6"),this);
	KnownEmpfaengerWidget *kew6 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient6);
	this->dock_KnownRecipient6->setWidget(kew6);
	this->dock_KnownRecipient6->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient6->setFloating(false);
	this->dock_KnownRecipient6->hide();
	this->dock_KnownRecipient6->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient6, Qt::Horizontal);

	this->dock_KnownRecipient7 = new QDockWidget(tr("Empf7"),this);
	KnownEmpfaengerWidget *kew7 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient7);
	this->dock_KnownRecipient7->setWidget(kew7);
	this->dock_KnownRecipient7->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient7->setFloating(false);
	this->dock_KnownRecipient7->hide();
	this->dock_KnownRecipient7->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient7, Qt::Horizontal);

	this->dock_KnownRecipient8 = new QDockWidget(tr("Empf8"),this);
	KnownEmpfaengerWidget *kew8 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient8);
	this->dock_KnownRecipient8->setWidget(kew8);
	this->dock_KnownRecipient8->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient8->setFloating(false);
	this->dock_KnownRecipient8->hide();
	this->dock_KnownRecipient8->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient8, Qt::Horizontal);




	this->createActions();
	this->createMenus();


	QTimer *timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->start(10);
	connect(timer, SIGNAL(timeout()), this, SLOT(TimerTimeOut()));
}

MainWindow::~MainWindow()
{
	disconnect(this->jobctrl, SIGNAL(jobNotAvailable(AB_JOB_TYPE)),
		   this, SLOT(DisplayNotAvailableTypeAtStatusBar(AB_JOB_TYPE)));

	delete this->page_transfer_new;	//SingleTransfer löschen
	delete this->da_edit_del;	//DauerAufträge ändern löschen
	delete this->da_new;		//DauerAufträge neu erstellen löschen
	delete this->outw;	//AusgangsWidget löschen
	delete this->logw;	//LogWidget löschen
	delete this->jobctrl;	//jobControl-Object löschen
	delete this->accounts;	//account-Object löschen
	delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
	qDebug() << "changeEvent called with: " << e;
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

//private Slot
void MainWindow::TimerTimeOut()
{
	//Actions können zur mainToolBar wohl erst hinzugefügt werden wenn die
	//execLoop läuft, deswegen erst hier nach ablauf des Timers.
	//(Der Timer läuft erst ab wenn die execLoop gestartet ist)
	this->ui->mainToolBar->addAction(this->dock_KnownRecipient->toggleViewAction());
	this->ui->mainToolBar->addAction(this->dock_Accounts->toggleViewAction());
	this->ui->mainToolBar->addAction(this->dock_KnownRecipient2->toggleViewAction());
	this->ui->mainToolBar->addAction(this->dock_KnownRecipient3->toggleViewAction());
	this->ui->mainToolBar->addAction(this->dock_KnownRecipient4->toggleViewAction());
	this->ui->mainToolBar->addAction(this->dock_KnownRecipient5->toggleViewAction());
	this->ui->mainToolBar->addAction(this->dock_KnownRecipient6->toggleViewAction());
	this->ui->mainToolBar->addAction(this->dock_KnownRecipient7->toggleViewAction());
	this->ui->mainToolBar->addAction(this->dock_KnownRecipient8->toggleViewAction());
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
	MenuStanding->addAction(this->actStandingEdit);
	MenuStanding->addAction(this->actStandingUpdate);
	QMenu *MenuDated = new QMenu("Terminüberweisungen", this);
	MenuDated->addAction(this->actDatedNew);
	MenuDated->addAction(this->actDatedEdit);
	MenuDated->addAction(this->actDatedUpdate);
	this->accountContextMenu->addMenu(MenuTransfer);
	this->accountContextMenu->addMenu(MenuStanding);
	this->accountContextMenu->addMenu(MenuDated);
	this->accountContextMenu->addAction(this->actDebitNote);
	this->accountContextMenu->addAction(this->actDebitNoteSepa);
	this->accountContextMenu->addSeparator();
	this->accountContextMenu->addAction(this->actUpdateBalance);
	this->accountContextMenu->addSeparator();
	this->accountContextMenu->addAction("Text1");
	this->accountContextMenu->addAction("Text2");
	this->accountContextMenu->addSeparator();
	this->accountContextMenu->addAction("Text3");
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
void MainWindow::onJobAddedToJobCtrlList(const abt_job_info* ji) const
{
	QMessageBox *msg = new QMessageBox();
	msg->setIcon(QMessageBox::Information);
	msg->setWindowTitle("Job zum Ausgang hinzugefügt");
	QString text = "\"" + ji->getType() + "\" wurde erfolgreich\n";
	text.append("zum Ausgangskorb hinzugefügt.");
	msg->setText(text);
	msg->setStandardButtons(QMessageBox::Ok);
	msg->setDefaultButton(QMessageBox::Ok);

	int ret = msg->exec();

	if (ret != QMessageBox::Ok) {
		qWarning() << "onJobAddedToJobCtrlList(): not handling return != OK, yet";
	}

	delete msg;
}

/*!
 * Item des Listwidget hat sich geändert, die entsprechende Seite des
 * stackedWidget anzeigen.
 */
void MainWindow::on_listWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	if (!current)
		current = previous;

	this->ui->stackedWidget->setCurrentIndex(this->ui->listWidget->row(current));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
	qApp->aboutQt();
}

void MainWindow::on_actionAbout_abTransfers_triggered()
{
	QDialog *about = new QDialog(this);
	about->setWindowTitle("about aqBanking Transfers");

	QVBoxLayout *vbox = new QVBoxLayout(about);
	QLabel *text1 = new QLabel(QString::fromUtf8("<b>aqBanking Transfers</b><br><br>"
			     "Dieses Programm nutzt die library aqbanking um Online-Banking<br>"
			     "Transaktionen durchzuführen.<br><br>"
			     "Es sind alle wesentlichen Vorgänge implementiert, u.a. auch<br>"
			     "Überweisungen und die Verwaltung von Daueraufträgen<br>"));
	vbox->addWidget(text1, 0, Qt::AlignLeft);
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

	QPushButton *ok = new QPushButton("OK");
	vbox->addWidget(ok,0,Qt::AlignRight);

	connect(ok, SIGNAL(clicked()), about, SLOT(accept()));
	//about->layout()->setSpacing(4);

	about->exec();


	delete about;
}

#include "pages/pagewidgettests.h"

void MainWindow::on_actionAddGetDAs_triggered()
{
	qDebug() << "Nothing to do here";
	static QDialog *d = NULL;

	if (d == NULL) {
		d = new QDialog(this);
		QVBoxLayout *vb = new QVBoxLayout();
		widgetTransfer *testw = new widgetTransfer(AB_Job_TypeTransfer,
							   this->accounts->getAccount(5),
							   d);
		vb->addWidget(testw);
		d->setLayout(vb);
	}
	d->showNormal();

}

void MainWindow::on_actionAddGetDated_triggered()
{
	this->ui->statusBar->showMessage("DebugOut Limits Transaction");
//	this->jobctrl->printAllLimits();
}

void MainWindow::on_actionExecQueued_triggered()
{
	this->ui->statusBar->showMessage("Executing queued jobs");
	this->jobctrl->execQueuedTransactions();
	this->ui->statusBar->showMessage("queued jobs executed");
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
	this->accountContextMenu->exec(this->dock_Accounts->widget()->mapToGlobal(p));
}
//private slot
void MainWindow::onActionTransferNationalTriggered()
{
	BankAccountsWidget *acc = this->dock_Accounts->findChild<BankAccountsWidget*>();
	widgetTransfer *trans = new widgetTransfer(AB_Job_TypeTransfer,
						   acc->getSelectedAccount(),
						   this);
	this->ui->tabWidget_UW->addTab(trans, dynamic_cast<QAction*>(QObject::sender())->text() );
}

//private slot
void MainWindow::onActionTransferInternationalTriggered()
{

}

//private slot
void MainWindow::onActionTransferSepaTriggered()
{

}

//private slot
void MainWindow::onActionTransferInternalTriggered()
{

}

//private slot
void MainWindow::onActionDatedNewTriggered()
{

}

//private slot
void MainWindow::onActionDatedEditTriggered()
{

}

//private slot
void MainWindow::onActionDatedDelTriggered()
{

}

//private slot
void MainWindow::onActionDatedUpdateTriggered()
{

}

//private slot
void MainWindow::onActionStandingNewTriggered()
{

}

//private slot
void MainWindow::onActionStandingEditTriggered()
{

}

//private slot
void MainWindow::onActionStandingDelTriggered()
{

}

//private slot
void MainWindow::onActionStandingUpdateTriggered()
{

}

//private slot
void MainWindow::onActionDebitNoteTriggered()
{

}

//private slot
void MainWindow::onActionDebitNoteSepaTriggered()
{

}

//private slot
void MainWindow::onActionUpdateBalanceTriggered()
{

}
