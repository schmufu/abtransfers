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
 *	Nur ein TestWidget um die wiget... klassen zu testen
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#include "pagewidgettests.h"

#include <QList>


pageWidgetTests::pageWidgetTests(aqb_AccountInfo *acc, QWidget *parent) :
    QWidget(parent)
{
	qDebug() << Q_FUNC_INFO << "constructor started";
	QHBoxLayout *hb = new QHBoxLayout();
	this->button1 = new QPushButton("Button 1");
	this->button2 = new QPushButton("Button 2");
	this->button3 = new QPushButton("Button 3");
	this->button4 = new QPushButton("Button 4");
	this->button5 = new QPushButton("Button 5");
	this->button6 = new QPushButton("Button 6");
	this->button7 = new QPushButton("Button 7");
	this->button8 = new QPushButton("Button 8");
	hb->addWidget(this->button1);
	hb->addWidget(this->button2);
	hb->addWidget(this->button3);
	hb->addWidget(this->button4);
	hb->addWidget(this->button5);
	hb->addWidget(this->button6);
	hb->addWidget(this->button7);
	hb->addWidget(this->button8);

	connect(this->button1, SIGNAL(clicked()), this, SLOT(onButton1Clicked()));
	connect(this->button2, SIGNAL(clicked()), this, SLOT(onButton2Clicked()));
	connect(this->button3, SIGNAL(clicked()), this, SLOT(onButton3Clicked()));
	connect(this->button4, SIGNAL(clicked()), this, SLOT(onButton4Clicked()));
	connect(this->button5, SIGNAL(clicked()), this, SLOT(onButton5Clicked()));
	connect(this->button6, SIGNAL(clicked()), this, SLOT(onButton6Clicked()));
	connect(this->button7, SIGNAL(clicked()), this, SLOT(onButton7Clicked()));
	connect(this->button8, SIGNAL(clicked()), this, SLOT(onButton8Clicked()));

	this->textEdit = new QPlainTextEdit();
	QVBoxLayout *vb = new QVBoxLayout();
	vb->addLayout(hb);
	vb->addWidget(this->textEdit);

	this->setLayout(vb);

	this->account = acc;

	qDebug() << Q_FUNC_INFO << "constructor ended";
}

pageWidgetTests::~pageWidgetTests()
{
	qDebug() << Q_FUNC_INFO << "destructor started";


	qDebug() << Q_FUNC_INFO << "destructor ended";

}

void pageWidgetTests::onButton1Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));

	if (this->account == NULL) {
		this->textEdit->appendPlainText("Account == NULL --> abort");
		return;
	}


	abt_transaction *t = new abt_transaction();
	t->fillLocalFromAccount(this->account->get_AB_ACCOUNT());
	t->setRemoteAccountNumber("123456");
	t->setRemoteBankCode("29050101");
	t->setRemoteBankName("Sparkasse Bremen");
	t->setRemoteName(QStringList("Test User"));
	t->setValue(abt_conv::ABValueFromString("5.44", "EUR"));
	t->setPurpose(QStringList("Verwendungszweck Test1"));
	t->setTextKey(51);


	this->iec1 = AB_ImExporterContext_new();
	//AB_ImExporterContext_Add
	this->iea1 = AB_ImExporterAccountInfo_new();
	AB_ImExporterAccountInfo_FillFromAccount(this->iea1, this->account->get_AB_ACCOUNT());


	AB_ImExporterContext_AddAccountInfo(this->iec1, this->iea1);
	const QList<abt_StandingInfo*> *stos;
	stos = this->account->getKnownStandingOrders();
	for (int i=0; i<stos->size(); i++) {
		AB_ImExporterContext_AddStandingOrder(this->iec1, AB_Transaction_dup(stos->at(i)->getTransaction()->getAB_Transaction()));
	}




//	int 	AB_Banking_FillGapsInImExporterContext (AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *iec)
//	int 	AB_Banking_ExportToBuffer (AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx, const char *exporterName, const char *profileName, GWEN_BUFFER *buf)
//	int 	AB_Banking_ExportToFile (AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx, const char *exporterName, const char *profileName, const char *fileName)
//	int 	AB_Banking_ExportToFileWithProfile (AB_BANKING *ab, const char *exporterName, AB_IMEXPORTER_CONTEXT *ctx, const char *profileName, const char *profileFile, const char *outputFileName)
//	int 	AB_Banking_ExportWithProfile (AB_BANKING *ab, const char *exporterName, AB_IMEXPORTER_CONTEXT *ctx, const char *profileName, const char *profileFile, GWEN_SYNCIO *sio)
//	int 	AB_Banking_ImportBuffer (AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx, const char *exporterName, const char *profileName, GWEN_BUFFER *buf)
//	int 	AB_Banking_ImportFileWithProfile (AB_BANKING *ab, const char *importerName, AB_IMEXPORTER_CONTEXT *ctx, const char *profileName, const char *profileFile, const char *inputFileName)
//	int 	AB_Banking_ImportWithProfile (AB_BANKING *ab, const char *importerName, AB_IMEXPORTER_CONTEXT *ctx, const char *profileName, const char *profileFile, GWEN_SYNCIO *sio)

	int ret = AB_Banking_ExportToFile(banking->getAqBanking(), this->iec1, "csv", "full", "/tmp/exporterFilename.csv");

	this->textEdit->appendPlainText(QString("%1").arg(ret));


	delete t;
	AB_ImExporterContext_free(this->iec1);



	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}

void pageWidgetTests::onButton2Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));


	this->iec2 = AB_ImExporterContext_new();
	int ret = AB_Banking_ImportFileWithProfile(banking->getAqBanking(), "csv", this->iec2, "full", NULL, "/tmp/exporterFilename.csv");

	this->textEdit->appendPlainText(QString("return value from import: %1").arg(ret));

	//parse the incoming context




	AB_ImExporterContext_free(this->iec2);


	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}

void pageWidgetTests::onButton3Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));



	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}

void pageWidgetTests::onButton4Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));



	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}

void pageWidgetTests::onButton5Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));



	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}

void pageWidgetTests::onButton6Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));



	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}

void pageWidgetTests::onButton7Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));



	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}

void pageWidgetTests::onButton8Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));



	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}
