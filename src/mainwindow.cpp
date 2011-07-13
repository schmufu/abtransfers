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

	Page_DA_Edit_Delete *page = new Page_DA_Edit_Delete(banking, this->accounts, ui->DA_Bearbeiten);
	ui->DA_Bearbeiten->layout()->addWidget(page);

}

MainWindow::~MainWindow()
{
	delete this->accounts;
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

//void MainWindow::on_pushButton_clicked()
//{
//	QProcess *proc;
//	QStringList args;
//	QString program = "ls";
//
//	args << "-l" << "/home/sod";
//
//	this->ui->plainTextEdit->appendPlainText("Starting ls -l /home/sod\n");
//
//	proc = new QProcess(this);
//	proc->start(program, args);
//
//	if (!proc->waitForFinished(30000))
//		return;	//Abbruch
//
//	this->ui->plainTextEdit->appendPlainText(proc->readAllStandardOutput());
//
//	delete proc;
//
//}
//
//void MainWindow::on_pushButton_2_clicked()
//{
//	AB_BANKING *ab;
//	int rv;
//	AB_ACCOUNT *a;
//	QT4_Gui *gui;
//
//	gui = new QT4_Gui();
//	GWEN_Gui_SetGui(gui->getCInterface());
//
//	ab=AB_Banking_new("tutorial3", NULL, 0);
//
//	/* This is the basic init function. It only initializes the minimum (like
//	 * setting up plugin and data paths). After this function successfully
//	 * returns you may freely use any non-online function. To use online
//	 * banking functions (like getting the list of managed accounts, users
//	 * etc) you will have to call AB_Banking_OnlineInit().
//	 */
//	rv=AB_Banking_Init(ab);
//	if (rv) {
//	  fprintf(stderr, "Error on init (%d)\n", rv);
//	  return;
//	}
//	fprintf(stderr, "AqBanking successfully initialized.\n");
//
//	/* This function loads the settings file of AqBanking so the users and
//	 * accounts become available after this function successfully returns.
//	 */
//	rv=AB_Banking_OnlineInit(ab);
//	if (rv) {
//	  fprintf(stderr, "Error on init of online modules (%d)\n", rv);
//	  return;
//	}
//
//	/* Any type of job needs an account to operate on. The following function
//	 * allows wildcards (*) and jokers (?) in any of the arguments. */
//	a=AB_Banking_FindAccount(ab,
//				 "aqhbci", /* backend name */
//				 "de",     /* two-char ISO country code */
//				 "29050101",   /* bank code (with wildcard) */
//				 "12596391",      /* account number (wildcard) */
//				 "*");     /* sub account id (Unterkontomerkmal) */
//	if (a) {
//	  AB_JOB_LIST2 *jl;
//	  AB_JOB *j;
//	  AB_IMEXPORTER_CONTEXT *ctx;
//
//	  /* create a job which retrieves transaction statements. */
//	  j=AB_JobGetTransactions_new(a);
//
//	  /* This function checks whether the given job is available with the
//	   * backend/provider to which the account involved is assigned.
//	   * The corresponding provider/backend might also check whether this job
//	   * is available with the given account.
//	   * If the job is available then 0 is returned, otherwise the error code
//	   * might give you a hint why the job is not supported. */
//	  rv=AB_Job_CheckAvailability(j);
//	  if (rv) {
//	    fprintf(stderr, "Job is not available (%d)\n", rv);
//	    return;
//	  }
//
//	  /* create a job list to which the jobs to be executed are added.
//	   * This list is later given as an argument to the queue execution
//	   * function.
//	   */
//	  jl=AB_Job_List2_new();
//
//	  /* add job to this list */
//	  AB_Job_List2_PushBack(jl, j);
//
//	  /* When executing a list of enqueued jobs (as we will do below) all the
//	   * data returned by the server will be stored within an ImExporter
//	   * context.
//	   */
//	  ctx=AB_ImExporterContext_new();
//
//	  /* execute the jobs which are in the given list (well, for this tutorial
//	   * there is only one job in the list, but the number is not limited).
//	   * This effectivly sends all jobs to the respective backends/banks.
//	   * It only returns an error code (!=0) if there has been a problem
//	   * sending the jobs.
//	   */
//	  rv=AB_Banking_ExecuteJobs(ab, jl, ctx);
//	  if (rv) {
//	    fprintf(stderr, "Error on executeQueue (%d)\n", rv);
//	    return;
//	  }
//	  else {
//	    AB_IMEXPORTER_ACCOUNTINFO *ai;
//
//	    ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
//	    while(ai) {
//	      const AB_TRANSACTION *t;
//
//	      t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
//	      while(t) {
//		const AB_VALUE *v;
//
//		v=AB_Transaction_GetValue(t);
//		if (v) {
//		  const GWEN_STRINGLIST *sl;
//		  const char *purpose;
//
//		  /* The purpose (memo field) might contain multiple lines.
//		   * Therefore AqBanking stores the purpose in a string list
//		   * of which the first entry is used in this tutorial */
//		  sl=AB_Transaction_GetPurpose(t);
//		  if (sl)
//		    purpose=GWEN_StringList_FirstString(sl);
//		  else
//		    purpose="";
//
//		  fprintf(stderr, " %-32s (%.2f %s)\n",
//			  purpose,
//			  AB_Value_GetValueAsDouble(v),
//			  AB_Value_GetCurrency(v));
//		}
//		t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
//	      } /* while transactions */
//	      ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
//	    } /* while ai */
//	  } /* if executeQueue successfull */
//	  /* free the job to avoid memory leaks */
//	  AB_Job_free(j);
//	} /* if account found */
//	else {
//	  fprintf(stderr, "No account found.\n");
//	}
//
//	/* This function MUST be called in order to let AqBanking save the changes
//	 * to the users and accounts (like they occur after executing jobs).
//	 */
//	rv=AB_Banking_OnlineFini(ab);
//	if (rv) {
//	  fprintf(stderr, "ERROR: Error on deinit online modules (%d)\n", rv);
//	  return;
//	}
//
//	/* This function deinitializes AqBanking. It undoes the effects of
//	 * AB_Banking_Init() and should be called before destroying an AB_BANKING
//	 * object.
//	 */
//	rv=AB_Banking_Fini(ab);
//	if (rv) {
//	  fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
//	  return;
//	}
//	AB_Banking_free(ab);
//
//	aqb_Accounts *accounts;
//	accounts = new aqb_Accounts(banking->getAqBanking());
//
//	qDebug() << "ID 0 - Name:" << accounts->getAccount(0)->Name();
//
//
//
//
//
//	BankAccountsWidget *w = new BankAccountsWidget(accounts,
//						       ui->scrollAreaWidgetContents);
//
//	w->show();
//
//
//
//	delete accounts;
//}

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
	QLabel *version = new QLabel(QString("Version: %1").arg(ABTRANSFER_VERSION));
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
