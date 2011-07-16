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

#include <aqbanking/banking.h>
#include <aqbanking/account.h>
#include <gwenhywfar4/gwen-gui-qt4/qt4_gui.hpp>
#include <aqbanking/jobgettransactions.h>


#include "globalvars.h"
#include "pages/page_da_edit_delete.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	this->accounts = new aqb_Accounts(banking->getAqBanking());
	this->jobctrl = new abt_job_ctrl(this);
	this->logw = new page_log();

	Page_DA_Edit_Delete *page = new Page_DA_Edit_Delete(banking, this->accounts, ui->DA_Bearbeiten);
	ui->DA_Bearbeiten->layout()->addWidget(page);

	QVBoxLayout *logLayout = new QVBoxLayout(ui->Log);
	ui->Log->setLayout(logLayout);
	ui->Log->layout()->addWidget(this->logw);
}

MainWindow::~MainWindow()
{
	delete this->accounts;
	delete this->jobctrl;
	delete this->logw;
	delete ui;
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


void MainWindow::on_actionDebug_Info_triggered()
{
	if (debugDialog->isVisible()) {
		debugDialog->hide();
	} else {
		debugDialog->showNormal();
	}
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

void MainWindow::on_actionAddGetDAs_triggered()
{
	this->jobctrl->addGetStandingOrders(this->accounts->getAccount(0));
	this->ui->statusBar->showMessage("Get Daueraufträge eingefügt");
}


void MainWindow::on_actionAddGetDated_triggered()
{
	this->jobctrl->addGetDatedTransfers(this->accounts->getAccount(0));
	this->ui->statusBar->showMessage("Get Dated Transfers eingefügt");
}

void MainWindow::on_actionExecQueued_triggered()
{
	this->ui->statusBar->showMessage("Executing queued jobs");
	this->logw->setLogText(this->jobctrl->getLog());
	this->jobctrl->execQueuedTransactions();
	this->ui->statusBar->showMessage("queued jobs executed");
	this->logw->setLogText(this->jobctrl->getLog());
}
