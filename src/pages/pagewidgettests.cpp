/******************************************************************************
 * Copyright (C) 2011, 2014-2015 Patrick Wacker
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
#include <QHBoxLayout>
#include <QPushButton>

#include <QTextCodec>


pageWidgetTests::pageWidgetTests(aqb_Accounts *accs, QWidget *parent) :
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

	this->accounts = accs;

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


	aqb_AccountInfo *acc = this->accounts->getAccount("1234567891");

	if (acc == NULL) {
		this->textEdit->appendPlainText("Account == NULL --> abort");
		return;
	}

	abt_transaction *t = new abt_transaction();
	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());
	t->setRemoteAccountNumber("123456");
	t->setRemoteBankCode("29050101");
	t->setRemoteBankName(QString::fromUtf8("Sparkasse Brämen"));
	t->setRemoteName(QStringList("Test Üser"));
	t->setValue(abt_conv::ABValueFromString("5.44", "EUR"));
	t->setPurpose(QStringList("Überweisung Öfter"));
	t->setTextKey(51);


	this->iec1 = AB_ImExporterContext_new();
	//AB_ImExporterContext_Add
	this->iea1 = AB_ImExporterAccountInfo_new();
	AB_ImExporterAccountInfo_FillFromAccount(this->iea1, acc->get_AB_ACCOUNT());


	AB_ImExporterContext_AddAccountInfo(this->iec1, this->iea1);
	const QList<abt_standingOrderInfo*> *stos;
	stos = acc->getKnownStandingOrders();
	if (stos != NULL) {
		for (int i=0; i<stos->size(); i++) {
			AB_ImExporterContext_AddStandingOrder(this->iec1, AB_Transaction_dup(stos->at(i)->getTransaction()->getAB_Transaction()));
		}
	}

	AB_TRANSACTION *t2 = AB_Transaction_dup(t->getAB_Transaction());

	QString bn = "Sparkasse AeÄ UeÜ OeÖ aeä ueü oeö";
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QByteArray encodedString = codec->fromUnicode(bn);
	qDebug() << encodedString;
	qDebug() << bn;



//	AB_Transaction_SetRemoteBankName(t2, abt_conv::encodeToAb(bn));
	AB_Transaction_SetRemoteBankName(t2, bn.toStdString().c_str());
	AB_Transaction_SetPurpose(t2, abt_conv::QStringListToGwenStringList(QStringList("AeÄ UeÜ OeÖ aeä ueü oeö")));
	AB_ImExporterContext_AddTransfer(this->iec1, t2);


//	int 	AB_Banking_FillGapsInImExporterContext (AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *iec)
//	int 	AB_Banking_ExportToBuffer (AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx, const char *exporterName, const char *profileName, GWEN_BUFFER *buf)
//	int 	AB_Banking_ExportToFile (AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx, const char *exporterName, const char *profileName, const char *fileName)
//	int 	AB_Banking_ExportToFileWithProfile (AB_BANKING *ab, const char *exporterName, AB_IMEXPORTER_CONTEXT *ctx, const char *profileName, const char *profileFile, const char *outputFileName)
//	int 	AB_Banking_ExportWithProfile (AB_BANKING *ab, const char *exporterName, AB_IMEXPORTER_CONTEXT *ctx, const char *profileName, const char *profileFile, GWEN_SYNCIO *sio)
//	int 	AB_Banking_ImportBuffer (AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx, const char *exporterName, const char *profileName, GWEN_BUFFER *buf)
//	int 	AB_Banking_ImportFileWithProfile (AB_BANKING *ab, const char *importerName, AB_IMEXPORTER_CONTEXT *ctx, const char *profileName, const char *profileFile, const char *inputFileName)
//	int 	AB_Banking_ImportWithProfile (AB_BANKING *ab, const char *importerName, AB_IMEXPORTER_CONTEXT *ctx, const char *profileName, const char *profileFile, GWEN_SYNCIO *sio)

	int ret = AB_Banking_ExportToFile(banking->getAqBanking(), this->iec1, "ctxfile", "default", "/tmp/exporterFilename.ctx");

	this->textEdit->appendPlainText(QString("/tmp/exporterFilename.ctx saved (0=OK): %1").arg(ret));


	delete t;
	AB_ImExporterContext_free(this->iec1);



	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}

void pageWidgetTests::onButton2Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));


	this->iec2 = AB_ImExporterContext_new();
	int ret = AB_Banking_ImportFileWithProfile(banking->getAqBanking(), "ctxfile", this->iec2, "default", NULL, "/tmp/exporterFilename.ctx");

	this->textEdit->appendPlainText(QString("return value from import: %1").arg(ret));

	//parse the incoming context
	this->parseContext(this->iec2);



	AB_ImExporterContext_free(this->iec2);


	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}

void pageWidgetTests::onButton3Clicked()
{
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" started"));


	QString t1 = "Grün Weiß";
	addlog(t1);
	addlog(t1.toStdString().c_str());
	addlog(QString::fromUtf8("Grün Weiß"));
	//qDebug() << QTextCodec::codecForCStrings()->availableCodecs());
	addlog(QTextCodec::codecForLocale()->name());
	addlog(QTextCodec::codecForUtfText(QByteArray("Mäüö"))->name());
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	addlog(codec->toUnicode(QByteArray::fromPercentEncoding("Gr%C3%BCn Wei%C3%9F")));
	addlog(QString::fromUtf8(QByteArray::fromPercentEncoding("Gr%C3%BCn Wei%C3%9F")));
	t1 = QString::fromUtf8("Gr%C3%BCn Wei%C3%9F");
	addlog(t1);
	QByteArray a = QByteArray::fromRawData(t1.toStdString().c_str(), t1.length());
	addlog(QString("%1 - %2").arg(t1).arg(QString(a.toPercentEncoding())));


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
	qDebug() << "\n" << "AVAILABLE CODECS:";
	qDebug() << QTextCodec::availableCodecs();
	qDebug() << "\n";
//	qDebug() << "for CStrings:" << QTextCodec::codecForCStrings();
	qDebug() << "for Locale  :" << QTextCodec::codecForLocale()->name();
//	qDebug() << "for Tr()    :" << QTextCodec::codecForTr()->name();
	qDebug() << "for UtfText :" << QTextCodec::codecForUtfText(QByteArray("Sparkasse Brämen"))->name();
	qDebug() << "for System  :" << QTextCodec::codecForName("System")->name();
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" printed supported codecs at stdout"));
	this->textEdit->appendPlainText(QString(Q_FUNC_INFO).append(" ended"));
}





void pageWidgetTests::addlog(const QString &logMsg)
{
	this->textEdit->appendPlainText(QString("LOG: ").append(logMsg));
}



void pageWidgetTests::parseContext(AB_IMEXPORTER_CONTEXT *ctx)
{
	AB_IMEXPORTER_ACCOUNTINFO *ai;

	QString log = AB_ImExporterContext_GetLog(ctx);
	this->addlog(QString("CTX-LOG: ").append(log));

	QString logmsg;
	QString logmsg2;
	int cnt = 0;

	/**********************************************************************/
	//this->parseImExporterContext_Messages(ctx);
	/**********************************************************************/
	AB_MESSAGE *msg;
	logmsg = "Recvd-Message: ";
	cnt = 0;

	msg = AB_ImExporterContext_GetFirstMessage(ctx);
	while (msg) {
		logmsg2 = QString("Empfangsdatum:\t");
		logmsg2.append(abt_conv::GwenTimeToQDate(
				AB_Message_GetDateReceived(msg)).toString(
						Qt::DefaultLocaleLongDate));
		this->addlog(logmsg + logmsg2);
		logmsg2 = QString("Betreff:\t");
		logmsg2.append(AB_Message_GetSubject(msg));
		this->addlog(logmsg + logmsg2);
		logmsg2 = QString("Text:\t");
		logmsg2.append(AB_Message_GetText(msg));
		this->addlog(logmsg + logmsg2);
		msg = AB_ImExporterContext_GetNextMessage(ctx);
		cnt++;
	}

	logmsg2 = QString("Count: %1").arg(cnt);
	this->addlog(logmsg + logmsg2);


	/**********************************************************************/
	//this->parseImExporterContext_Securitys(ctx);
	/**********************************************************************/
	AB_SECURITY *s;
	logmsg = "Recvd-Security: ";
	const AB_VALUE *v;
	cnt = 0;

	s = AB_ImExporterContext_GetFirstSecurity(ctx);
	while (s) {
		logmsg2 = QString("Name:\t");
		logmsg2.append(AB_Security_GetName(s));
		this->addlog(logmsg + logmsg2);

		logmsg2 = QString("UnitPriceValue:\t");
		v = AB_Security_GetUnitPriceValue(s);
		logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
		this->addlog(logmsg + logmsg2);

		s = AB_ImExporterContext_GetNextSecurity(ctx);
		cnt++;
	}

	logmsg2 = QString("Count: %1").arg(cnt);
	this->addlog(logmsg + logmsg2);



	ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
	while(ai) {
		//Beim Anlegen einer Terminüberweisung wird hierher nicht verzweigt!
		/**********************************************************************/
		//this->parseImExporterAccountInfo_Status(ai);
		/**********************************************************************/
		AB_ACCOUNT_STATUS *s;
		logmsg = "Recvd-AccountStatus: ";
		const AB_BALANCE *b;
		cnt = 0;

		s = AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
		while (s) {
			logmsg2 = QString("Balance for: ");
			logmsg2.append(AB_ImExporterAccountInfo_GetAccountNumber(ai));
			logmsg2.append("(");
			logmsg2.append(AB_ImExporterAccountInfo_GetAccountName(ai));
			logmsg2.append(")");
			this->addlog(logmsg + logmsg2);

			v = AB_AccountStatus_GetBankLine(s);
			if (v != NULL) {
				logmsg2 = QString("BankLine:\t");
				logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2));
				this->addlog(logmsg + logmsg2);
			}

			b = AB_AccountStatus_GetNotedBalance(s);
			if (b != NULL) {
				v = AB_Balance_GetValue(b);
				if (v != NULL) {
					logmsg2 = QString("NotedBalance:\t");
					logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2));
					this->addlog(logmsg + logmsg2);
				}
			}

			b = AB_AccountStatus_GetBookedBalance(s);
			if (b != NULL) {
				v = AB_Balance_GetValue(b);
				if (v != NULL) {
					logmsg2 = QString("BookedBalance:\t");
					logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2));
					this->addlog(logmsg + logmsg2);
				}
			}

			v = AB_AccountStatus_GetDisposable(s);
			if (v != NULL) {
				logmsg2 = QString("Disposable:\t");
				logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2));
				this->addlog(logmsg + logmsg2);
			}

			v = AB_AccountStatus_GetDisposed(s);
			if (v != NULL) {
				logmsg2 = QString("Disposed:\t");
				logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v), 0, 'f', 2));
				this->addlog(logmsg + logmsg2);
			}

			logmsg2 = QString("Time:\t");
			logmsg2.append(abt_conv::GwenTimeToQDate(
							AB_AccountStatus_GetTime(s)).toString(
									Qt::DefaultLocaleLongDate));
			this->addlog(logmsg + logmsg2);

			s = AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
			cnt++;
		}

		logmsg2 = QString("Count: %1").arg(cnt);
		this->addlog(logmsg + logmsg2);



		/**********************************************************************/
		//this->parseImExporterAccountInfo_DatedTransfers(ai);	//Terminüberweisungen
		/**********************************************************************/
		AB_TRANSACTION *t;
		logmsg = "Recvd-DatedTransfers: ";
		QStringList strList;
		const GWEN_STRINGLIST *l;
		cnt = 0;

		cnt = AB_ImExporterAccountInfo_GetDatedTransferCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetFirstDatedTransfer(ai);
		while (t) {

			logmsg2 = QString("Purpose:\t");
			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("Value:\t");
			v = AB_Transaction_GetValue(t);
			logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteName:\t");
			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);

			switch (AB_Transaction_GetStatus(t)) {
			case AB_Transaction_StatusRevoked:
				//Bei der Bank hinterlegte Terminüberweisung wurde gelöscht
				this->addlog(QString(
					"Lösche bei der Bank gelöschte Terminüberweisung (ID: %1)"
					).arg(AB_Transaction_GetFiId(t)));
				break;
			case AB_Transaction_StatusManuallyReconciled:
			case AB_Transaction_StatusAutoReconciled:
				//Bei der Bank hinterlegte Terminüberweisung wurde geändert
				this->addlog(QString(
					"Speichere bei der Bank geänderte Terminüberweisung (ID: %1)"
					).arg(AB_Transaction_GetFiId(t)));
				break;
			default:
				//Bei der Bank hinterlegte Terminüberweisung auch lokal speichern
				this->addlog(QString(
					"Speichere bei der Bank hinterlegte Terminüberweisung (ID: %1)"
					).arg(AB_Transaction_GetFiId(t)));
				break;
			}

			t = AB_ImExporterAccountInfo_GetNextDatedTransfer(ai);
		}



		/**********************************************************************/
		//this->parseImExporterAccountInfo_NotedTransactions(ai);	//geplante Buchungen
		/**********************************************************************/
		//AB_TRANSACTION *t;
		logmsg = "Recvd-NotedTransactions: ";
		strList.clear();
		const AB_VALUE *v;
		//const GWEN_STRINGLIST *l;
		cnt = 0;

		cnt = AB_ImExporterAccountInfo_GetNotedTransactionCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetFirstNotedTransaction(ai);
		while (t) {
			logmsg2 = QString("Purpose:\t");
			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("Value:\t");
			v = AB_Transaction_GetValue(t);
			logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteName:\t");
			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);

			t = AB_ImExporterAccountInfo_GetNextNotedTransaction(ai);
		}


		/**********************************************************************/
		//this->parseImExporterAccountInfo_StandingOrders(ai);	//Daueraufträge
		/**********************************************************************/
//		AB_TRANSACTION *t;
		logmsg = "Recvd-StandingOrders: ";
		strList.clear();
//		const AB_VALUE *v;
//		const GWEN_STRINGLIST *l;
		cnt = 0;

		cnt = AB_ImExporterAccountInfo_GetStandingOrderCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetFirstStandingOrder(ai);
		while (t) {
			logmsg2 = QString("Purpose:\t");
			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("Value:\t");
			v = AB_Transaction_GetValue(t);
			logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteName:\t");
			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);


			switch (AB_Transaction_GetStatus(t)) {
			case AB_Transaction_StatusRevoked:
				//Bei der Bank hinterlegter Dauerauftrag wurde gelöscht
				this->addlog(QString(
					"Lösche bei der Bank gelöschten Dauerauftrag (ID: %1)"
					).arg(AB_Transaction_GetFiId(t)));
				break;
			case AB_Transaction_StatusManuallyReconciled:
			case AB_Transaction_StatusAutoReconciled:
				//Bei der Bank hinterlegter Dauerauftrag wurde geändert
				this->addlog(QString(
					"Speichere bei der Bank geänderten Dauerauftrag (ID: %1)"
					).arg(AB_Transaction_GetFiId(t)));
				break;
			default:
				//Bei der Bank hinterlegten Dauerauftrag auch lokal speichern
				this->addlog(QString(
					"Speichere bei der Bank hinterlegten Dauerauftrag (ID: %1)"
					).arg(AB_Transaction_GetFiId(t)));
				break;
			}

			t = AB_ImExporterAccountInfo_GetNextStandingOrder(ai);
		}


		/**********************************************************************/
		//this->parseImExporterAccountInfo_Transactions(ai);	//Buchungen
		/**********************************************************************/
//		AB_TRANSACTION *t;
		logmsg = "Recvd-Transactions: ";
		strList.clear();;
//		const AB_VALUE *v;
//		const GWEN_STRINGLIST *l;
		cnt = 0;

		cnt = AB_ImExporterAccountInfo_GetTransactionCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetFirstTransaction(ai);
		while (t) {
			logmsg2 = QString("Purpose:\t");
			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("Value:\t");
			v = AB_Transaction_GetValue(t);
			logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteName:\t");
			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);

			t = AB_ImExporterAccountInfo_GetNextTransaction(ai);
		}


		/**********************************************************************/
		//this->parseImExporterAccountInfo_Transfers(ai);		//Überweisungen
		/**********************************************************************/
//		AB_TRANSACTION *t;
		logmsg = "Recvd-Transfers: ";
		strList.clear();
//		const AB_VALUE *v;
//		const GWEN_STRINGLIST *l;
		cnt = 0;

		cnt = AB_ImExporterAccountInfo_GetTransferCount(ai);
		logmsg2 = QString("Count: %1").arg(cnt);
		this->addlog(logmsg + logmsg2);

		t = AB_ImExporterAccountInfo_GetFirstTransfer(ai);
		while (t) {
			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << AB_Transaction_GetRemoteBankName(t);
			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QString(AB_Transaction_GetRemoteBankName(t));
			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QString(QByteArray::fromPercentEncoding(AB_Transaction_GetRemoteBankName(t)));
//			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QString::fromAscii(AB_Transaction_GetRemoteBankName(t));
			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QString::fromLatin1(AB_Transaction_GetRemoteBankName(t));
			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QString::fromLocal8Bit(AB_Transaction_GetRemoteBankName(t));
			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QString::fromStdString(AB_Transaction_GetRemoteBankName(t));
			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QString::fromUtf8(AB_Transaction_GetRemoteBankName(t));
			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QByteArray::fromPercentEncoding(QString::fromLatin1(AB_Transaction_GetRemoteBankName(t)).toLatin1());
			qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QString::fromUtf8(QByteArray::fromPercentEncoding(AB_Transaction_GetRemoteBankName(t)));

			QByteArray encodedString = AB_Transaction_GetRemoteBankName(t);
			QTextCodec *codec = QTextCodec::codecForName("UTF-8");
			qDebug() << Q_FUNC_INFO << "1" << codec->name();
			qDebug() << Q_FUNC_INFO << "1 Should always work1:" << codec->toUnicode(encodedString);
			qDebug() << Q_FUNC_INFO << "1 Should always work2:" << codec->toUnicode(AB_Transaction_GetRemoteBankName(t));
			codec = QTextCodec::codecForCStrings();
			if (codec) {
				qDebug() << Q_FUNC_INFO << "2" << codec->name();
				qDebug() << Q_FUNC_INFO << "2 Should always work1:" << codec->toUnicode(encodedString);
				qDebug() << Q_FUNC_INFO << "2 Should always work2:" << codec->toUnicode(AB_Transaction_GetRemoteBankName(t));
			}
			codec = QTextCodec::codecForLocale();
			qDebug() << Q_FUNC_INFO << "3" << codec->name();
			qDebug() << Q_FUNC_INFO << "3 Should always work1:" << QString(codec->toUnicode(encodedString).toStdString().c_str());
			qDebug() << Q_FUNC_INFO << "3 Should always work2:" << QString::fromAscii(codec->toUnicode(encodedString).toStdString().c_str());
			qDebug() << Q_FUNC_INFO << "3 Should always work3:" << QString::fromUtf8(codec->toUnicode(encodedString).toStdString().c_str());
			qDebug() << Q_FUNC_INFO << "3 Should always work4:" << codec->toUnicode(AB_Transaction_GetRemoteBankName(t));
			qDebug() << Q_FUNC_INFO << "3 Should always work5:" << QString::fromUtf8(AB_Transaction_GetRemoteBankName(t));
			codec = QTextCodec::codecForUtfText(encodedString);
			qDebug() << Q_FUNC_INFO << "4" << codec->name();
			qDebug() << Q_FUNC_INFO << "4 Should always work1:" << codec->toUnicode(encodedString);
			qDebug() << Q_FUNC_INFO << "4 Should always work2:" << codec->toUnicode(AB_Transaction_GetRemoteBankName(t));

			qDebug() << Q_FUNC_INFO << "WORKS!:" << abt_conv::encodeFromAb(AB_Transaction_GetRemoteBankName(t));

			const char* str = AB_Transaction_GetRemoteBankName(t);
			while (*str != 0) {
				qDebug("*str = 0x%4X = %c", *str, *str);
				fprintf(stdout, "0x%4X ", *str);
				str++;
			}
			fprintf(stdout, "\n");

			//qDebug() << Q_FUNC_INFO << "RemoteBankName:" << QByteArray::fromRawData(str);

			logmsg2 = QString("RemoteBankName:\t");
			QString s = QString("%1").arg(AB_Transaction_GetRemoteBankName(t));
			logmsg2.append(s);
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteBankName:\t");
			QString bankName = abt_conv::encodeFromAb(AB_Transaction_GetRemoteBankName(t));
			logmsg2.append(bankName);
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteBankName:\t");
			bankName = AB_Transaction_GetRemoteBankName(t);
			logmsg2.append(bankName);
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteBankName:\t");
			bankName = QString::fromLocal8Bit(AB_Transaction_GetRemoteBankName(t));
			logmsg2.append(bankName);
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteBankName:\t");
			bankName = QString::fromUtf8(AB_Transaction_GetRemoteBankName(t));
			logmsg2.append(bankName);
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteBankName:\t");
			bankName = QString::fromAscii(AB_Transaction_GetRemoteBankName(t));
			logmsg2.append(bankName);
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteBankName:\t");
			bankName = QString::fromLatin1(AB_Transaction_GetRemoteBankName(t));
			logmsg2.append(bankName);
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("Purpose:\t");
			l = AB_Transaction_GetPurpose(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("Value:\t");
			v = AB_Transaction_GetValue(t);
			qDebug() << Q_FUNC_INFO << "Value:" << abt_conv::ABValueToString(v);
			logmsg2.append(QString("%1").arg(AB_Value_GetValueAsDouble(v)));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteName:\t");
			l = AB_Transaction_GetRemoteName(t);
			strList = abt_conv::GwenStringListToQStringList(l);
			logmsg2.append(strList.join(" - "));
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("LocalName:\t");
			bankName = QString::fromUtf8(AB_Transaction_GetLocalName(t));
			logmsg2.append(bankName);
			this->addlog(logmsg + logmsg2);

			abt_transaction trans(t);
			this->addlog("----- abt_transaction usage below -----");

			logmsg2 = QString("LocalName:\t");
			bankName = trans.getLocalName();
			logmsg2.append(bankName);
			this->addlog(logmsg + logmsg2);

			logmsg2 = QString("RemoteBankName:\t");
			bankName = trans.getRemoteBankName();
			logmsg2.append(bankName);
			this->addlog(logmsg + logmsg2);


			t = AB_ImExporterAccountInfo_GetNextTransfer(ai);
		}





		ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
	} /* while ai */

	//return true;
}
