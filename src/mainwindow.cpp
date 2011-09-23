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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	this->accounts = new aqb_Accounts(banking->getAqBanking());
	this->jobctrl = new abt_job_ctrl(this);
	this->logw = new page_log();
	this->outw = new Page_Ausgang(this->jobctrl);
	this->dock_KnownRecipient = NULL;

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
	this->dock_KnownRecipient->setWidget(kew);
	//this->dock_KnownRecipient->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	this->dock_KnownRecipient->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient->setFloating(false);
	this->dock_KnownRecipient->hide();
	this->dock_KnownRecipient->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient);

	//DockWidget für Accounts erstellen
	this->dock_Accounts = new QDockWidget(tr("Unterstüzte Online Konten"),this);
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



	this->dock_KnownRecipient2 = new QDockWidget(tr("Empf2"),this);
	this->dock_KnownRecipient2->setObjectName("KnownRecipients2");
	KnownEmpfaengerWidget *kew2 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient2);
	this->dock_KnownRecipient2->setWidget(kew2);
	this->dock_KnownRecipient2->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient2->setFloating(false);
	this->dock_KnownRecipient2->hide();
	this->dock_KnownRecipient2->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient2);

	this->dock_KnownRecipient3 = new QDockWidget(tr("Empf3"),this);
	this->dock_KnownRecipient3->setObjectName("KnownRecipients3");
	KnownEmpfaengerWidget *kew3 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient3);
	this->dock_KnownRecipient3->setWidget(kew3);
	this->dock_KnownRecipient3->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient3->setFloating(false);
	this->dock_KnownRecipient3->hide();
	this->dock_KnownRecipient3->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient3);

	this->dock_KnownRecipient4 = new QDockWidget(tr("Empf4"),this);
	this->dock_KnownRecipient4->setObjectName("KnownRecipients4");
	KnownEmpfaengerWidget *kew4 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient4);
	this->dock_KnownRecipient4->setWidget(kew4);
	this->dock_KnownRecipient4->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient4->setFloating(false);
	this->dock_KnownRecipient4->hide();
	this->dock_KnownRecipient4->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient4, Qt::Vertical);

	this->dock_KnownRecipient5 = new QDockWidget(tr("Empf5"),this);
	this->dock_KnownRecipient5->setObjectName("KnownRecipients5");
	KnownEmpfaengerWidget *kew5 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient5);
	this->dock_KnownRecipient5->setWidget(kew5);
	this->dock_KnownRecipient5->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient5->setFloating(false);
	this->dock_KnownRecipient5->hide();
	this->dock_KnownRecipient5->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient5, Qt::Vertical);

	this->dock_KnownRecipient6 = new QDockWidget(tr("Empf6"),this);
	this->dock_KnownRecipient6->setObjectName("KnownRecipients6");
	KnownEmpfaengerWidget *kew6 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient6);
	this->dock_KnownRecipient6->setWidget(kew6);
	this->dock_KnownRecipient6->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient6->setFloating(false);
	this->dock_KnownRecipient6->hide();
	this->dock_KnownRecipient6->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient6, Qt::Horizontal);

	this->dock_KnownRecipient7 = new QDockWidget(tr("Empf7"),this);
	this->dock_KnownRecipient7->setObjectName("KnownRecipients7");
	KnownEmpfaengerWidget *kew7 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient7);
	this->dock_KnownRecipient7->setWidget(kew7);
	this->dock_KnownRecipient7->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient7->setFloating(false);
	this->dock_KnownRecipient7->hide();
	this->dock_KnownRecipient7->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient7, Qt::Horizontal);

	this->dock_KnownRecipient8 = new QDockWidget(tr("Empf8"),this);
	this->dock_KnownRecipient8->setObjectName("KnownRecipients8");
	KnownEmpfaengerWidget *kew8 = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this->dock_KnownRecipient8);
	this->dock_KnownRecipient8->setWidget(kew8);
	this->dock_KnownRecipient8->setAllowedAreas(Qt::AllDockWidgetAreas);
	this->dock_KnownRecipient8->setFloating(false);
	this->dock_KnownRecipient8->hide();
	this->dock_KnownRecipient8->toggleViewAction()->setIcon(QIcon(":/icons/knownEmpfaenger"));
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock_KnownRecipient8, Qt::Horizontal);


	this->createActions();
	this->createMenus();
	this->createDockToolbar();

	this->ui->tabWidget_UW->setTabText(0, tr("Überischt"));
	connect(this->ui->pushButton, SIGNAL(clicked()),
		this->actTransferNational, SLOT(trigger()));
	connect(this->ui->pushButton_2, SIGNAL(clicked()),
		this->actTransferInternational, SLOT(trigger()));
	connect(this->ui->pushButton_3, SIGNAL(clicked()),
		this->actTransferSepa, SLOT(trigger()));
	connect(this->ui->pushButton_4, SIGNAL(clicked()),
		this->actTransferInternal, SLOT(trigger()));

	//QGroupBox *grpKSO = new QGroupBox(this->ui->MainTab);
	//widgetKnownStandingOrders *kso = new widgetKnownStandingOrders(this->accounts->getAccountHash().value(5, NULL), grpKSO);


//	QTimer *timer = new QTimer(this);
//	timer->setSingleShot(true);
//	timer->start(10);
//	connect(timer, SIGNAL(timeout()), this, SLOT(TimerTimeOut()));
}

MainWindow::~MainWindow()
{
	disconnect(this->jobctrl, SIGNAL(jobNotAvailable(AB_JOB_TYPE)),
		   this, SLOT(DisplayNotAvailableTypeAtStatusBar(AB_JOB_TYPE)));

//	delete this->page_transfer_new;	//SingleTransfer löschen
//	delete this->da_edit_del;	//DauerAufträge ändern löschen
//	delete this->da_new;		//DauerAufträge neu erstellen löschen
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
//	this->ui->mainToolBar->addAction(this->dock_KnownRecipient->toggleViewAction());
//	this->ui->mainToolBar->addAction(this->dock_Accounts->toggleViewAction());
//	this->ui->mainToolBar->addAction(this->dock_KnownRecipient2->toggleViewAction());
//	this->ui->mainToolBar->addAction(this->dock_KnownRecipient3->toggleViewAction());
//	this->ui->mainToolBar->addAction(this->dock_KnownRecipient4->toggleViewAction());
//	this->ui->mainToolBar->addAction(this->dock_KnownRecipient5->toggleViewAction());
//	this->ui->mainToolBar->addAction(this->dock_KnownRecipient6->toggleViewAction());
//	this->ui->mainToolBar->addAction(this->dock_KnownRecipient7->toggleViewAction());
//	this->ui->mainToolBar->addAction(this->dock_KnownRecipient8->toggleViewAction());
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

//private
void MainWindow::createDockToolbar()
{
	this->dockToolbar = new QToolBar(tr("Hilfsfenster"),this);
	this->dockToolbar->setObjectName("dockToolbar");
	this->dockToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	this->dockToolbar->addAction(this->dock_KnownRecipient->toggleViewAction());
	this->dockToolbar->addAction(this->dock_Accounts->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownRecipient2->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownRecipient3->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownRecipient4->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownRecipient5->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownRecipient6->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownRecipient7->toggleViewAction());
	this->dockToolbar->addAction(this->dock_KnownRecipient8->toggleViewAction());
	this->addToolBar(Qt::TopToolBarArea, this->dockToolbar);
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

	//Lizenz-Text auf Anforderung (Click) anzeigen
	QDialog *licenseDialog = new QDialog(about);
	licenseDialog->setWindowTitle(tr("Lizenz"));
	QVBoxLayout *licenseLayout = new QVBoxLayout(licenseDialog);
	QLabel *licenseText = new QLabel(licenseDialog);
	licenseText->setText("Copyright (C) 2011 Patrick Wacker<br /><br />"
			     "Dieses Programm ist freie Software. Sie können es unter den Bedingungen der<br />"
			     "GNU General Public License, wie von der Free Software Foundation veröffentlicht,<br />"
			     "weitergeben und/oder modifizieren, entweder gemÃ¤ÃŸ Version 2 der Lizenz oder (nach<br />"
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

	QLabel *text1 = new QLabel(QString::fromUtf8("<b>aqBanking Transfers</b><br><br>"
			     "Dieses Programm nutzt die library aqbanking um Online-Banking<br>"
			     "Transaktionen durchzuführen.<br><br>"
			     "Es sind alle wesentlichen Vorgänge von aqbanking implementiert,<br>"
			     "u.a. auch Überweisungen, Lastschriften Daueraufträge usw.<br>"
			     "Sowie eine selbst Implementierte Verwaltung von Daueraufträgen<br>"
			     "und Terminierten Überweisungen."));
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
							   this->accounts,
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
	BankAccountsWidget *acc = this->dock_Accounts->findChild<BankAccountsWidget*>();
	if (acc->getSelectedAccount() != NULL) {
		//Menü nur anzeigen wenn auch ein Account ausgewählt ist
		this->accountContextMenu->exec(this->dock_Accounts->widget()->mapToGlobal(p));
	}
}
//private slot
void MainWindow::onActionTransferNationalTriggered()
{
//	BankAccountsWidget *acc = this->dock_Accounts->findChild<BankAccountsWidget*>();
//	widgetTransfer *trans = new widgetTransfer(AB_Job_TypeTransfer,
//						   acc->getSelectedAccount(),
//						   this);
//	this->ui->tabWidget_UW->addTab(trans, dynamic_cast<QAction*>(QObject::sender())->text() );
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

}

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
void MainWindow::createTransferWidgetAndAddTab(AB_JOB_TYPE type)
{
	BankAccountsWidget *acc = this->dock_Accounts->findChild<BankAccountsWidget*>();
	widgetTransfer *trans = new widgetTransfer(type,
						   acc->getSelectedAccount(),
						   this->accounts,
						   this);
	//this->ui->tabWidget_UW->addTab(trans, dynamic_cast<QAction*>(QObject::sender())->text() );
	this->ui->tabWidget_UW->addTab(trans, abt_conv::JobTypeToQString(type));

	connect(trans, SIGNAL(createTransfer(AB_JOB_TYPE,const widgetTransfer*)),
		this, SLOT(onWidgetTransferCreateTransfer(AB_JOB_TYPE,const widgetTransfer*)));
	connect(trans, SIGNAL(cancelClicked(widgetTransfer*)),
		this, SLOT(onWidgetTransferCancelClicked(widgetTransfer*)));
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
				      tr("Folgende Eingaben sind fehlerhaft:\n"
					 "%1\n"
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
		qWarning() << "onWidgetTransferCreateTransfer(): type '" <<
				type << "' not supported! No Job added! ABBRUCH!";
		return;
		break;
	}

	//Der Transfer wurde in den Ausgangskorb übernommen, das Widget und den
	//Tab löschen
	this->deleteTabWidgetAndTab(sender);
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
void MainWindow::createAndSendEUTransfer(const widgetTransfer *sender)
{
	qWarning() << "create EU Transfer not implemented yet!";
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

	this->jobctrl->addNewSingleTransfer(acc, t);

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

	this->jobctrl->addNewSingleTransfer(acc, t);

	delete t;
}

//private
/** darf nur aufgerufen werden wenn alle Eingaben OK sind! */
void MainWindow::createAndSendSepaTransfer(const widgetTransfer *sender)
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
void MainWindow::createAndSendModifyDatedTransfer(const widgetTransfer *sender)
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
void MainWindow::createAndSendModifyStandingOrder(const widgetTransfer *sender)
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
void MainWindow::createAndSendDebitNote(const widgetTransfer *sender)
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
void MainWindow::createAndSendInternalTransfer(const widgetTransfer *sender)
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
void MainWindow::createAndSendSepaDebitNote(const widgetTransfer *sender)
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

